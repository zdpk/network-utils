#pragma once

#include <stdbool.h>
#include <stdint.h>

extern uint16_t calc_checksum(uint16_t* buf, int len);

extern bool verify_checksum(uint16_t* buf, int len, uint16_t checksum);

extern uint16_t calc_udp_checksum(struct iphdr* iph, struct udphdr* udph, uint8_t* data, uint16_t data_len);