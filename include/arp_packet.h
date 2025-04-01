#include "if_ether.h"
#include "if_arp.h"
#include <stdio.h>

struct arp_packet {
    struct ethhdr eth;     
    struct arphdr arp;     
} __attribute__((packed)); 

void print_arp_packet(const struct arp_packet *packet) {
    const struct ethhdr *eth = &packet->eth;
    const struct arphdr *arp = &packet->arp;
    
    printf("\n");
    printf("---------- ARP Packet Information ----------\n");
    print_ethhdr(eth);
    printf("\n");
    print_arphdr(arp);
    printf("----------------------------------\n\n");
}
