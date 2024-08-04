#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "main.h"
#include "mtv230m.h"

#define CASE break; case
#define DEFAULT break; default


enum ParamType {
    DEVICE = 'D',
    FIRMWARE_FILE = 'F',
    START_PAGE = 'P',
    I2C_ADDR = 'A',
    OSD = 'O',
    LENGTH = 'L', 
    SILENCE = 'S'
};

static struct Settings settings = { .silence = false, .mcuAddress = 0x3F, .osd = false, .startPage = 0, .length = 0 };
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
    parse_arguments(argcount, argvals);

    init_mcu();

    FILE* fwfile;

    fwfile = fopen(settings.filepath, "wb");
    if (fwfile == NULL)
    {
        fclose(fwfile);
        dprintf("FCKU!");
        perror("Failed to open firmware file: ");
        exit(EXIT_FAILURE);
    }

    // uint8_t page = settings.startPage;

    uint8_t fwPageContents[256] = {0};

    for (uint16_t page = settings.startPage, lastPage = (settings.startPage + settings.length - 1); page <= lastPage; page++)
    {
        dprintf("Reading page %u\n", page);
        int res = MTV230M_read_page(&g_mcu, settings.osd, page, 64, fwPageContents);
        if (res) 
        {
            dprintf("Read failed, error code: %d!\n", res);
            fclose(fwfile);
            exit(EXIT_FAILURE);
        }
        fwrite(fwPageContents, 256, 1, fwfile);
    }
    fclose(fwfile);

    return EXIT_SUCCESS;
}

signed init_mcu(void)
{
    dprintf("init()!\n");

    g_mcu = MTV230M_connect(settings.mcuAddress, settings.tty);
    switch (g_mcu.status)
    {
        case MTV230M_UNINITIALIZED: cprintf("Where's the I2CDriver???\n");  exit(EXIT_FAILURE);
        case MTV230M_GOT_I2C:       cprintf("Where's the MCU???\n");        exit(EXIT_FAILURE);
        case MTV230M_GOT_MCU:       cprintf("MTV230M found!\n");
    }
    return 0;
}

int32_t parse_arguments(int argcount, char** args)
{
    for (int i = 1; i < argcount; i++)
    {
        if (args[i][0] != '-')
        {
            cprintf("The f*** is a \"%s\" ?\n", args[i]);
            exit(EXIT_FAILURE);
        }

        switch (args[i][1])
        {
            CASE FIRMWARE_FILE: strncpy(settings.filepath, args[++i], 128);
            CASE DEVICE:        strncpy(settings.tty, args[++i], 32);
            CASE I2C_ADDR:      settings.mcuAddress = atoi(args[++i]);
            CASE START_PAGE:    settings.startPage  = atoi(args[++i]);
            CASE OSD:           settings.osd        = true;
            CASE SILENCE:       settings.silence    = true;
            CASE LENGTH:        settings.length     = atoi(args[++i]);

            DEFAULT:            cprintf("The f*** is a -%d ?\n", args[i][1]);
        }
    }
    
    // greeting message - can only be printed now that silence has been determined
    cprintf("mtv230mdump - Myson MTV230 firmware dumping tool\n");


    if (settings.tty[0] == 0)
        strncpy(settings.tty, defaults.tty, 32);
    cprintf("Serial port: %s\n", settings.tty);

    if (settings.filepath[0] == 0)
        strncpy(settings.filepath, settings.osd? defaults.osdFilepath : defaults.fwFilepath, 128);
    cprintf("F/W file path: %s\n", settings.filepath);

    if (settings.mcuAddress > 0b11111100 || settings.mcuAddress < 0b00000100)
    {
        cprintf("Invalid I2C address - %d\n", settings.mcuAddress);
        exit(EXIT_FAILURE);
    } 
    cprintf("I2C base address = %d\n", settings.mcuAddress);

    cprintf("Memory to read: %s\n", settings.osd? "OSD" : "Code");

    if (settings.startPage >= (settings.osd? MTV230M_OSD_PAGE_COUNT : MTV230M_CODE_PAGE_COUNT)) 
    {
        cprintf("Start page too high - %d\n", settings.startPage);
        exit(EXIT_FAILURE);
    }
    cprintf("Starting at page %d\n", settings.startPage);

    if (settings.length == 0) settings.length = (settings.osd? MTV230M_OSD_PAGE_COUNT : MTV230M_CODE_PAGE_COUNT) - settings.startPage;
    if ((settings.startPage + settings.length) > (settings.osd? MTV230M_OSD_PAGE_COUNT : MTV230M_CODE_PAGE_COUNT))
    {
        cprintf("Read length too long - %d\n", settings.length);
        exit(EXIT_FAILURE);
    }
    cprintf("Read %d pages\n", settings.length); 

    return 0;
}
