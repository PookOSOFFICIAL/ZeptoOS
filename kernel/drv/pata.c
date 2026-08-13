#include "pata.h"
#include "../i386/io.h"

#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_FEATURE      0x1F1
#define ATA_PRIMARY_SEC_COUNT    0x1F2
#define ATA_PRIMARY_LBA_LOW      0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HIGH     0x1F5
#define ATA_PRIMARY_DEVICE       0x1F6
#define ATA_PRIMARY_COMMAND      0x1F7
#define ATA_PRIMARY_STATUS       0x1F7
#define ATA_CMD_READ_PIO         0x20
#define ATA_CMD_WRITE_PIO        0x30
#define ATA_STATUS_BSY           0x80
#define ATA_STATUS_DRDY          0x40
#define ATA_STATUS_DRQ           0x08

void pata_init() {
}

static void pata_wait_bsy() {
    while (inb(ATA_PRIMARY_STATUS) & ATA_STATUS_BSY);
}

static void pata_wait_drq() {
    while (!(inb(ATA_PRIMARY_STATUS) & ATA_STATUS_DRQ));
}

int pata_read_sectors(uint32_t lba, uint8_t sector_count, uint8_t *buf) {
    pata_wait_bsy();
    outb(ATA_PRIMARY_DEVICE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SEC_COUNT, sector_count);
    outb(ATA_PRIMARY_LBA_LOW, (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_PIO);

    for (int s = 0; s < sector_count; s++) {
        pata_wait_bsy();
        pata_wait_drq();
        for (int i = 0; i < 256; i++) {
            uint16_t data = inw(ATA_PRIMARY_DATA);
            buf[s * 512 + i * 2] = (uint8_t)(data & 0xFF);
            buf[s * 512 + i * 2 + 1] = (uint8_t)(data >> 8);
        }
    }
    return 0;
}

int pata_write_sectors(uint32_t lba, uint8_t sector_count, const uint8_t *buf) {
    pata_wait_bsy();
    outb(ATA_PRIMARY_DEVICE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SEC_COUNT, sector_count);
    outb(ATA_PRIMARY_LBA_LOW, (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_PIO);

    for (int s = 0; s < sector_count; s++) {
        pata_wait_bsy();
        pata_wait_drq();
        for (int i = 0; i < 256; i++) {
            uint16_t data = buf[s * 512 + i * 2] | ((uint16_t)buf[s * 512 + i * 2 + 1] << 8);
            outw(ATA_PRIMARY_DATA, data);
        }
    }
    pata_wait_bsy();
    return 0;
}
