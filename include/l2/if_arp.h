#pragma once

#include <stdint.h>
#include "if_ether.h"

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

void print_arphdr(const struct arphdr* arp);
