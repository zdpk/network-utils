#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

static void print_ifreq(struct ifreq *ifr) {
  printf("Interface: %s\n", ifr->ifr_name);
  printf("Interface Index: %d\n", ifr->ifr_ifindex);
  printf("Hardware Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
         ifr->ifr_hwaddr.sa_data[0], ifr->ifr_hwaddr.sa_data[1],
         ifr->ifr_hwaddr.sa_data[2], ifr->ifr_hwaddr.sa_data[3],
         ifr->ifr_hwaddr.sa_data[4], ifr->ifr_hwaddr.sa_data[5]);
}

int init_if_req(struct ifreq *ifr, int sock_fd, const char *if_name) {
  char if_name_buf[IFNAMSIZ] = {
      0,
  };

  if (ioctl(sock_fd, SIOCGIFINDEX, ifr) < 0) {
    perror("failed to get interface index");
    return -1;
  }
  strncpy(if_name_buf, ifr->ifr_name, IFNAMSIZ - 1);
  print_ifreq(ifr);

  if (ioctl(sock_fd, SIOCGIFHWADDR, ifr) < 0) {
    perror("failed to get hardware address");
    return -1;
  }
  strncpy(if_name_buf, ifr->ifr_name, IFNAMSIZ - 1);
  print_ifreq(ifr);

  if (ioctl(sock_fd, SIOCGIFADDR, ifr) < 0) {
    perror("failed to get IP address");
    return -1;
  }
  strncpy(if_name_buf, ifr->ifr_name, IFNAMSIZ - 1);
  print_ifreq(ifr);
  return 0;
}