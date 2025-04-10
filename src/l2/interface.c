#include "../../include/l2/interface.h"

#include <arpa/inet.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

extern int init_interface(struct interface_t* interface,
                          const char* interface_name) {
  struct ifreq ifr = {
      0,
  };

  int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
  if (sock_fd < 0) {
    perror("failed to create a raw socket");
    return -1;
  }
  strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);

  /*
    once `ioctl` is called, ifr's previous values are overwritten
    need to save the previous values before calling `ioctl`
   */
  if (ioctl(sock_fd, SIOCGIFINDEX, &ifr) < 0) {
    perror("failed to get interface index");
    return -1;
  }
  interface->index = ifr.ifr_ifindex;
  if (ioctl(sock_fd, SIOCGIFHWADDR, &ifr) < 0) {
    perror("failed to get hardware address");
    return -1;
  }
  memcpy(interface->mac_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

  if (ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0) {
    perror("failed to get IP address");
    return -1;
  }
  memcpy(&interface->ip_addr, (struct sockaddr_in*)&ifr.ifr_addr,
         sizeof(struct sockaddr_in));
  print_interface_info(interface);
  return 0;
}

extern void print_interface_info(const struct interface_t* interface) {
  printf("========== Interface Info ==========\n");
  printf("Index      : %d\n", interface->index);
  printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", interface->mac_addr[0],
         interface->mac_addr[1], interface->mac_addr[2], interface->mac_addr[3],
         interface->mac_addr[4], interface->mac_addr[5]);
  printf("IP Address : %s\n", inet_ntoa(interface->ip_addr.sin_addr));
  printf("====================================\n");
}
