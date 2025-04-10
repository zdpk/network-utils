#pragma once

#include <stdint.h>

#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO 8 /* ICMP Echo Request */

struct icmphdr {
  uint8_t type;      // Message type
  uint8_t code;      // Message code
  uint16_t checksum; // Checksum
  uint16_t id;       // Identifier
  uint16_t sequence; // Sequence number
} __attribute__((packed));

extern void print_icmphdr(struct icmphdr *icmp_header);


