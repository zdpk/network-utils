#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../../include/l2/interface.h"
#include "../../include/l3/ip_packet.h"
#include "../../include/utils/checksum.h"

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
  inet_pton(AF_INET, ip_addr_str, &addr->sin_addr);
  // addr->sin_addr.s_addr = inet_addr(ip_addr_str);
  return 0;
}

int send_icmp_echo_request(int socket_fd, struct sockaddr_in* src_addr,
                           struct sockaddr_in* dest_addr, int sequence) {
  // Allocate memory for the entire packet structure including the flexible
  // array member
  uint8_t ip_packet_buf[sizeof(struct ip_packet) + ICMP_DATA_SIZE];
  memset(ip_packet_buf, 0, sizeof(struct ip_packet) + ICMP_DATA_SIZE);
  struct ip_packet* packet = (struct ip_packet*)ip_packet_buf;

  /* initialize ip header */
  packet->ip.version = IPVERSION; /* version of IP */
  packet->ip.ihl = 5;             /* default header length (20bytes) */
  packet->ip.tos = 0;             /* type of service */
  packet->ip.tot_len = htons(sizeof(struct ip_packet) +
                             ICMP_DATA_SIZE); /* total length of IP packet */
  packet->ip.id = htons(getpid());            /* identifier of ICMP */
  packet->ip.frag_off = 0;                    /* fragment offset */
  /* usually set to 64 or 128 */
  packet->ip.ttl = 64;                           /* time to live */
  packet->ip.protocol = IPPROTO_ICMP;            /* protocol of IP */
  packet->ip.check = 0;                          /* checksum of IP */
  packet->ip.saddr = src_addr->sin_addr.s_addr;  /* source address */
  packet->ip.daddr = dest_addr->sin_addr.s_addr; /* destination address */

  /* initialize icmp header */
  packet->icmp.type = ICMP_ECHO;           /* type of ICMP */
  packet->icmp.code = 0;                   /* code of ICMP */
  packet->icmp.checksum = 0;               /* checksum of ICMP */
  packet->icmp.id = htons(getpid());       /* identifier of ICMP */
  packet->icmp.sequence = htons(sequence); /* sequence number of ICMP */

  /* icmp data (56bytes) */
  uint32_t* timestamp = (uint32_t*)(packet->data);
  struct timeval tv;
  gettimeofday(&tv, NULL);
  *timestamp++ = htonl(tv.tv_sec);
  *timestamp = htonl(tv.tv_usec);
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
  // printf("----------------------------------\n\n");

  /*
    send ICMP Echo request

    partial write will not be occurred
    because the packet size is small and fixed
    and the socket is raw socket
  */
  int one = 1;
  /*
    IP_HDRINCL - ip header include (manually)
    if `SOCK_DGRAM` is used, Kernel will automatically add IP header
    but if `SOCK_RAW` is used, we need to manually add IP header
    in this case, `IP_HDRINCL` option is required for adding IP header that I
    wrote manually
   */
  if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
    perror("Error setting IP_HDRINCL");
    return -1;
  }
  if (sendto(socket_fd, (uint8_t*)packet,
             sizeof(struct ip_packet) + ICMP_DATA_SIZE, 0,
             (struct sockaddr*)dest_addr, sizeof(struct sockaddr_in)) < 0) {
    perror("failed to send ICMP Echo request");
    return -1;
  }

  return 0;
}

