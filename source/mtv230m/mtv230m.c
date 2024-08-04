#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "mtv230m.h"
#include "i2cdriver.h"

#if defined(NDEBUG)
#define dprintf(...)
#else
#define dprintf(...) printf("[MTV230M]\t" __VA_ARGS__ )
#endif

static uint8_t data_address(uint8_t cmdAddress) { return cmdAddress-1; }

struct MTV230M MTV230M_connect(uint8_t address, const char* portname)
{
    dprintf("MTV230M_connect(0x%2X, %s)\n", address, portname? portname : "NULL" );
    I2CDriver i2c = {0};
    struct MTV230M mcu = {
        .address = address,
        .link = i2c,
        .status = MTV230M_UNINITIALIZED,
        .codeAddress = 0
    };

    if (address > 0b11111100 || address < 0b00000100)
    {
        dprintf("Invalid I2C address: %u\n", address);
        return mcu; // Invalid argument!
    }

    if (!portname)
    {
        dprintf("portname == NULL!\n");
        return mcu;
    }

    i2c_connect(&i2c, portname);
    if (!i2c.connected) 
    {
        dprintf("No I2CDriver found!\n");
        return mcu;
    }

    mcu.status = MTV230M_GOT_I2C;
    mcu.link = i2c;

    struct MTV230M_Command cmd = { .cmd = PROGRAM, .osd = false, .page = 0};
  
    // Temporary set status to GOT_MCU so the CMD write function can do its job
    mcu.status = MTV230M_GOT_MCU;
    int cmdWriteResult = MTV230M_cmd_write(&mcu, &cmd);
    if (cmdWriteResult)
    {
        dprintf("I2C command write not ACK'ed, error code %d!\n", cmdWriteResult);
        goto failure;
    }

    struct MTV230M_Command lastCmd = MTV230M_cmd_read(&mcu, NULL);

    mcu.lastCommand = lastCmd;

    // mcu.lastCommand = MTV230M_cmd_read(&mcu, NULL);
    if (mcu.lastCommand.cmd != cmd.cmd) 
    {
        dprintf("Read command does not match last programmed command!\n");
        goto failure;
    }

    // mcu.status = MTV230M_GOT_MCU;
    dprintf("MTV230M initialized!\n");
    return mcu;

failure:
    mcu.status = MTV230M_GOT_I2C;
    i2c_stop(&mcu.link);
    return mcu;    
}

int32_t MTV230M_cmd_write(struct MTV230M* mcu, struct MTV230M_Command* cmd)
{
    // dprintf("MTV230M_cmd_write(%p, %p)\n", (void*)mcu, (void*)cmd);
    if (!mcu)   return -1;
    if (!cmd)   return -2;

    if (mcu->status != MTV230M_GOT_MCU) return -3;

    uint8_t command[2] = { (cmd->cmd | cmd->osd), cmd->page };

    if (!i2c_start(&mcu->link, mcu->address, 0))
        return -4;

    bool ack = i2c_write(&mcu->link, command, sizeof(command));
    i2c_stop(&mcu->link);
    if (!ack)
        return -5;


    mcu->lastCommand = *cmd;
    return 0;
}

struct MTV230M_Command MTV230M_cmd_read(struct MTV230M* mcu, uint16_t* crc)
{
    dprintf("MTV230M_cmd_read(%p, %p)\n", (void*)mcu, (void*)crc);
    struct MTV230M_Command cmd = {NULL_CMD, 0, 0};
    if (!mcu) return cmd;
    if (mcu->status != MTV230M_GOT_MCU) return cmd;

    uint8_t readLength = (crc? 6 : 4);

    if (!i2c_start(&mcu->link, mcu->address, 1)) {
        dprintf("\t I2C start not ACK'ed!\n");
        return cmd;
    }

    uint8_t rx[6] = {0};
    i2c_read(&mcu->link, rx, readLength);
    i2c_stop(&mcu->link);

    cmd.cmd  = rx[0] & 0b11111110;
    cmd.osd  = rx[0] & 1;
    cmd.page = rx[1];

    mcu->codeAddress = rx[2];
    if (crc) *crc = (rx[3] << 8) | rx[4];

    return cmd;
}


/**
 * @brief Write data to the MTV230M
 * 
 * @param mcu       Pointer to MTV230M handle
 * @param data      Data to write lies here
 * @param length    Write length
 * @return -1: invalid MCU, -2: invalid data pointer, -3: MCU not initialized, -4: failed to start I2C write, -5: I2C write not ack'ed
 */
