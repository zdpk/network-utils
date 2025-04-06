#pragma once

#include "../l2/if_ether.h"
#include "icmp.h"
#include "ip.h"

#define ICMP_DATA_SIZE 56

struct ip_packet {
    // struct ethhdr eth;
    struct iphdr ip;
    struct icmphdr icmp;
    /* flexible array member for variable length data (above C99) */
    uint8_t data[];
} __attribute__((packed));

void print_ip_packet(struct ip_packet* packet);