int recv_icmp_echo_reply(int socket_fd, struct sockaddr_in* src_addr,
                         struct sockaddr_in* dest_addr) {
  uint8_t recv_buf[sizeof(struct ip_packet) + ICMP_DATA_SIZE];
  socklen_t addr_len = sizeof(struct sockaddr_in);

  ssize_t recv_len = recvfrom(socket_fd, recv_buf, sizeof(recv_buf), 0,
                              (struct sockaddr*)src_addr, &addr_len);

  if (recv_len < 0) {
    perror("failed to receive ICMP Echo reply");
    return -1;
  }

  if (recv_len < sizeof(struct ip_packet) + ICMP_DATA_SIZE) {
    // `%zd` is for ssize_t
    fprintf(stderr, "received packet is too short: %zd bytes\n", recv_len);
    return -1;
  }

  struct ip_packet* packet = (struct ip_packet*)recv_buf;

  // validate IP version
  if (packet->ip.version != IPVERSION) {
    fprintf(stderr, "invalid IP version\n");
    return -1;
  }

  // validate ICMP type
  if (packet->icmp.type != ICMP_ECHO_REPLY) {
    fprintf(stderr, "not an ICMP Echo Reply (type: %d)\n", packet->icmp.type);
    return -1;
  }

  if (packet->icmp.code != 0) {
    fprintf(stderr, "invalid ICMP code: %d\n", packet->icmp.code);
    return -1;
  }

  // validate ICMP id
  if (ntohs(packet->icmp.id) != (getpid())) {
    fprintf(stderr, "invalid ICMP ID\n");
    return -1;
  }

  // validate ip, icmp checksum
  uint16_t ip_checksum = packet->ip.check;
  uint16_t icmp_checksum = packet->icmp.checksum;
  packet->ip.check = 0;
  packet->icmp.checksum = 0;

  /* check ip checksum */
  if (!verify_checksum((uint16_t*)&packet->ip, sizeof(struct iphdr),
                       ip_checksum)) {
    fprintf(stderr, "invalid IP checksum\n");
    return -1;
  }

  /* check icmp checksum */
  if (!verify_checksum((uint16_t*)&packet->icmp,
                       sizeof(struct icmphdr) + ICMP_DATA_SIZE,
                       icmp_checksum)) {
    fprintf(stderr, "invalid ICMP checksum\n");
    return -1;
  }

  printf("Received ICMP Echo Reply from %s\n", inet_ntoa(src_addr->sin_addr));
  print_ip_packet(packet);

  return 0;
}

typedef struct {
  int socket_fd;
  struct sockaddr_in src_addr;
  struct sockaddr_in dest_addr;
} recv_thread_data_t;

void* recv_loop(void* arg) {
  recv_thread_data_t* data = (recv_thread_data_t*)arg;
  int socket_fd = data->socket_fd;
  struct sockaddr_in* src_addr = &data->src_addr;
  struct sockaddr_in* dest_addr = &data->dest_addr;

  while (1) {
    printf("--------------recv_loop--------------\n");
    if (recv_icmp_echo_reply(socket_fd, src_addr, dest_addr) < 0) {
      perror("failed to receive ICMP Echo reply");
      exit(1);
    }
  }
  return NULL;
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
  pthread_attr_t recv_thread_attr;
  pthread_t recv_thread;
  recv_thread_data_t thread_data;

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
  memcpy(&src_addr, &interface.ip_addr, sizeof(struct sockaddr_in));

  /* 3. create raw socket */
  socket_fd = create_socket();
  if (socket_fd < 0) {
    perror("failed to create socket");
    exit(1);
  }

  /* 4. convert target IP address */
  init_sockaddr_in(&dest_addr, target_ip);

  /* 5. initialize sockaddr_ll */
  thread_data.socket_fd = socket_fd;
  thread_data.src_addr = src_addr;
  thread_data.dest_addr = dest_addr;

  memset(&recv_thread, 0, sizeof(pthread_t));
  pthread_attr_init(&recv_thread_attr);
  /* set thread detached. if thread is exited, resources are released
   * automatically */
  pthread_attr_setdetachstate(&recv_thread_attr, PTHREAD_CREATE_DETACHED);
  thread_data.socket_fd = socket_fd;
  /* copy src_addr and dest_addr to thread_data */
  memcpy(&thread_data.src_addr, &dest_addr, sizeof(struct sockaddr_in));
  memcpy(&thread_data.dest_addr, &src_addr, sizeof(struct sockaddr_in));

  /* 6. receive Echo reply in a separate thread */
  if (pthread_create(&recv_thread, &recv_thread_attr, recv_loop, &thread_data) <
      0) {
    perror("failed to create receive thread");
    exit(1);
  }

  /* 7. send ICMP Echo request */
  clock_gettime(CLOCK_MONOTONIC, &current_time);
  next_time = current_time;

  while (1) {
    if (send_icmp_echo_request(socket_fd, &src_addr, &dest_addr, sequence++) <
        0) {
      perror("failed to send ICMP Echo request");
      exit(1);
    }
    next_time.tv_sec += 1;
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_time, NULL);
  }

  return 0;
}