#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "../../include/interface.h"
#include "../../include/ip_packet.h"

uint16_t calc_checksum(uint16_t* buf, int len) {
  uint32_t sum = 0;
  while (len > 1) {
    /* Accumulate the sum by adding 16-bit units until 0-1 bytes remain */
    sum += *buf++;
    len -= 2;
  }

  /* if buf is odd, add the last byte to the sum */
  if (len == 1) {
    sum += *(uint8_t*)buf;
  }

  /* 상위 16bits를 하위 16bits로 옮긴 값과 하위 16bits를 더한다 */
  sum = (sum >> 16) + (sum & 0x0000FFFF);
  /* 이 때 더함으로 인해 상위 16bits 쪽에 다시 carry가 생겼을 수도 있으니 그
   * 부분만 하위 16bits에 더한다 */
  sum += (sum >> 16);

  /* 최종 값의 하위 16bits 부분을 1의 보수로 변환 및 반환 */
  return (uint16_t)(~sum);
}

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

/* create raw socket for ICMP protocol */
int create_socket() {
  int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (socket_fd < 0) {
    perror("failed to create socket");
    return -1;
  }
  return socket_fd;
}

int init_sockaddr_in(struct sockaddr_in* addr, const char* ip_addr_str) {
  addr->sin_family = AF_INET;
  /* 0 is wildcard port */
  addr->sin_port = 0;
  addr->sin_addr.s_addr = inet_addr(ip_addr_str);
  return 0;
}

int send_icmp_echo_request(int socket_fd, struct sockaddr_in* src_addr,
                           struct sockaddr_in* dest_addr, int sequence) {
  // Allocate memory for the entire packet structure including the flexible
  // array member
  uint8_t ip_packet_buf[sizeof(struct ip_packet) + ICMP_DATA_SIZE];
  memset(ip_packet_buf, 0, sizeof(struct ip_packet) + ICMP_DATA_SIZE);
  struct ip_packet* packet = (struct ip_packet*)ip_packet_buf;

  if (!packet) {
    perror("Failed to allocate packet memory");
    return -1;
  }

  /* initialize ip header */
  packet->ip.version = IPVERSION; /* version of IP */
  packet->ip.ihl = 5;             /* default header length (20bytes) */
  packet->ip.tos = 0;             /* type of service */
  packet->ip.tot_len = htons(sizeof(struct ip_packet) +
                             ICMP_DATA_SIZE);   /* total length of IP packet */
  packet->ip.id = htons(getpid() & 0x0000FFFF); /* identifier of ICMP */
  packet->ip.frag_off = 0;                      /* fragment offset */
  /* usually set to 64 or 128 */
  packet->ip.ttl = 64;                           /* time to live */
  packet->ip.protocol = IPPROTO_ICMP;            /* protocol of IP */
  packet->ip.check = 0;                          /* checksum of IP */
  packet->ip.saddr = src_addr->sin_addr.s_addr;  /* source address */
  packet->ip.daddr = dest_addr->sin_addr.s_addr; /* destination address */

  /* initialize icmp header */
  packet->icmp.type = ICMP_ECHO;                  /* type of ICMP */
  packet->icmp.code = 0;                          /* code of ICMP */
  packet->icmp.checksum = 0;                      /* checksum of ICMP */
  packet->icmp.id = htons(getpid() & 0x0000FFFF); /* identifier of ICMP */
  packet->icmp.sequence = htons(sequence);        /* sequence number of ICMP */

  /* icmp data (56bytes) */
  uint32_t* timestamp = (uint32_t*)(packet->data);
  struct timeval tv;
  gettimeofday(&tv, NULL);
  *timestamp++ = htonl(tv.tv_sec);
  *timestamp++ = htonl(tv.tv_usec);
  /* timestamp is 8 bytes */

  /* rest of data is filled with "abc..." pattern */
  int size = ICMP_DATA_SIZE - 8;
  uint8_t* data_ptr = (uint8_t*)(packet->data + 8);
  for (int i = 0; i < size; i++) {
    data_ptr[i] = (i % 26) + 'a';
  }

  /* calculate checksum */
  packet->ip.check = htons(calc_checksum(&packet->ip, sizeof(struct iphdr)));
  packet->icmp.checksum = calc_checksum(
      (uint16_t*)&packet->icmp, sizeof(struct icmphdr) + ICMP_DATA_SIZE);

  print_ip_packet(packet);
  printf("----------------------------------\n\n");

  /*
    send ICMP Echo request

    partial write will not be occurred
    because the packet size is small and fixed
    and the socket is raw socket
  */
  // int one = 1;
  // if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
  //   perror("Error setting IP_HDRINCL");
  //   return -1;
  // }
  if (sendto(socket_fd, (uint8_t*)packet,
             sizeof(struct ip_packet) + ICMP_DATA_SIZE, 0,
             (struct sockaddr*)dest_addr, sizeof(struct sockaddr_in)) < 0) {
    perror("failed to send ICMP Echo request");
    return -1;
  }

  return 0;
}

int main(int argc, char** argv) {
  int socket_fd;
  struct sockaddr_in src_addr;
  struct sockaddr_in dest_addr;
  struct timeval timeout;
  char send_buf[1024];
  char recv_buf[1024];
  int send_len;
  char target_ip[INET_ADDRSTRLEN];
  struct timespec current_time, next_time;
  int sequence = 1;
  struct interface_t interface;
  const char* interface_name = "eth0";

  /* 1. parse arguments */
  if (parse_args(argc, argv, target_ip) < 0) {
    perror("failed to parse arguments");
    exit(1);
  }

  /* 2. initialize interface */
  if (init_interface(&interface, interface_name) < 0) {
    perror("failed to initialize interface");
    exit(1);
  }

  /* 3. create raw socket */
  socket_fd = create_socket();
  if (socket_fd < 0) {
    perror("failed to create socket");
    exit(1);
  }

  /* 4. convert target IP address */
  init_sockaddr_in(&dest_addr, target_ip);

  /* 5. initialize sockaddr_ll */

  /* 6. send ICMP Echo request */
  clock_gettime(CLOCK_MONOTONIC, &current_time);
  next_time = current_time;

  if (send_icmp_echo_request(socket_fd, &interface.ip_addr, &dest_addr,
                             sequence++) < 0) {
    perror("failed to send ICMP Echo request");
    exit(1);
  }

  // while (1) {
  //   if (send_icmp_echo_request(socket_fd, &src_addr, &dest_addr, sequence++)
  //   <
  //       0) {
  //     perror("failed to send ICMP Echo request");
  //     exit(1);
  //   }

  //   next_time.tv_sec += 1;
  //   clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_time, NULL);
  // }

  // /* bind raw socket to interface */
  // /* 7. receive ARP reply */
  return 0;
}