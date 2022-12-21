#ifndef RTL8139_H_
#define RTL8139_H_

#include <net/ethernet.h>

void rtl8139_init(void);
void rtl8139_send_packet(void* data, size_t size);
uint8_t rtl8139_got_packet(void);
uint8_t* rtl8139_read_packet(void);


extern mac_address_t g_rtl8139_mac_addr;

#endif
