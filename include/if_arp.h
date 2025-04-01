#include <stdint.h>
#ifndef _IF_ETHER_H
  #include "if_ether.h"
#endif

typedef unsigned short __be16;
#define ARP_HLEN 28
#define ARPOP_REQUEST 1
#define ARPOP_REPLY 2
#define ARPHRD_ETHER 1

struct arphdr {
    __be16 ar_hrd;        /* format of hardware address */
    __be16 ar_pro;        /* format of protocol address */
    uint8_t ar_hln; /* length of hardware address */
    uint8_t ar_pln; /* length of protocol address */
    __be16 ar_op;         /* ARP opcode (command) */

    uint8_t ar_sha[ETH_ALEN]; /* sender hardware address */
    uint8_t ar_sip[4];        /* sender IP address */
    uint8_t ar_tha[ETH_ALEN]; /* target hardware address */
    uint8_t ar_tip[4];        /* target IP address */
} __attribute__((packed));

void print_arphdr(const struct arphdr *arp) {
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
