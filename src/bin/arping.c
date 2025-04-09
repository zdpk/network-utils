#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../../include/l2/arp_packet.h"
#include "../../include/l2/if_arp.h"
#include "../../include/l2/if_ether.h"
#include "../../include/l2/interface.h"

void print_usage(const char* program_name) {
  printf("Usage: %s <target_ip>\n", program_name);
}

int parse_args(int argc, char** argv, char* target_ip) {
  if (argc != 2) {
    print_usage(argv[0]);
    exit(1);
  }
  strncpy(target_ip, argv[1], INET_ADDRSTRLEN - 1);
  return 0;
}

int create_raw_socket() {
  int socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));

  if (socket_fd < 0) {
    perror("failed to create a raw socket");
    exit(1);
  }
  return socket_fd;
}

int init_sockaddr_ll(struct sockaddr_ll* sockaddr_ll,
                     const struct interface_t* interface) {
  sockaddr_ll->sll_family = AF_PACKET;
  sockaddr_ll->sll_ifindex = interface->index;
  sockaddr_ll->sll_protocol = ETH_P_ARP;
  // sockaddr_ll->sll_hatype = ARPHRD_ETHER;
  // sockaddr_ll->sll_pkttype = PACKET_BROADCAST;
  // sockaddr_ll->sll_halen = ETH_ALEN;
  // memset(sockaddr_ll->sll_addr, 0xFF, ETH_ALEN);
  // memcpy(sockaddr_ll->sll_addr, interface->mac_addr, ETH_ALEN);
  return 0;
}

int bind_raw_socket(int socket_fd, struct sockaddr_in* source_addr,
                    socklen_t source_addr_len) {
  if (bind(socket_fd, (struct sockaddr*)source_addr, source_addr_len) < 0) {
    perror("failed to bind raw socket to interface");
    return -1;
  }
  return 0;
}

int send_arp_request(int socket_fd, struct interface_t* interface,
                     const struct in_addr* target_ip
                     //  struct sockaddr_ll *sockaddr_ll,
                     //  socklen_t sockaddr_ll_len
) {
  struct sockaddr_ll sockaddr_ll = {
      0,
  };
  struct arp_packet packet = {
      0,
  };

  /* set Ethernet header */
  memcpy(packet.eth.h_source, interface->mac_addr, ETH_ALEN);
  memset(packet.eth.h_dest, 0xFF, ETH_ALEN);
  packet.eth.h_proto = htons(ETH_P_ARP);

  /* set ARP header */
  packet.arp.ar_hrd = htons(ARPHRD_ETHER);
  packet.arp.ar_pro = htons(ETH_P_IP);
  packet.arp.ar_hln = ETH_ALEN;
  packet.arp.ar_pln = 4;
  packet.arp.ar_op = htons(ARPOP_REQUEST);

  sockaddr_ll.sll_ifindex = interface->index;
  sockaddr_ll.sll_protocol = htons(ETH_P_ARP);
  sockaddr_ll.sll_family = AF_PACKET;

  print_arp_packet(&packet);

  /* send ARP request */
  if (sendto(socket_fd, &packet, sizeof(packet), 0,
             (struct sockaddr*)&sockaddr_ll, sizeof(sockaddr_ll)) < 0) {
    return -1;
  }
  return 0;
}

int recv_arp_reply(int socket_fd, struct interface_t* interface,
                   const struct in_addr* source_ip) {
  struct arp_packet packet = {
      0,
  };
  struct sockaddr_ll sockaddr_ll = {
      0,
  };
  /*
    Kernel will fill in the sockaddr_ll structure when the socket receives a
    packet No need to set it manually and it can be reused for the next packet
  */
  socklen_t sockaddr_ll_len = sizeof(sockaddr_ll);

  while (1) {
    ssize_t recv_len =
        recvfrom(socket_fd, &packet, sizeof(struct arp_packet), 0,
                 (struct sockaddr*)&sockaddr_ll, &sockaddr_ll_len);
    printf("recv_len: %zd\n", recv_len);
    print_arp_packet(&packet);
    if (recv_len < sizeof(struct arp_packet)) {
      perror("failed to receive ARP reply");
      continue;
    }

    /* check if the ARP reply is for me */
    if (memcmp(&packet.eth.h_dest, interface->mac_addr, ETH_ALEN) != 0) {
      fprintf(stderr, "ARP reply is not for me\n");
      continue;
    }

    /* check if the ARP reply IP address is the source IP address */
    if (memcmp(&packet.arp.ar_sip, source_ip, 4) != 0) {
      fprintf(stderr, "ARP reply IP address is not the source IP address\n");
      continue;
    }

    /* check if the ARP reply IP address is the target IP address */
    if (memcmp(&packet.arp.ar_tip, &interface->ip_addr.sin_addr.s_addr, 4) !=
        0) {
      fprintf(stderr, "ARP reply IP address is not the target IP address\n");
      continue;
    }

    /* check if the ARP reply is a reply */
    if (packet.arp.ar_op != htons(ARPOP_REPLY)) {
      fprintf(stderr, "ARP reply is not a reply\n");
      continue;
    }

    print_arp_packet(&packet);
    printf("ARP reply received\n");
    return 0;
  }
}

int main(int argc, char** argv) {
  struct interface_t interface;
  const char interface_name[IFNAMSIZ] = "eth0";
  const char target_ip_str[INET_ADDRSTRLEN] = {
      0,
  };
  struct sockaddr_ll sockaddr_ll;
  socklen_t sockaddr_ll_len = sizeof(sockaddr_ll);
  struct sockaddr_in source_addr;
  socklen_t source_addr_len = sizeof(source_addr);
  struct in_addr target_ip = {
      0,
  };

  /* 1. parse arguments */
  if (parse_args(argc, argv, (char*)target_ip_str) < 0) {
    perror("failed to parse arguments");
    exit(1);
  }

  /* 2. initialize interface */
  if (init_interface(&interface, interface_name) < 0) {
    perror("failed to initialize interface");
    exit(1);
  }

  /* 3. create raw socket */
  int socket_fd = create_raw_socket();
  if (socket_fd < 0) {
    perror("failed to create a raw socket");
    exit(1);
  }

  /* 4. convert target IP address */
  if (inet_aton(target_ip_str, &target_ip) == 0) {
    perror("failed to convert target IP address");
    return -1;
  }

  /* 5. initialize sockaddr_ll */
  if (init_sockaddr_ll(&sockaddr_ll, &interface) < 0) {
    perror("failed to initialize sockaddr_ll");
    exit(1);
  }

  /* 6. send ARP request */
  if (send_arp_request(socket_fd, &interface, &target_ip) < 0) {
    perror("failed to send ARP request");
    exit(1);
  }

  /* bind raw socket to interface */
  // if (bind_raw_socket(socket_fd, &source_addr, source_addr_len) < 0) {
  //   perror("failed to bind raw socket to interface");
  //   exit(1);
  // }

  /* 7. receive ARP reply */
  if (recv_arp_reply(socket_fd, &interface, &target_ip) < 0) {
    perror("failed to receive ARP reply");
    exit(1);
  }

  return 0;
}
