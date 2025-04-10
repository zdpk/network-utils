#pragma once

#include <stdbool.h>
#include <stdint.h>

extern uint16_t calc_checksum(uint16_t* buf, int len);

extern bool verify_checksum(uint16_t* buf, int len, uint16_t checksum);