#include <net/ip.h>
#include <net/arp.h>
#include <net/ethernet.h>
#include <mm/heap.h>
#include <lib/log.h>
#include <lib/math.h>
#include <lib/string.h>

#define IP_DEBUG      1
#define VERSION_4     4
#define HDR_LEN       5

static uint16_t 
convert(uint16_t os) 
{
  uint8_t* s = (uint8_t*)&os;
  return (uint16_t)(s[0] << 8 | s[1]);
}

static uint16_t 
internet_checksum(void* ptr, size_t count) 
{
  uint32_t checksum = 0;
  uint16_t* w = (uint16_t*)ptr;

  while (count > 1) 
  {
      checksum += convert(*w++);

      if (checksum & 0x80000000)
      {
        checksum = (checksum & 0xffff) | (checksum >> 16);
      }

      count -= 2;
  }

  while (checksum >> 16)
  {
    checksum = (checksum & 0xffff) + (checksum >> 16);
  }
  return convert(~checksum);
}

void 
ipv4_send(ipv4_address_t dest, ip_protocol_t protocol, 
          uint8_t* payload, unsigned int length)
{
  mac_address_t dest_mac;
  mac_address_t* dest_mac_ptr = arp_resolve(dest);

  if (dest_mac_ptr == NULL)
  {
    if (IP_DEBUG)
    {
      printk(PRINTK_WARN "IP: Could not resolve %d.%d.%d.%d\n",
             (dest >> 0) & 0xFF,
             (dest >> 8) & 0xFF,
             (dest >> 16) & 0xFF,
             (dest >> 24) & 0xFF);
    }

    return;
  }

  memcpy(dest_mac, dest_mac_ptr, sizeof(mac_address_t));
  unsigned int size = sizeof(ipv4_header_t) + length;

  /* Construct the IPv4 header */
  ipv4_header_t* hdr = kmalloc(size);
  hdr->version_ihl = (VERSION_4 << 4) | HDR_LEN;
  hdr->len = BIG_ENDIAN(size);
  hdr->ident = BIG_ENDIAN(1);
  hdr->ttl = 64;
  hdr->protocol = protocol;
  hdr->checksum = internet_checksum(hdr, sizeof(ipv4_header_t));

  hdr->source = IPv4(192, 168, 1, 166);
  hdr->dest = dest;

  /* Copy the payload and send it off! */
  memcpy((void*)((uint64_t)hdr + sizeof(ipv4_header_t)), payload, length);
  ethernet_send(dest_mac, ETHERTYPE_IPV4, (uint8_t*)hdr, size);
}
