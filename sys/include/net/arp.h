#ifndef NET_ARP_H_
#define NET_ARP_H_

#include <net/ethernet.h>
#include <net/ip.h>
#include <lib/types.h>
#include <lib/asm.h>


#define ARP_HW_ETHERNET 1

#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2


typedef struct
{
  uint16_t htype;
  uint16_t ptype;
  uint8_t haddr_len;
  uint8_t paddr_len;
  uint16_t op;

  mac_address_t sender_haddr;
  ipv4_address_t sender_paddr;
  mac_address_t target_haddr;
  ipv4_address_t target_paddr;
} _packed arp_packet_t;

mac_address_t* arp_resolve(ipv4_address_t target_paddr);

#endif
