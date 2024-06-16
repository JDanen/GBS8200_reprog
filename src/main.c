#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "main.h"
#include "mtv230m.h"
// #include "../vendor/i2cdriver/c/common/i2cdriver.h"


enum ParamType {
    DEVICE = 'D',
    INTERACTIVE = 'I',
    FIRMWARE_FILE = 'F',
    START_PAGE = 'P',
    READ = 'R',
    WRITE = 'W',
    I2C_ADDR = 'A',
    OSD = 'O'
};

static struct ProgState progState = { true, 0, .osd = false };
struct MTV230M g_mcu;

/**
 * @brief It's the main function, duh
 *
 * @param argcount count of how many string args the user started it with
 * @param argvals  the array of argument strings themselves
 * @returns a zero
 */
signed main(int argcount, char** argvals)
{
    printf("Hello World!\n");
    progState.mode = 0;
    parse_arguments(argcount, argvals);

    init_mcu();

    FILE* fwfile;

    // if (fwfile == NULL)
    // {
    //     perror("Failed to open firmware file: ");
    //     exit(EXIT_FAILURE);
    // }

    switch (progState.mode) {
    case READ: {
        fwfile = fopen(progState.filepath, "wb");
        if (fwfile == NULL)
        {
            perror("Failed to open firmware file: ");
            exit(EXIT_FAILURE);
        }

        struct MTV230M_Command cmd = {
            .cmd  = PROGRAM,
            .osd  = progState.osd,
            .page = progState.startPage
        };

        uint8_t fwPageContents[256] = {0};
        do
        {
            MTV230M_cmd_write(&g_mcu, &cmd);
            for (uint16_t addr = 0; addr < 256; addr += 64) {
                MTV230M_data_read(&g_mcu, (uint8_t)addr, fwPageContents+addr, 64);
            }
            fwrite(fwPageContents, 256, 1, fwfile);
        } while(cmd.page++ < MTV230M_MAX_CODE_PAGE);
        fclose(fwfile);
    } exit(EXIT_SUCCESS);

    case WRITE: {
        fwfile = fopen(progState.filepath, "r+b");
        if (fwfile == NULL)
        {
            perror("Failed to open firmware file: ");
            exit(EXIT_FAILURE);
        }

        struct MTV230M_Command cmd = {
            .cmd  = BLANK,
            .osd  = progState.osd,
            .page = progState.startPage
        };
        dprintf("Erasing...\n");
        MTV230M_cmd_write(&g_mcu, &cmd);
        uint8_t b[] = {0, 0};
        MTV230M_data_write(&g_mcu, &b, 2);
        sleep(1);
        cmd.cmd = PROGRAM;

        uint8_t fwPageContents[257] = {0};
        do
        {
            uint32_t br = fread(fwPageContents+1, 1, 256, fwfile);
            dprintf("Programming page %u \n", cmd.page);
            MTV230M_cmd_write(&g_mcu, &cmd);
            MTV230M_data_write(&g_mcu, &fwPageContents, 257);
        } while(++cmd.page < MTV230M_MAX_CODE_PAGE);
        printf("Done programming! \n");
        fclose(fwfile);
    } exit(EXIT_SUCCESS);

    case INTERACTIVE:
        printf("Not supported yet!\n");
        exit(EXIT_SUCCESS);
    }

    while (progState.run)
    {
        loop();
        progState.loopCount++;
        if (progState.loopCount > 5)
            progState.run = false;
    }

    return EXIT_SUCCESS;
}

signed init_mcu(void)
{
    dprintf("init()!\n");

    g_mcu = MTV230M_connect(0x3F, progState.tty);
    switch (g_mcu.status)
    {
        case MTV230M_UNINITIALIZED: dprintf("Where's the I2CDriver???\n");      exit(EXIT_FAILURE);
        case MTV230M_GOT_I2C:       dprintf("Where's the MCU?? \n");            exit(EXIT_FAILURE);
        case MTV230M_GOT_MCU:       dprintf("MCU connected!\n");                break;
    }
    return 0;
}

signed loop(void)
{
    //printf("loop() %llu", progState.loopCount);
    return 0;
}

int32_t parse_arguments(int argcount, char** args)
{
    dprintf("argcount: %d\n", argcount);

    for (int i = 1; i < argcount;)
    {
        if (args[i][0] != '-')
        {
            printf("The f*** is a \"%s\" ?\n", args[i]);
            exit(EXIT_FAILURE);
        }

        switch (args[i][1])
        {
        case FIRMWARE_FILE:
            dprintf("F/W file path: %s\n", args[i+1]);
            strncpy(progState.filepath, args[i+1], 32);
            i += 2;
            break;


if(0)   case INTERACTIVE:   dprintf("Interactive mode\n");
if(0)   case WRITE:         dprintf("Write\n");
if(0)   case READ:          dprintf("Read\n");
            if (progState.mode)
            {
                dprintf("Multiple mode parameters detected! \n");
                return -2;
            }
            progState.mode = args[i][1];
            i++;
            break;

        case DEVICE:
            dprintf("Device path: %s\n", args[i+1]);
            strncpy(progState.tty, args[i+1], 32);
            i += 2;
            break;

        case I2C_ADDR: {
            int addrArg = atoi(args[i+1]);
            dprintf("MCU I2C base address = %X\n", progState.mcuAddress);
            if (addrArg > 127 || addrArg == 0) exit(EXIT_FAILURE);
            progState.mcuAddress = (uint8_t)addrArg;
            i += 2;
        } break;

        case START_PAGE: {
            int pageArg = atoi(args[i+1]);
            if (pageArg > 255) exit(EXIT_FAILURE);
            progState.startPage = (uint8_t)atoi(args[i+1]);
            dprintf("MCU memory start page: %X\n", progState.startPage);
            i++;
        } break;

        case OSD:
            progState.osd = true;
            i++;
            break;

        default:
            printf("The f*** is a -%d ?\n", args[i][1]);
            break;
        }
    }
    if (!progState.mode)
    {
        printf("Gimme a mode!\n");
        exit(EXIT_FAILURE);
    }

    if (progState.tty[0] == 0)
        strncpy(progState.tty, defaults.tty, 32);

    if (progState.filepath[0] == 0)
        strncpy(progState.filepath, defaults.filepath, 16);

    return 0;
}
