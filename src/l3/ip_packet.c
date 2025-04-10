#include "../../include/l3/ip_packet.h"

#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "../../include/l2/if_ether.h"
#include "../../include/l3/icmp.h"

extern void print_ip_packet(struct ip_packet* packet) {
  print_iphdr(&packet->ip);
  print_icmphdr(&packet->icmp);

  // Print data section
  printf("Data:\n");

  /* Get total length of IP packet */
  uint16_t total_len = ntohs(packet->ip.tot_len);
  /* Calculate ICMP header and data length */
  size_t icmp_len = total_len - (packet->ip.ihl * 4);
  /* Calculate data section length (excluding ICMP header) */
  size_t data_len = icmp_len - sizeof(struct icmphdr);

  /* Ensure we have valid data length */
  if (data_len <= 0) {
    printf("  No data available\n");
    return;
  }

  /* Create a safe buffer for data access */
  uint8_t* data_ptr =
      (uint8_t*)packet + sizeof(struct iphdr) + sizeof(struct icmphdr);

  /* Check if we have enough data for timestamp (8 bytes) */
  if (data_len >= 8) {
    // The first 4 bytes are seconds, and the next 4 bytes are microseconds
    uint32_t sec = ntohl(*(uint32_t*)data_ptr);
    uint32_t usec = ntohl(*(uint32_t*)(data_ptr + 4));

    // Convert seconds to time
    time_t t = (time_t)sec;
    char time_str[26];
    ctime_r(&t, time_str);
    time_str[24] = '\0';  // Remove newline character

    printf("  Timestamp: %s, usec: %06u\n", time_str, usec);

    /* Print remaining data if any */
    if (data_len > 8) {
      printf("  Alphabet: ");
      /* Print only the actual data bytes */
      for (size_t i = 8; i < data_len && i < ICMP_DATA_SIZE; i++) {
        printf("%c", data_ptr[i]);
      }
      printf("\n");
    }
  } else {
    printf("  No timestamp data available\n");
  }
}