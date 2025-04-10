#pragma once

#include <stdbool.h>
#include <stdint.h>

uint16_t calc_checksum(uint16_t* buf, int len);

bool verify_checksum(uint16_t* buf, int len, uint16_t checksum);