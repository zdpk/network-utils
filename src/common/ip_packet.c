#include "../../include/ip_packet.h"

#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "../../include/icmp.h"
#include "../../include/if_ether.h"

void print_ip_packet(struct ip_packet* packet) {
  // print_ethhdr(&packet->eth);
  print_iphdr(&packet->ip);
  print_icmphdr(&packet->icmp);

  // Print data section
  printf("Data:\n");
  uint64_t timestamp = *(uint64_t*)packet->data;
  // Convert nanoseconds to seconds (nanoseconds -> seconds)
  time_t t = (time_t)(timestamp / 1000000000);
  char time_str[26];
  ctime_r(&t, time_str);
  time_str[24] = '\0';  // Remove newline character
  printf("  Timestamp: %s\n", time_str);

  printf("  Alphabet: ");
  for (int i = 8; i < ICMP_DATA_SIZE - 8; i++) {
    printf("%c", packet->data[i]);
  }
  printf("\n");
}