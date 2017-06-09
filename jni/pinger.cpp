#include <stdlib.h>
#include <string>
#include <jni.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
// #include <resolv.h>
#include <unistd.h>
#include <sys/un.h>
#include <netdb.h>
// #include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <android/bitmap.h>

#include "ping.h"
#include "os_generic.h"

#define UDSOCK_NAME "/data/data/com.viknet.cnping/_socket"

int read_sd(int ufd){
  unsigned char data = 0;
  struct msghdr msg;
  struct iovec iov;
  char   control[CMSG_SPACE(sizeof(int))];

  struct cmsghdr *cmsg;

  iov.iov_base = &data;
  iov.iov_len = sizeof(data);

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  
  msg.msg_flags = 0;

  msg.msg_control = (void*) control;
  msg.msg_controllen = sizeof(control);

  int nr = recvmsg(ufd, &msg, 0);
  if (nr == -1){
    ERROR("Error in recvmsg: %d\n", errno);
    return -1;
  }

  cmsg = CMSG_FIRSTHDR(&msg);
  if (cmsg == NULL || cmsg->cmsg_len != msg.msg_controllen){
    ERROR("bad cmsg header / message length");
    return -1;
  }
  if (cmsg->cmsg_level != SOL_SOCKET){
    ERROR("cmsg_level != SOL_SOCKET");
    return -1;
  }
  if (cmsg->cmsg_type != SCM_RIGHTS){
    ERROR("cmsg_type != SCM_RIGHTS");
    return -1;
  }

  return *((int *) CMSG_DATA(cmsg));
}

extern "C" {

  JNIEXPORT jboolean JNICALL Java_com_viknet_cnping_MainActivity_requestSockets(JNIEnv *env, jobject obj){
    
    if (ping_socket>=0){
      close(ping_socket);
      ping_socket = -1;
    }

    if (listen_socket>=0){
      close(listen_socket);
      listen_socket = -1;
    }

    unlink(UDSOCK_NAME);
    int ufd = socket(PF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UDSOCK_NAME, sizeof(addr.sun_path)-1);
    bind(ufd, (struct sockaddr*)&addr, sizeof(addr));
    //su --context u:r:system_app:s0 -c

    FILE *se_file = fopen("/sys/fs/selinux/enforce", "r");
    if (se_file){
      char is_enforced = 0;
      if (!fread(&is_enforced, 1, 1, se_file))
        ERROR("Error reading \"/sys/fs/selinux/enforce\" file");
      fclose(se_file);

      if (is_enforced == '1'){
        INFO("SELinux enforced. Patching policies.");
        char buf[80];
        FILE *fp = NULL;
        if ((fp = popen("su -c 'supolicy --live \"allow untrusted_app init rawip_socket { read write }\"'", "r")) == NULL)
          ERROR("Error launching supolicy!\n")
        else {
          while (fgets(buf, 80, fp) != NULL)
            INFO("OUTPUT: %s", buf);
          pclose(fp);
        }
      } else
        INFO("SELinux is not enforced.");
    } else
      INFO("SELinux not found");

    int retval = system("su -c /data/data/com.viknet.cnping/lib/libhelper.so");
    INFO("Helper exit: %d", retval);

    if (retval != 0){
      ERROR("Helper failed.");
      return false;
    }

    ping_socket = read_sd(ufd);
    if (ping_socket < 0){
      ERROR("Ping socket retreival failed");
      return false;
    }

    listen_socket = read_sd(ufd);
    if (listen_socket < 0){
      ERROR("Listen socket retreival failed");
      return false;
    }

    INFO("Got sockets");

    return true;
  }

  JNIEXPORT jboolean JNICALL Java_com_viknet_cnping_MainActivity_startPing(JNIEnv *env, jobject obj, jstring jhostname){
    if (ping_socket < 0 || listen_socket < 0)
      return false;

    //TODO test with non-latin encoding
    const char *hostname = env->GetStringUTFChars(jhostname, NULL);
    struct hostent *host_info = gethostbyname(hostname);

    if (!host_info)
      return false;

    memset(&ping_address, 0, sizeof(ping_address));
    ping_address.sin_family = host_info->h_addrtype;
    memcpy((char *)&ping_address.sin_addr.s_addr,host_info->h_addr_list[0],host_info->h_length);

    is_running = true;

    OGCreateThread(pinger, 0);
    OGCreateThread(listener, 0);

    return true;
  }

  JNIEXPORT void JNICALL Java_com_viknet_cnping_MainActivity_stopPing(JNIEnv *env, jobject obj){
    is_running = false;
  }

  JNIEXPORT void JNICALL Java_com_viknet_cnping_PingImageView_drawFrame(JNIEnv *env, jobject ob, jobject bitmap){
    AndroidBitmapInfo info;
    void* pixels;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
      ERROR("AndroidBitmap_getInfo() failed ! error=%d", ret);
      return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
      ERROR("AndroidBitmap_lockPixels() failed ! error=%d", ret);
      return;
    }

    draw((uint32_t *)pixels, info.width, info.height);
    AndroidBitmap_unlockPixels(env, bitmap);
  }

}
