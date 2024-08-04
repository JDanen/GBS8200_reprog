#pragma once

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

    char tty[32];
    char filepath[128];

    uint8_t mcuAddress;
    bool osd;
    uint8_t startPage;
    uint16_t length;
};
// conditional print - don't do on silent mode
#define cprintf(...) if (!settings.silence) printf( __VA_ARGS__ )

struct Defaults {
    const char tty[16];
    const char fwFilepath[128];
    const char osdFilepath[128];
};
const struct Defaults defaults = {
    .tty = "/dev/ttyUSB0",
    .fwFilepath  = "mtv230m_fw.bin",
    .osdFilepath = "mtv230m_osd.bin"
};

/** Put all the initialisation-related stuff here */
signed init_mcu(void);
FILE* init_file(void);

/** The good old main loop. Stops getting called when something sets progState.run to 0 */
signed loop(void);

int32_t parse_arguments(int argcount, char** args);
void init_i2c(void);
