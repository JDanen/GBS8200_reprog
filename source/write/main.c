#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "main.h"
#include "mtv230m.h"

#define CASE    break; case
#define DEFAULT break; default

enum ParamType
{
    DEVICE = 'D',
    FIRMWARE_FILE = 'F',
    START_PAGE = 'P',
    I2C_ADDR = 'A',
    OSD = 'O',
    SILENCE = 'S',
    DELICATE = 'd'
};

static struct Settings settings = {.silence = false, .delicate = false, .mcuAddress = 0x3F, .osd = false, .startPage = 0};
struct MTV230M g_mcu;

signed main(int argcount, char **argvals)
{
    parse_arguments(argcount, argvals);
    init_mcu();

    FILE *fwfile;

    fwfile = fopen(settings.filepath, "r+b");
    if (fwfile == NULL)
    {
        perror("Failed to open firmware file: ");
        exit(EXIT_FAILURE);
    }

    // calculate filesize
    fseek(fwfile, 0, SEEK_END);
    size_t filesize = ftell(fwfile);
    rewind(fwfile);
    if (filesize > (settings.osd ? MTV230M_OSD_FLASH_SIZE : MTV230M_CODE_FLASH_SIZE))
    {
        cprintf("F/W file too large! (filesize %d B, free space %d B)\n", 
                filesize,
                256 * ((settings.osd? MTV230M_OSD_PAGE_COUNT : MTV230M_CODE_PAGE_COUNT) - settings.startPage)
        );

        exit(EXIT_FAILURE);
    }
    size_t lastPage = settings.startPage + (filesize / 512) + (filesize % 512 ? 1 : 0);
    dprintf("Last page = %u", lastPage);

    // Prepare memory
    if (settings.delicate)
    {
        /** TODO: implement a mode that reads two pages, compares them to new f/w and only writes them if they're different */
        cprintf("Delicate mode not supported yet!");
        exit(EXIT_FAILURE);
    }
    else
    {
        if (settings.startPage == 0 && lastPage == (settings.osd? MTV230M_OSD_PAGE_COUNT-1 : MTV230M_CODE_PAGE_COUNT-1))
            MTV230M_blank_memory(&g_mcu, settings.osd);
        else for (size_t page = settings.startPage; page <= lastPage; page += 2)
            MTV230M_erase_2pages(&g_mcu, settings.osd, page);

        MTV230M_clear_CRC(&g_mcu);

        uint8_t fwPageContents[257] = {0}; // Extra zero at the beginning for the data address
        for (uint8_t page = settings.startPage; page <= lastPage; page++)
        {
            uint32_t br = fread(fwPageContents + 1, 1, 256, fwfile);
            dprintf("Programming page %u \n", page);
            MTV230M_write_page(&g_mcu, settings.osd, page, fwPageContents, br+1);
        } //while (++page < lastPage);
        cprintf("Done programming! \n");
        
        /** TODO: Calculate and check CRC at this point */

        fclose(fwfile);
    }

    exit(EXIT_SUCCESS);
}

signed init_mcu(void)
{
    g_mcu = MTV230M_connect(settings.mcuAddress, settings.tty);
    switch (g_mcu.status)
    {
        case MTV230M_UNINITIALIZED: cprintf("Where's the I2CDriver???\n");  exit(EXIT_FAILURE);
        case MTV230M_GOT_I2C:       cprintf("Where's the MCU???\n");        exit(EXIT_FAILURE);
        case MTV230M_GOT_MCU:       cprintf("MTV230M found!\n");
    }
    return 0;
}

int32_t parse_arguments(int argcount, char **args)
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
            CASE SILENCE : settings.silence = true;
            CASE DELICATE : settings.delicate = true;

            CASE FIRMWARE_FILE : strncpy(settings.filepath, args[++i], 128);
            CASE DEVICE : strncpy(settings.tty, args[++i], 32);

            CASE I2C_ADDR : settings.mcuAddress = atoi(args[++i]);
            CASE OSD : settings.osd = true;
            CASE START_PAGE : settings.startPage = atoi(args[++i]);

        DEFAULT:
            cprintf("The f*** is a -%d ?\n", args[i][1]);
        }
    }

    cprintf("mtv230mflash - A firmware flashing tool for the Myson MTV230M microcontroller");

    if (settings.tty[0] == 0)
        strncpy(settings.tty, defaults.tty, 32);
    cprintf("F/W file path: %s\n", settings.tty);

    if (settings.filepath[0] == 0)
        strncpy(settings.filepath, defaults.filepath, 16);
    cprintf("Serial port: %s\n", settings.filepath);

    if (settings.mcuAddress > 0b11111100 || settings.mcuAddress < 0b00000100)
    {
        cprintf("Invalid I2C address - %d\n", settings.mcuAddress);
        exit(EXIT_FAILURE);
    }
    cprintf("I2C base address = %d\n", settings.mcuAddress);

    cprintf("Memory to flash: %s\n", settings.osd ? "OSD" : "Code");

    if (settings.startPage % 2) // Flash can only be erased in 2-page (512 B) blocks, so no odd addresses
    {
        cprintf("Can't start on an odd page (%d)\n", settings.startPage);
        exit(EXIT_FAILURE);
    }
    if (settings.startPage >= (settings.osd ? MTV230M_OSD_PAGE_COUNT : MTV230M_CODE_PAGE_COUNT))
    {
        cprintf("Start page too high - %d\n", settings.startPage);
        exit(EXIT_FAILURE);
    }
    cprintf("Starting at page %d\n", settings.startPage);
}