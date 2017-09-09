#include <stdio.h>
#include <math.h>
#include "ping.h"
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>

#include "os_generic.h"
#include "CNFGDriver.h"
#include "CNFGFunctions.h"

#define PATTERN_SIZE 8
#define PINGCYCLEWIDTH 4096

uint8_t pattern[PATTERN_SIZE];
double PingSendTimes[PINGCYCLEWIDTH];
double PingRecvTimes[PINGCYCLEWIDTH];
uint32_t current_cycle = 0;
float GuiYScaleFactor = 0;
bool GuiYscaleFactorIsConstant = false;

void display(uint8_t *buf, ssize_t bytes)
{
  uint32_t reqid = ntohl(*(uint32_t *)buf) & (PINGCYCLEWIDTH-1);

  double STime = PingSendTimes[reqid];
  double LRTime = PingRecvTimes[reqid];

  if( memcmp( buf+4, pattern, PATTERN_SIZE ) != 0 ) return;
  if( LRTime > STime ) return;
  if( STime < 1 ) return;

  //Otherwise this is a legit packet.

  PingRecvTimes[reqid] = OGGetAbsoluteTime();
}

ssize_t load_ping_packet( uint8_t * buffer)
{
  *((uint32_t*)buffer) = htonl(current_cycle);

  memcpy( buffer+4, pattern, PATTERN_SIZE );

  uint32_t reqid = current_cycle&(PINGCYCLEWIDTH-1);
  PingSendTimes[reqid] = OGGetAbsoluteTime();
  PingRecvTimes[reqid] = 0;

  current_cycle++;

  return 4 + PATTERN_SIZE + extra_ping_size;
}

uint16_t checksum(void *buffer, size_t size){
  uint16_t *b = (uint16_t *) buffer;
  uint32_t summ = 0;

  for(;size > 1; size -= 2)
    summ += *b++;

  if (size)
    summ += *(uint8_t*)b;

  summ = (summ >> 16) + (summ & 0xffff);
  summ += summ >> 16;
  return (uint16_t)(~summ);
}

extern "C" {
  volatile bool is_running = false;
  struct sockaddr_in ping_address;
  int ping_socket = -1;
  int listen_socket = -1;
  uint16_t extra_ping_size = 0;
  double ping_period = 0.02;

  void *pinger(void *r){
    INFO("pinger started");

    current_cycle = 0;
    memset(PingSendTimes, 0, sizeof(PingSendTimes));
    memset(PingRecvTimes, 0, sizeof(PingSendTimes));
    for(uint16_t i = 0; i < PATTERN_SIZE; i++)
      pattern[i] = rand();

    struct icmp_packet pckt;
    uint16_t cnt = 0;
    pid_t pid = getpid();

    memset(&pckt, 0, sizeof(pckt));
    pckt.header.type = 8;
    pckt.header.code = 0;
    pckt.header.id = pid;

    while (is_running){
      pckt.header.seq_num = htons(cnt);
      pckt.header.checksum = 0;
      ssize_t rsize = load_ping_packet(pckt.message);
      pckt.header.checksum = checksum(&pckt.header, sizeof(pckt.header) + rsize );

      if ( sendto(ping_socket, (char*)&pckt, sizeof(pckt.header) + rsize , 0, (struct sockaddr*)&ping_address, sizeof(ping_address)) <= 0 ){
        ERROR("sendto failed: %d", errno);
        is_running = false;
        return 0;
      }

      INFO("Sended: %d", cnt);
      usleep(ping_period * 1000000);
      cnt++;
    }
    INFO("pinger exited");
    return 0;
  }

  void *listener(void *r){
    INFO("listener started");
    uint8_t buffer[8192];
    struct sockaddr_in received_address;
    ssize_t headers_size = sizeof(ip_header) + sizeof(icmp_header);
    struct timeval tv;
    fd_set readset;

    memset(&buffer, 0, sizeof(buffer));
    while (is_running){
      FD_ZERO(&readset);
      FD_SET(listen_socket, &readset);
      tv.tv_sec = 0;
      tv.tv_usec = 500000;
      int result = select(listen_socket + 1, &readset, NULL, NULL, &tv);
      if (result == 0) continue;
      if (result == -1){
        ERROR("Select error: %d", errno);
        continue;
      }

      socklen_t address_length = sizeof(received_address);
      ssize_t received = recvfrom(listen_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&received_address, &address_length);
      if (received == -1){
        ERROR("Recvfrom error: %d", errno);
        continue;
      }
      if (received < headers_size) continue;
      if (received_address.sin_addr.s_addr != ping_address.sin_addr.s_addr) continue;
      if (((ip_header *)buffer)->protocol != 1) continue;
      icmp_packet *packet = (icmp_packet *)&buffer[20];

      uint16_t sequence = ntohs(packet->header.seq_num);
      INFO("Received: %d", sequence);
      display(packet->message, received - headers_size );
    }
    INFO("listener exited");
    return 0;
  }

  void draw(uint32_t *bitmap_buffer, uint32_t width, uint32_t height){
    buffer = bitmap_buffer;
    bufferx = width;
    buffery = height;
    CNFGBGColor = 0xFF000000;
    CNFGClearFrame();
    CNFGColor( 0xFFFFFFFF );

    DrawFrame();
  }
}

