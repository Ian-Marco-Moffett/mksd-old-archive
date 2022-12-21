#include <net/udp.h>
#include <mm/heap.h>
#include <lib/string.h>
#include <lib/math.h>


void 
udp_send_ipv4(ipv4_address_t dest, uint8_t* payload, size_t length,
                   uint16_t source_port, uint16_t dest_port)
{
  size_t size = sizeof(udp_datagram_header_t) + length;
  udp_datagram_header_t* hdr = kmalloc(size);
  hdr->source_port = BIG_ENDIAN(source_port);
  hdr->dest_port = BIG_ENDIAN(dest_port);
  hdr->length = BIG_ENDIAN(size);
  memcpy((uint8_t*)((uint64_t)hdr + sizeof(udp_datagram_header_t)),
         payload, length);

  ipv4_send(dest, IP_PROTOCOL_UDP, (uint8_t*)hdr, size);
  kfree(hdr);
}
