#include <net/icmp.h>
#include <net/checksum.h>
#include <mm/heap.h>
#include <lib/string.h>


void 
icmp_send_msg_ipv4(ipv4_address_t dest, uint8_t type,
                   uint8_t code, uint8_t* payload,
                   size_t length)
{
  size_t size = sizeof(icmp_header_t) + length;
  icmp_header_t* hdr = kmalloc(size);
  hdr->type = type;
  hdr->code = code;
  memcpy((uint8_t*)((uint64_t)hdr + sizeof(icmp_header_t)), 
         payload, length);

  hdr->checksum = internet_checksum(hdr, size);

  ipv4_send(dest, IP_PROTOCOL_ICMP, (uint8_t*)hdr, size);
  kfree(hdr);
}
