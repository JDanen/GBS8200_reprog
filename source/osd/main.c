#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "mtv230m.h"

#define cprintf(...) if (settings.silence) printf(__VA_ARGS__)

#define CASE    break; case
#define DEFAULT break; default
int32_t parse_arguments(int argcount, char** args);

enum ParamType
{
    FILEPATH = 'F',
    RGB      = 'R',
    POSITION = 'P',
    LENGTH   = 'L',
    SILENCE  = 'S'
};

struct Settings {
    char filepath[128];
    bool rgb;
    uint16_t startPosition;
    uint16_t length;
    bool silence; 
} settings = {0};

signed main(int argcount, char** argvals) 
{
    FILE* osdFile;

    osdFile = fopen(settings.filepath, "wb");
    if (osdFile == NULL)
    {
        fclose(osdFile);
        perror("Failed to open firmware file: ");
        exit(EXIT_FAILURE);
    }
    
    uint16_t lastCharPos = settings.startPosition + settings.length - 1;
    for (uint16_t symbol = settings.startPosition; symbol <= lastCharPos; symbol++) 
    {
        uint16_t character[16][18] = {0};
        fread(character, )
    }
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
            CASE SILENCE:   settings.silence = true;

            CASE FILEPATH:      strncpy(settings.filepath, args[++i], 128);

            CASE POSITION:  settings.startPosition = atoi(args[++i]);
            CASE LENGTH:    settings.length        = atoi(args[++i]);
            CASE RGB:       settings.rgb           = true;

            DEFAULT:    cprintf("The f*** is a -%d ?\n", args[i][1]);
        }
    }

    cprintf("The MTV230M OSD graphics decoder at your service! \n");

    uint16_t maxCharPos = (settings.rgb? MTV230M_RGB_CHAR_COUNT : MTV230M_MONO_CHAR_COUNT) - 1;
    if (settings.startPosition > maxCharPos)
    {
        cprintf("Starting position (%u) too high, max is %u!\n", settings.startPosition, maxCharPos);
        exit(EXIT_FAILURE);
    }

    uint16_t lastCharPos = settings.startPosition + settings.length - 1;
    if (lastCharPos > maxCharPos) {
        cprintf("You're trying to read too many chars (length too long)\n");
        exit(EXIT_FAILURE);
    }

    if (settings.rgb)
    {
        cprintf("RGB mode not supported yet!\n");
        exit(EXIT_FAILURE);
    }
}

