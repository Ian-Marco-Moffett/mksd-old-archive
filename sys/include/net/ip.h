#ifndef NET_IP_H_
#define NET_IP_H_

#include <lib/types.h>
#include <lib/asm.h>


#define IPv4(a,b,c,d) ((a << 0) | (b << 8) | (c << 16) | (d << 24))


typedef uint32_t ipv4_address_t;

typedef enum
{
  IP_PROTOCOL_ICMP = 1,
  IP_PROTOCOL_TCP  = 6,
  IP_PROTOCOL_UDP  = 17
} ip_protocol_t;


typedef struct
{
  uint8_t version_ihl;
  uint8_t dscp_ecn;
  uint16_t len;
  uint16_t ident;
  uint16_t flags_fragment;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t checksum;
  ipv4_address_t source;      /* Source address */
  ipv4_address_t dest;        /* Dest address */
} _packed ipv4_header_t;

void ipv4_send(ipv4_address_t dest, ip_protocol_t protocol, 
               uint8_t* payload, unsigned int length);

#endif
