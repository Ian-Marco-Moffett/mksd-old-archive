#ifndef NET_ICMP_H_
#define NET_ICMP_H_

#include <lib/asm.h>
#include <lib/types.h>
#include <net/ip.h>

#define ECHO_REQ_TYPE 8
#define ECHO_REQ_CODE 0

#define ECHO_REPLY_TYPE 0
#define ECHO_REPLY_CODE 0


typedef struct
{
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
} _packed icmp_header_t;


void icmp_send_msg_ipv4(ipv4_address_t dest, uint8_t type,
                        uint8_t code, uint8_t* payload,
                        size_t length);

#endif
