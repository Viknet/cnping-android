// #include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <resolv.h>
#include <unistd.h>
#include <sys/un.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>

#include <android/log.h>

#define ERROR(msg...) __android_log_print(ANDROID_LOG_ERROR,"cnping:helper",msg);
#define INFO(msg...) __android_log_print(ANDROID_LOG_INFO,"cnping:helper",msg);

// void print_errors(){
//     printf("EAGAIN: %d\n", EAGAIN);
//     printf("EWOULDBLOCK: %d\n", EWOULDBLOCK);
//     printf("EBADF: %d\n", EBADF);
//     printf("ECONNREFUSED: %d\n", ECONNREFUSED);
//     printf("EINTR: %d\n", EINTR);
//     printf("EINVAL: %d\n", EINVAL);
//     printf("ENOMEM: %d\n", ENOMEM);
//     printf("ENOTCONN: %d\n", ENOTCONN);
//     printf("ENOTSOCK: %d\n", ENOTSOCK);
// }

#define UDSOCK_NAME "/data/data/com.viknet.cnping/_socket"
#define ICMP_PROTO_NUMBER 1

bool send_sd(int ufd, int sd){
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

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = msg.msg_controllen;

    *(int*) CMSG_DATA(cmsg) = sd;

    return sendmsg(ufd, &msg, 0) != -1;
}

int main(int argc, char const *argv[])
{
    int ufd = socket(PF_UNIX, SOCK_DGRAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UDSOCK_NAME, sizeof(addr.sun_path)-1);
    connect(ufd, (struct sockaddr*)&addr, sizeof(addr));

    const uint8_t val = 255;
    struct timeval tv;
    tv.tv_sec = 1;

    int sd = socket(PF_INET, SOCK_RAW, ICMP_PROTO_NUMBER);
    if (sd < 0){
        ERROR("Socket creation error : %d\n",errno);
        exit(-1);
    }
    if (setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0){
        ERROR("Set TTL failed\n");
        exit(-1);
    }
    if (fcntl(sd, F_SETFL, O_NONBLOCK) != 0){
        ERROR("Set O_NONBLOCK failed\n");
        exit(-1);
    }
    if (!send_sd(ufd, sd)){
        ERROR("Ping socket send error: %d", errno);
        exit(-1);
    }

    sd = socket(PF_INET, SOCK_RAW, ICMP_PROTO_NUMBER);
    if (sd < 0){
        ERROR("Socket creation error : %d\n",errno);
        exit(-1);
    }
    if (setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0){
        ERROR("Set TTL failed\n");
        exit(-1);
    }
    if (setsockopt (sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
        ERROR("Set timeout failed\n");
        exit(-1);
    }
    if (!send_sd(ufd, sd)){
        ERROR("Listen socket send error: %d", errno);
        exit(-1);
    }

    unlink(UDSOCK_NAME);
    return 0;
}

