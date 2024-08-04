#pragma once

#include <stdint.h>

/**
 * A tool to program the MTV230M microcontroller using a FT232-based I2C interface.
 *
 */

#if defined(NDEBUG)
#define dprintf(...)
#else
#define dprintf(...) printf("[DBG] " __VA_ARGS__ )
#endif


/**
 * @brief Contains stuff relating to the execution of the program
 *
 */
struct Settings {
    bool silence;
    bool delicate;

    char tty[32];
    char filepath[128];

    uint8_t mcuAddress;
    bool osd;
    uint8_t startPage;
};
// conditional print - don't do on silent mode
#define cprintf(...) if (!settings.silence) printf( __VA_ARGS__ )

struct Defaults {
    const char tty[16];
    const char filepath[16];
};
const struct Defaults defaults = {
    .tty = "/dev/ttyUSB0",
    .filepath = "mtv230m_fw.bin"
};

/** Put all the initialisation-related stuff here */
signed init_mcu(void);
FILE* init_file(void);


int32_t parse_arguments(int argcount, char** args);
void init_i2c(void);