double GetGlobMaxPingTime()
{
  double maxtime = 0;

  for(uint32_t i = 0; i < bufferx; i++ )
  {
    uint32_t index = ((current_cycle - i - 1) + PINGCYCLEWIDTH) & (PINGCYCLEWIDTH-1);
    double st = PingSendTimes[index];
    double rt = PingRecvTimes[index];
    double dt = 0;
    if( rt > st )
    {
      dt = rt - st;
      dt *= 1000;
      if( dt > maxtime ) maxtime = dt;
    }
  }

  return maxtime;
}

void DrawFrame()
{
  double now = OGGetAbsoluteTime();
  double globmaxtime = GetGlobMaxPingTime();

  double totaltime = 0;
  uint32_t totalcountok = 0;
  uint32_t totalcountloss = 0;
  double mintime = 10000;
  double maxtime = 0;
  double stddev = 0;
  double last = -1;
  double loss = 100.00;

  for(uint32_t i = 0; i < bufferx; i++ )
  {
    uint32_t index = ((current_cycle - i - 1) + PINGCYCLEWIDTH) & (PINGCYCLEWIDTH-1);
    double st = PingSendTimes[index];
    double rt = PingRecvTimes[index];

    double dt = 0;

    if( rt > st ) // ping received
    {
      CNFGColor( 0xffffffff );
      dt = rt - st;
      dt *= 1000;
      totaltime += dt;
      if( dt < mintime ) mintime = dt;
      if( dt > maxtime ) maxtime = dt;
      totalcountok++;
      if( last < 0)
        last = dt;
    }
    else if (st != 0) // ping sent but not received
    {
      CNFGColor( 0xff0000ff );
      dt = now - st;
      dt *= 1000;
      totalcountloss++;
    }
    else // no ping sent for this point in time (after startup)
    {
      CNFGColor( 0xff000000 );
      dt = 99 * 1000; // assume 99s to fill screen black
    }

    if (!GuiYscaleFactorIsConstant)
    {
      GuiYScaleFactor =  (buffery - 50) / globmaxtime;
    }

    int32_t top = buffery - dt*GuiYScaleFactor;
    if( top < 0 ) top = 0;
    CNFGTackSegment( i, buffery-1, i, top );
  }

  double avg = totaltime / totalcountok;
  loss = (double) totalcountloss / (totalcountok + totalcountloss) * 100;

  for(uint32_t i = 0; i < bufferx; i++ )
  {
    uint32_t index = ((current_cycle - i - 1) + PINGCYCLEWIDTH) & (PINGCYCLEWIDTH-1);
    double st = PingSendTimes[index];
    double rt = PingRecvTimes[index];

    double dt = 0;
    if( rt > st )
    {
      dt = rt - st;
      dt *= 1000;
      stddev += (dt-avg)*(dt-avg);
    }
  }

  stddev /= totalcountok;

  stddev = sqrt(stddev);

  uint32_t avg_gui    = avg*GuiYScaleFactor;
  uint32_t stddev_gui = stddev*GuiYScaleFactor;

  CNFGColor( 0xff00ff00 );

  uint32_t l = avg_gui;
  CNFGTackSegment( 0, buffery-l, bufferx, buffery - l );
  l = (avg_gui) + (stddev_gui);
  CNFGTackSegment( 0, buffery-l, bufferx, buffery - l );
  l = (avg_gui) - (stddev_gui);
  CNFGTackSegment( 0, buffery-l, bufferx, buffery - l );

  char stbuf[1024];
  char *sptr = &stbuf[0];
  sptr += sprintf( sptr, "Last: %5.2f ms\n", last );
  sptr += sprintf( sptr, "Min : %5.2f ms\n", mintime );
  sptr += sprintf( sptr, "Max : %5.2f ms\n", maxtime );
  sptr += sprintf( sptr, "Avg : %5.2f ms\n", avg );
  sptr += sprintf( sptr, "Std : %5.2f ms\n", stddev );
  sptr += sprintf( sptr, "Loss: %5.1f %%\n", loss );
  CNFGColor( 0xff000000 );
  for(uint32_t x = -1; x < 2; x++ )
    for(uint32_t y = -1; y < 2; y++ )
    {
      CNFGPenX = 10+x; CNFGPenY = 10+y;
      CNFGDrawText( stbuf, 5 ); //TODO font size should depend on DPI
    }
  CNFGColor( 0xffffffff );
  CNFGPenX = 10; CNFGPenY = 10;
  CNFGDrawText( stbuf, 5 );
  OGUSleep( 1000 );
}
