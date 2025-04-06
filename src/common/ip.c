#include "../../include/ip.h"

#include <arpa/inet.h>
#include <stdio.h>

void print_iphdr(struct iphdr* ip_header) {
  printf("IP Header:\n");
  printf("  Version: %d\n", ip_header->version);
  printf("  Header Length: %d bytes\n", ip_header->ihl * 4);
  printf("  Type of Service: 0x%02x\n", ip_header->tos);
  printf("  Total Length: %d bytes\n", ntohs(ip_header->tot_len));
  printf("  Identification: 0x%04x\n", ntohs(ip_header->id));
  printf("  Flags & Fragment Offset: 0x%04x\n", ntohs(ip_header->frag_off));
  printf("  TTL: %d\n", ip_header->ttl);
  printf("  Protocol: %d (ICMP)\n", ip_header->protocol);
  printf("  Checksum: 0x%04x\n", ntohs(ip_header->check));

  struct in_addr addr;
  addr.s_addr = ip_header->saddr;
  printf("  Source IP: %s\n", inet_ntoa(addr));
  addr.s_addr = ip_header->daddr;
  printf("  Destination IP: %s\n", inet_ntoa(addr));
  printf("\n");
}