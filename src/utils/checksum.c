#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/l3/ip.h"
#include "../../include/l4/udphdr.h"

extern uint16_t calc_checksum(uint16_t* buf, int len) {
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

  /* move upper 16 bits to lower 16 bits and add them */
  sum = (sum >> 16) + (sum & 0x0000FFFF);
  /* if the addition of upper 16 bits causes a carry, add it to the lower 16
   * bits */
  sum += (sum >> 16);

  /* return the lower 16 bits of the final value as a one's complement */
  return (uint16_t)(~sum);
}

extern bool verify_checksum(uint16_t* buf, int len, uint16_t checksum) {
  return calc_checksum(buf, len) == checksum;
}

extern uint16_t calc_udp_checksum(struct iphdr* iph, struct udphdr* udph,
                                  uint8_t* data, uint16_t data_len) {
  uint16_t udp_len = sizeof(struct udphdr) + data_len;
  int total_len = sizeof(struct udphdr) + data_len;
  /* pseudo ip header */
  uint16_t* udp_pseudo_buf = NULL;
  uint32_t sum = 0;
  uint16_t result;

  udph->len = htons(udp_len);
  udph->check = 0;

  /* allocate buf for pseudo ip header + UDP header + UDP payload */
  udp_pseudo_buf = (uint16_t*)malloc(total_len);
  if (NULL == udp_pseudo_buf) {
    perror("Failed to allocate memory for pseudo ip header");
    return 0;
  }
  memset(udp_pseudo_buf, 0, total_len);

  uint8_t* ptr = (uint8_t*)udp_pseudo_buf;

  /* 1. fill pseudo ip header */
  memcpy(ptr, &iph->saddr, sizeof(iph->saddr));
  ptr += sizeof(iph->saddr);
  memcpy(ptr, &iph->daddr, sizeof(iph->daddr));
  ptr += sizeof(iph->daddr);
  *ptr++ = 0;
  *ptr++ = iph->protocol;
  *(uint16_t*)ptr = htons(udp_len);
  ptr += sizeof(uint16_t);

  /* 2. copy UDP header from pseudo ip header to UDP header */
  memcpy(ptr, udph, sizeof(struct udphdr));
  ptr += sizeof(struct udphdr);

  /* 3. copy UDP payload(data) */
  if (NULL != data && data_len > 0) {
    memcpy(ptr, data, data_len);
  }

  /* 4. calculate checksum */
  result = calc_checksum(udp_pseudo_buf, total_len);

  free(udp_pseudo_buf);

  /*
    UDP checksum of 0 is transmitted as 0xFFFF
    because 0 checksum in UDP means "not computed"(because UDP checksum is
      optional)

    so it cannot be distinguished between "not computed" and "computed as 0"
    so we need to return 0xFFFF in this case(computed as 0)
  */
  return result == 0 ? 0xFFFF : result;
}