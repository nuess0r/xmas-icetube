#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>        // for using standard integer types
#include <avr/pgmspace.h>  // for accessing data in program memory

#include "config.h"  // for configuration macros


#define DISPLAY_SIZE 9
#define DISPLAY_OFF_TIMEOUT 30

// status flags for display.status
#define DISPLAY_ANIMATED     0x01  // animated display transitions
#define DISPLAY_ZEROPAD      0x02  // zero-pad all numbers
#define DISPLAY_ALTNINE      0x04  // alternative display for 9s
#define DISPLAY_PULSING      0x10  // display brightness pulsing
#define DISPLAY_PULSE_DOWN   0x20  // display brightness dimming
#define DISPLAY_DISABLED     0x40  // display disabled

// savable settings in lower nibble of display.status
#define DISPLAY_SETTINGS_MASK 0x0F

// amount of time required for one step of OCR0A when pulsing
#define DISPLAY_PULSE_DELAY 8  // (semiticks)


// types of display transitions
enum {
    DISPLAY_TRANS_NONE,
    DISPLAY_TRANS_INSTANT,
    DISPLAY_TRANS_UP,
    DISPLAY_TRANS_DOWN,
    DISPLAY_TRANS_LEFT,
};

// duration of each left/right or up/down transiton step
#define DISPLAY_TRANS_LR_DELAY 16  // (semiticks)
#define DISPLAY_TRANS_UD_DELAY 48  // (semiticks)


typedef struct {
    uint8_t status;                 // display status flags

    uint8_t trans_type;             // current transition type
    uint8_t trans_timer;            // current transition timer
    uint8_t prebuf[DISPLAY_SIZE];   // future display contents
    uint8_t postbuf[DISPLAY_SIZE];  // current display contents

#ifdef AUTOMATIC_DIMMER
    int8_t  bright_min;             // minimum display brightness
    int8_t  bright_max;             // maximum display brightness

    // display turns off during wake mode when photosensor is below
    // 256 * threshold and the display-off timer is expired
    uint8_t off_threshold;         // display-off threshold

    // photoresistor adc result (times 2^6, running average)
    uint16_t photo_avg;
#else
    int8_t  brightness;             // display brightness
#endif  // AUTOMATIC_DIMMER

    uint8_t off_timer;             // display-off timer

    // length of time to display each digit (32 microsecond units)
    uint8_t digit_times[DISPLAY_SIZE];
    uint8_t digit_time_shift;  // flicker reduction adjustment
    uint8_t gradient;          // display gradient adjustment
} display_t;

volatile extern display_t display;


void display_init(void);
void display_wake(void);
void display_sleep(void);

void display_tick(void);
uint8_t display_varsemitick(void);
void display_semitick(void);

void display_savestatus(void);
void display_loadstatus(void);

void display_loadbright(void);
void display_savebright(void);

void display_loaddigittimes(void);
void display_savedigittimes(void);

void display_noflicker(void);

#ifdef AUTOMATIC_DIMMER
void display_loadoff(void);
void display_saveoff(void);
#endif  // AUTOMATIC_DIMMER

uint8_t display_onbutton(void);

void display_autodim(void);

void display_pstr(const uint8_t idx, PGM_P pstr);
void display_digit(uint8_t idx, uint8_t n);
void display_twodigit_rightadj(uint8_t idx, int8_t n);
void display_twodigit_leftadj(uint8_t idx, int8_t n);
void display_twodigit_zeropad(uint8_t idx, int8_t n);
void display_char(uint8_t idx, char c);
void display_clear(uint8_t idx);

void display_clearall(void);
void display_dotselect(uint8_t idx_start, uint8_t idx_end);
void display_dot(uint8_t idx, uint8_t show);
void display_dash(uint8_t idx, uint8_t show);
void display_dial(uint8_t idx, uint8_t seconds);

void display_transition(uint8_t type);

#endif
