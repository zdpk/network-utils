#pragma once

#include <stddef.h>
#include <stdint.h>

#define IPVERSION 4

struct iphdr {
  uint8_t ihl:4, version:4;
  uint8_t tos; /* type of service */
  uint16_t tot_len; /* total length */
  uint16_t id; /* identification */
  uint16_t frag_off; /* fragment offset */
  uint8_t ttl; /* time to live */
  uint8_t protocol; 
  uint16_t check;  /* checksum */
  uint32_t saddr; /* source address */
  uint32_t daddr; /* destination address */
} __attribute__((packed));

extern void print_iphdr(struct iphdr* ip_header);
