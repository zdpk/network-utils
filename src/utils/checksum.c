#include <stdbool.h>
#include <stdint.h>

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