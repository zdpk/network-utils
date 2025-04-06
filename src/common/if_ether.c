#include "../../include/if_ether.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

void print_mac(const uint8_t* mac_addr) {
  printf("%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1], mac_addr[2],
         mac_addr[3], mac_addr[4], mac_addr[5]);
}

void print_ip(const uint8_t* ip_addr) {
  struct in_addr in_addr;
  memcpy(&in_addr.s_addr, ip_addr, 4);
  printf("%s", inet_ntoa(in_addr));
}

void print_ethhdr(const struct ethhdr* eth) {
  printf("=== Ethernet Header ===\n");
  printf("Destination MAC: ");
  print_mac(eth->h_dest);
  printf("\n");

  printf("Source MAC: ");
  print_mac(eth->h_source);
  printf("\n");

  printf("Type of Protocol: 0x%04x", ntohs(eth->h_proto));
  switch (ntohs(eth->h_proto)) {
    case ETH_P_IP:
      printf(" (IPv4)");
      break;
    case ETH_P_ARP:
      printf(" (ARP)");
      break;
    case 0x86DD:
      printf(" (IPv6)");
      break;
    default:
      printf(" (Unknown)");
  }
  printf("\n");
}
