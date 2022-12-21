#ifndef NET_ETHERNET_H_
#define NET_ETHERNET_H_

#include <lib/types.h>

typedef uint8_t mac_address_t[6];

typedef enum
{
  ETHERNET_ARP  = 0x0806,
  ETHERNET_IPV4 = 0x0800,
  ETHERNET_IPV6 = 0x86DD
} ethertype_t;


typedef struct
{
  mac_address_t dest;
  mac_address_t src;
  uint16_t ether_type;
  uint8_t payload[1];
} ethernet_header_t;

void ethernet_send(mac_address_t dest, ethertype_t ethertype, uint8_t* data, unsigned int length);

#endif
