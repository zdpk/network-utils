#include "../../include/l3/icmp.h"

#include <arpa/inet.h>
#include <stdio.h>

extern void print_icmphdr(struct icmphdr* icmp_header) {
  printf("ICMP Header:\n");
  printf("  Type: %d", icmp_header->type);
  if (icmp_header->type == ICMP_ECHO) {
    printf(" (Echo Request)\n");
  } else if (icmp_header->type == ICMP_ECHO_REPLY) {
    printf(" (Echo Reply)\n");
  } else {
    printf(" (Other)\n");
  }
  printf("  Code: %d\n", icmp_header->code);
  printf("  Checksum: 0x%04x\n", ntohs(icmp_header->checksum));
  printf("  Identifier: 0x%04x\n", ntohs(icmp_header->id));
  printf("  Sequence Number: %d\n", ntohs(icmp_header->sequence));
  printf("\n");
}