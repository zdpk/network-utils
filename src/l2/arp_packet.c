#include "../../include/l2/arp_packet.h"

#include "../../include/l2/if_ether.h"

void print_arp_packet(const struct arp_packet* packet) {
  const struct ethhdr* eth = &packet->eth;
  const struct arphdr* arp = &packet->arp;

  printf("\n");
  printf("---------- ARP Packet Information ----------\n");
  print_ethhdr(eth);
  printf("\n");
  print_arphdr(arp);
  printf("----------------------------------\n\n");
}
