#pragma once

#include "if_ether.h"
#include "if_arp.h"

struct arp_packet {
    struct ethhdr eth;     
    struct arphdr arp;     
} __attribute__((packed)); 

void print_arp_packet(const struct arp_packet* packet);