#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "mtv230m.h"
#include "i2cdriver.h"

#if defined(NDEBUG)
#define dprintf(...)
#else
#define dprintf(...) printf("[DBG] " __VA_ARGS__ )
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

    if (address > 127 || !portname)
        return mcu; // Invalid argument!

    i2c_connect(&i2c, portname);
    if (!i2c.connected) return mcu;
    mcu.status = MTV230M_GOT_I2C;
    mcu.link = i2c;

    struct MTV230M_Command cmd = { .cmd = PROGRAM, .osd = false, .page = 0};

    uint8_t command[2] = { (cmd.cmd | cmd.osd), cmd.page };

    if (!i2c_start(&mcu.link, mcu.address, 0))
        dprintf("\t I2C start not ACK'ed!\n");

    bool ack = i2c_write(&mcu.link, command, sizeof(command));
    i2c_stop(&mcu.link);

    if (!MTV230M_cmd_write(&mcu, &cmd))
        dprintf("Could not write command to MCU!");

    if (!i2c_start(&mcu.link, mcu.address, 1)) {
        dprintf("\t I2C start not ACK'ed!\n");
    }

    uint8_t rx[4] = {0};
    i2c_read(&mcu.link, rx, 4);
    i2c_stop(&mcu.link);

    mcu.lastCommand.cmd  = rx[0] & 0b11111110;
    mcu.lastCommand.osd  = rx[0] & 1;
    mcu.lastCommand.page = rx[1];

    // mcu.lastCommand = MTV230M_cmd_read(&mcu, NULL);
    if (mcu.lastCommand.cmd == NULL_CMD) return mcu;
    mcu.status = MTV230M_GOT_MCU;

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
