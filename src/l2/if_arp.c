#include <netinet/in.h>

#include "../../include/l2/arp_packet.h"
#include "../../include/l2/if_ether.h"

extern void print_arphdr(const struct arphdr* arp) {
  printf("=== ARP HEADER ===\n");

  printf("Hardware type: %d", ntohs(arp->ar_hrd));
  if (ntohs(arp->ar_hrd) == ARPHRD_ETHER) {
    printf(" (Ethernet)");
  }
  printf("\n");

  printf("Protocol type: 0x%04x", ntohs(arp->ar_pro));
  if (ntohs(arp->ar_pro) == ETH_P_IP) {
    printf(" (IPv4)");
  }
  printf("\n");

  printf("Hardware address length: %d\n", arp->ar_hln);
  printf("Protocol address length: %d\n", arp->ar_pln);

  printf("Operation: %d", ntohs(arp->ar_op));
  switch (ntohs(arp->ar_op)) {
    case ARPOP_REQUEST:
      printf(" (Request)");
      break;
    case ARPOP_REPLY:
      printf(" (Reply)");
      break;
    default:
      printf(" (Other)");
  }
  printf("\n");

  printf("Sender MAC: ");
  print_mac(arp->ar_sha);
  printf("\n");

  printf("Sender IP: ");
  print_ip(arp->ar_sip);
  printf("\n");

  printf("Target MAC: ");
  print_mac(arp->ar_tha);
  printf("\n");

  printf("Target IP: ");
  print_ip(arp->ar_tip);
  printf("\n");
}
