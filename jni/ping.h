#ifndef _PING_H
#define _PING_H

#include <unistd.h>
#include <stdbool.h>

#include <android/log.h>
#define ERROR(msg...) __android_log_print(ANDROID_LOG_ERROR,"cnping:pinger",msg);
#define INFO(msg...) __android_log_print(ANDROID_LOG_INFO,"cnping:pinger",msg);

struct ip_header
{
  uint8_t ver_ihl;
  uint8_t tos;
  uint16_t tot_len;
  uint16_t id;
  uint16_t frag_off;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t check;
  uint32_t saddr;
  uint32_t daddr;
};

struct icmp_header {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint16_t id;
  uint16_t seq_num;
};

struct icmp_packet
{
  struct icmp_header header;
  uint8_t message[2048];
};

extern "C" {
  extern volatile bool is_running;
  extern struct sockaddr_in ping_address;
  extern int ping_socket;
  extern int listen_socket;
  extern uint16_t extra_ping_size;
  extern double ping_period;

  unsigned short checksum(void *b, int len);
  void *pinger(void *r);
  void *listener(void *r);
  void draw(uint32_t *bitmap_buffer, uint32_t width, uint32_t height);
}

void DrawFrame();

#endif
