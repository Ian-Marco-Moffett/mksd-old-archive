#include <net/ethernet.h>
#include <mm/heap.h>
#include <lib/string.h>
#include <lib/math.h>
#include <drivers/net/rtl8139.h>


void
ethernet_send(mac_address_t dest, ethertype_t ethertype, uint8_t* data, unsigned int length)
{
  unsigned int size = length + sizeof(ethernet_header_t);
  ethernet_header_t* hdr = kmalloc(size);
  if (dest)
  {
    memcpy(hdr->dest, dest, sizeof(mac_address_t));
  }
  else
  {
    /* Broadcast */
    memset(hdr->dest, 0xFF, sizeof(mac_address_t));
  }

  hdr->ether_type = BIG_ENDIAN(ethertype);
  memcpy(hdr->payload, data, length);
  rtl8139_send_packet(hdr, size);
  kfree(hdr);
}
