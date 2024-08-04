#pragma once

/**
 * Functions for interfacing with the MTV230M microcontroller, as used in the GBS8200 video scaler
 */
#include <stdint.h>
#include <stdbool.h>

#include <i2cdriver.h>

// const uint8_t MTV230M_MAX_CODE_PAGE = 255;
#define MTV230M_MAX_CODE_PAGE 255

#define MTV230M_CODE_FLASH_SIZE  (64 * 1024)       /** Code flash size = 64 kB */
#define MTV230M_OSD_FLASH_SIZE   (9 * 2 * 1024)    /** OSD flash size = 9 k-words = 18 kB */

#define MTV230M_FLASH_BYTES_PER_PAGE (256)

#define MTV230M_CODE_PAGE_COUNT  (MTV230M_CODE_FLASH_SIZE / MTV230M_FLASH_BYTES_PER_PAGE)   /** Should be 256 */
#define MTV230M_OSD_PAGE_COUNT   (MTV230M_OSD_FLASH_SIZE  / MTV230M_FLASH_BYTES_PER_PAGE)   /** Should be 72  */

#define MTV230M_MONO_CHAR_COUNT 496
#define MTV230M_RGB_CHAR_COUNT  16

enum Commands {
    PROGRAM     = 0b10100000,
    PAGE_ERASE  = 0b00110000,
    BLANK       = 0b01101000,
    CLEAR_CRC   = 0b11010000,
    RESET_CPU   = 0b01001000,

    NULL_CMD = 0
};

enum MTV230M_Status {
    MTV230M_UNINITIALIZED = 0,
    MTV230M_GOT_I2C,
    MTV230M_GOT_MCU
};

struct MTV230M_Command {
    enum Commands cmd;
    bool osd;
    uint8_t page;
};

struct MTV230M {
    uint8_t address;
    I2CDriver link;

    struct MTV230M_Command lastCommand;
    uint8_t codeAddress;

    enum MTV230M_Status status;
};


struct MTV230M MTV230M_connect(uint8_t address, const char* portname);

int32_t MTV230M_cmd_write(struct MTV230M* mcu, struct MTV230M_Command* cmd);
struct MTV230M_Command MTV230M_cmd_read(struct MTV230M* mcu, uint16_t* crc);
int32_t MTV230M_data_write(struct MTV230M* mcu, uint8_t* data, uint32_t length);
int32_t MTV230M_data_read(struct MTV230M* mcu, uint8_t addr, uint8_t* data, uint32_t length);

int32_t MTV230M_read_page(struct MTV230M* mcu, bool osd, uint8_t page, uint8_t burstLength, uint8_t* data);
int32_t MTV230M_blank_memory(struct MTV230M* mcu, bool osd);
int32_t MTV230M_erase_2pages(struct MTV230M* mcu, bool osd, uint8_t page);
int32_t MTV230M_clear_CRC(struct MTV230M* mcu);
int32_t MTV230M_write_page(struct MTV230M* mcu, bool osd, uint8_t page, uint8_t* data, uint32_t length);
