#pragma once
#include "../lib/types.h"

void pata_init();
int pata_read_sectors(uint32_t lba, uint8_t sector_count, uint8_t *buf);
int pata_write_sectors(uint32_t lba, uint8_t sector_count, const uint8_t *buf);