int32_t MTV230M_data_write(struct MTV230M* mcu, uint8_t* data, uint32_t length)
{
    // dprintf("MTV230M_data_write(%p, %p, %u)", (void*)mcu, (void*)data, length);
    if (!mcu)    return -1;
    if (!data)   return -2;

    if (mcu->status != MTV230M_GOT_MCU) return -3;

    if (!i2c_start(&mcu->link, data_address(mcu->address), 0))
        return -4;

    bool ack = i2c_write(&mcu->link, data, length);
    if (!ack) return -5;
    i2c_stop(&mcu->link);

    return 0;
}
/** @brief Read data from a MTV230M
 *  @param mcu      Pointer to a MTV230M handle
 *  @param addr     Start the read at this address
 *  @param data     Stash the read data here
 *  @param length   Read length in bytes
 * 
 *  @return -1: invalid MCU, -2: invalid data pointer, -3: MCU not initialised, -4: failed to start I2C write, -5: I2C write not ack'ed, -6: failed to start I2C read
*/
int32_t MTV230M_data_read(struct MTV230M* mcu, uint8_t addr, uint8_t* data, uint32_t length)
{
    // dprintf("MTV230M_data_read(%p, %u, %p, %u)\n", (void*)mcu, addr, (void*)data, length);
    if (!mcu)    return -1;
    if (!data)   return -2;

    if (mcu->status != MTV230M_GOT_MCU) return -3;

    if (!i2c_start(&mcu->link, data_address(mcu->address), 0))
        return -4;

    if (!i2c_write(&mcu->link, &addr, 1))
        return -5;

    if (!i2c_start(&mcu->link, data_address(mcu->address), 1))
        return -6;

    i2c_read(&mcu->link, data, length);
    i2c_stop(&mcu->link);

    return 0;
}

/** @brief Read a page from the MTV230M's flash 
 *  @param mcu      Pointer to a MTV230M handle
 *  @param osd      Whether to read from OSD or code memory
 *  @param page     Start reading at this page
 *  @param data     Stash the read data here
 *  @param burstLength  Read in blocks of this many bytes. Must be a power of 2 and less than 256
 * 
 *  @return  -1: Invalid MCU, -2: invalid page, -3: invalid data pointer, -4: invalid burst length, -5: command write failed, -6: data read failed
*/
int32_t MTV230M_read_page(struct MTV230M* mcu, bool osd, uint8_t page, uint8_t burstLength, uint8_t* data) 
{       
    if (!mcu) return -1;
    if (page >= (osd? MTV230M_OSD_PAGE_COUNT : MTV230M_CODE_PAGE_COUNT)) return -2;
    if (!data) return -3;
    if (256 % burstLength) return -4;

    struct MTV230M_Command cmd = {
        .cmd  = PROGRAM,
        .osd  = osd,
        .page = page
    };

    int cmdWriteResult = MTV230M_cmd_write(mcu, &cmd);
    if (cmdWriteResult) 
        return -5;

    //  There were problems reading everything out in one go
    for (uint16_t addr = 0; addr < MTV230M_FLASH_BYTES_PER_PAGE; addr += burstLength) {
        int32_t result = MTV230M_data_read(mcu, (uint8_t)addr, data+addr, burstLength);
        if (result) return -6;
    }
    return 0;   
}

int32_t MTV230M_blank_memory(struct MTV230M* mcu, bool osd)
{
    if (!mcu) return -1;

    struct MTV230M_Command cmd = {
        .cmd = BLANK,
        .osd = osd,
        .page = 0
    };

    // Blank whole memory
    if (MTV230M_cmd_write(mcu, &cmd)) return -2;
    uint8_t b[] = {0, 0};
    if (MTV230M_data_write(mcu, b, 2)) return -3; 

    return 0;
}

int32_t MTV230M_erase_2pages(struct MTV230M* mcu, bool osd, uint8_t page)
{
    if (!mcu) return -1;
    if (page & 1 || page >= (osd? MTV230M_OSD_PAGE_COUNT : MTV230M_CODE_PAGE_COUNT)) return -2;

    struct MTV230M_Command cmd = {
        .cmd = PAGE_ERASE,
        .osd = osd,
        .page = page
    };

    if (MTV230M_cmd_write(mcu, &cmd)) return -4;
    uint8_t b[] = {0, 0};
    if (MTV230M_data_write(mcu, b, 2)) return -5;

    return 0;
}

int32_t MTV230M_clear_CRC(struct MTV230M* mcu) 
{
    struct MTV230M_Command cmd = {0}; cmd.cmd = CLEAR_CRC;
    return MTV230M_cmd_write(mcu, &cmd);
}

int32_t MTV230M_write_page(struct MTV230M* mcu, bool osd, uint8_t page, uint8_t* data, uint32_t length)
{
    struct MTV230M_Command cmd = {
        .cmd = PROGRAM,
        .osd = osd,
        .page = page
    };

    MTV230M_cmd_write(mcu, &cmd);
    MTV230M_data_write(mcu, data, length);
    
}
