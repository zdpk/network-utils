#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ETH_ALEN 6
#define ETH_HLEN 14
#define ETH_P_ARP 0x0806
#define ETH_P_IP 0x0800

 struct ethhdr {
    uint8_t h_dest[6];
    uint8_t h_source[6];
    uint16_t h_proto;
} __attribute__((packed));

void print_ip(const uint8_t* ip_addr);
void print_mac(const uint8_t* mac_addr);
void print_ethhdr(const struct ethhdr* eth);

