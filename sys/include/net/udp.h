#ifndef NET_UDP_H_
#define NET_UDP_H_

#include <net/ip.h>
#include <lib/types.h>


typedef struct
{
  uint16_t source_port;
  uint16_t dest_port;
  uint16_t length;        /* Length of header and data */
} udp_datagram_header_t;

void udp_send_ipv4(ipv4_address_t dest, uint8_t* payload, size_t length,
                   uint16_t source_port, uint16_t dest_port);

#endif
