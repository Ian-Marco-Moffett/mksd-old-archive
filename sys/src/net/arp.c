#include <net/arp.h>
#include <drivers/net/rtl8139.h>
#include <mm/heap.h>
#include <lib/math.h>
#include <lib/string.h>
#include <lib/log.h>


#define ARP_HW_ETHERNET 1
#define ARP_DEBUG 1


mac_address_t*
arp_resolve(ipv4_address_t target_paddr)
{
  arp_packet_t* packet = kmalloc(sizeof(arp_packet_t));
  packet->htype = BIG_ENDIAN(ARP_HW_ETHERNET);
  packet->ptype = BIG_ENDIAN(ETHERTYPE_IPV4);
  packet->haddr_len = sizeof(mac_address_t);
  packet->paddr_len = sizeof(ipv4_address_t);
  packet->op = BIG_ENDIAN(ARP_OP_REQUEST);

  /* Target haddr shall be broadcast (FF:FF:FF:FF:FF:FF) */
  memset(packet->target_haddr, 0xFF, sizeof(mac_address_t));
  packet->sender_paddr = IPv4(0, 0, 0, 0);

  /* 
   * Copy the NIC's MAC address into
   * the sender_haddr field.
   */
  memcpy(packet->sender_haddr, g_rtl8139_mac_addr, sizeof(mac_address_t));
  packet->target_paddr = target_paddr;

  /* Send off the packet! */
  ethernet_send(packet->target_haddr, ETHERTYPE_ARP,
                (uint8_t*)packet, sizeof(arp_packet_t)); 

  kfree(packet);
  ssize_t spin = 10000000;

  while (!(rtl8139_got_packet()) && spin--);
  if (spin <= 0) return NULL;

  /* Skip ethernet header */
  arp_packet_t* pkt = (arp_packet_t*)((uintptr_t)rtl8139_read_packet()
                                      + sizeof(ethernet_header_t));
  if (pkt->op == BIG_ENDIAN(ARP_OP_REPLY))
  {
    if (ARP_DEBUG)
    {
      printk(PRINTK_INFO 
             "ARP: Got reply; %d.%d.%d.%d is at %X:%X:%X:%X:%X:%X.\n",
             (pkt->sender_paddr >> 0) & 0xFF, 
             (pkt->sender_paddr >> 8) & 0xFF, 
             (pkt->sender_paddr >> 16) & 0xFF, 
             (pkt->sender_paddr >> 24) & 0xFF,
             pkt->sender_haddr[0], 
             pkt->sender_haddr[1], 
             pkt->sender_haddr[2], 
             pkt->sender_haddr[3], 
             pkt->sender_haddr[4], 
             pkt->sender_haddr[5]);
    } 
    else 
    {
      return NULL;
    }
  }

  mac_address_t* mac = kmalloc(sizeof(mac_address_t));
  memcpy(mac, pkt->sender_haddr, sizeof(mac_address_t));
  return mac;
}
