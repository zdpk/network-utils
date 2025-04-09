#pragma once

#include <stdint.h>
#include <netinet/in.h>
#include "if_ether.h"

struct interface_t {
  int index;
  uint8_t mac_addr[ETH_ALEN];
  struct sockaddr_in ip_addr;
};

int init_interface(struct interface_t* interface, const char* interface_name);
void print_interface_info(const struct interface_t* interface);
