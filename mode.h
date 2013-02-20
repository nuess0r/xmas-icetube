#ifndef MODE_H
#define MODE_H

#include <stdint.h>  // for using standard integer types

#include "config.h"  // for configuration macros


// default menu timeout; on timeout, mode changes to time display
#define MODE_TIMEOUT 30000  // semiticks (~milliseconds)


// various clock modes; current mode given by mode.state
enum {
    MODE_TIME_DISPLAY,
        MODE_DAYOFWEEK_DISPLAY,
        MODE_MONTHDAY_DISPLAY,
        MODE_YEAR_DISPLAY,
        MODE_ALARMSET_DISPLAY,
        MODE_ALARMIDX_DISPLAY,
        MODE_ALARMTIME_DISPLAY,
        MODE_ALARMDAYS_DISPLAY,
        MODE_ALARMOFF_DISPLAY,
        MODE_SNOOZEON_DISPLAY,
    MODE_SETALARM_MENU,
	MODE_SETALARM_IDX,
	MODE_SETALARM_ENABLE,
        MODE_SETALARM_HOUR,
        MODE_SETALARM_MINUTE,
        MODE_SETALARM_DAYS_OPTIONS,
        MODE_SETALARM_DAYS_CUSTOM,
    MODE_SETTIME_MENU,
        MODE_SETTIME_HOUR,
        MODE_SETTIME_MINUTE,
        MODE_SETTIME_SECOND,
    MODE_SETDATE_MENU,
        MODE_SETDATE_YEAR,
        MODE_SETDATE_MONTH,
        MODE_SETDATE_DAY,
    MODE_CFGALARM_MENU,
	MODE_CFGALARM_SETSOUND_MENU,
	    MODE_CFGALARM_SETSOUND,
	MODE_CFGALARM_SETVOL_MENU,
	    MODE_CFGALARM_SETVOL,
	    MODE_CFGALARM_SETVOL_MIN,
	    MODE_CFGALARM_SETVOL_MAX,
	    MODE_CFGALARM_SETVOL_TIME,
	MODE_CFGALARM_SETSNOOZE_MENU,
	    MODE_CFGALARM_SETSNOOZE_TIME,
	MODE_CFGALARM_SETHEARTBEAT_MENU,
	    MODE_CFGALARM_SETHEARTBEAT_TOGGLE,
    MODE_CFGDISP_MENU,
	MODE_CFGDISP_SETBRIGHT_MENU,
	    MODE_CFGDISP_SETBRIGHT_LEVEL,
#ifdef AUTOMATIC_DIMMER
	    MODE_CFGDISP_SETBRIGHT_MIN,
	    MODE_CFGDISP_SETBRIGHT_MAX,
#endif  // AUTOMATIC_DIMMER
	MODE_CFGDISP_SETDIGITBRIGHT_MENU,
	    MODE_CFGDISP_SETDIGITBRIGHT_LEVEL,
	MODE_CFGDISP_SETAUTOOFF_MENU,
#ifdef AUTOMATIC_DIMMER
	    MODE_CFGDISP_SETPHOTOOFF_MENU,
	    MODE_CFGDISP_SETPHOTOOFF_THRESH,
#endif  // AUTOMATIC_DIMMER
	    MODE_CFGDISP_SETOFFTIME_MENU,
		MODE_CFGDISP_SETOFFTIME_TOGGLE,
		MODE_CFGDISP_SETOFFTIME_OFFHOUR,
		MODE_CFGDISP_SETOFFTIME_OFFMINUTE,
		MODE_CFGDISP_SETOFFTIME_ONHOUR,
		MODE_CFGDISP_SETOFFTIME_ONMINUTE,
	    MODE_CFGDISP_SETOFFDAYS_MENU,
		MODE_CFGDISP_SETOFFDAYS_OPTIONS,
		MODE_CFGDISP_SETOFFDAYS_CUSTOM,
	    MODE_CFGDISP_SETONDAYS_MENU,
		MODE_CFGDISP_SETONDAYS_OPTIONS,
		MODE_CFGDISP_SETONDAYS_CUSTOM,
	MODE_CFGDISP_SETANIMATED_MENU,
	    MODE_CFGDISP_SETANIMATED_TOGGLE,
    MODE_CFGREGN_MENU,
	MODE_CFGREGN_SETDST_MENU,
	    MODE_CFGREGN_SETDST_STATE,
	    MODE_CFGREGN_SETDST_ZONE,
#ifdef GPS_TIMEKEEPING
	MODE_CFGREGN_SETZONE_MENU,
	    MODE_CFGREGN_SETZONE_HOUR,
	    MODE_CFGREGN_SETZONE_MINUTE,
#endif  // GPS_TIMEKEEPING
	MODE_CFGREGN_DATEFMT_MENU,
	    MODE_CFGREGN_DATEFMT_SHOWWDAY,
	    MODE_CFGREGN_DATEFMT_FORMAT,
	    MODE_CFGREGN_DATEFMT_SHOWYEAR,
	MODE_CFGREGN_TIMEFMT_MENU,
	    MODE_CFGREGN_TIMEFMT_12HOUR,
	    MODE_CFGREGN_TIMEFMT_FORMAT,
	    MODE_CFGREGN_TIMEFMT_SHOWDST,
#ifdef GPS_TIMEKEEPING
	    MODE_CFGREGN_TIMEFMT_SHOWGPS,
#endif  // GPS_TIMEKEEPING
	MODE_CFGREGN_TEXTFMT_MENU,
	    MODE_CFGREGN_TEXTFMT_ZEROPAD,
	    MODE_CFGREGN_TEXTFMT_ALTNINE,
};


#define MODE_TMP_YEAR  0
#define MODE_TMP_MONTH 1
#define MODE_TMP_DAY   2

#define MODE_TMP_HOUR   0
#define MODE_TMP_MINUTE 1
#define MODE_TMP_SECOND 2

#define MODE_TMP_DAYS 1
#define MODE_TMP_IDX  2

#define MODE_TMP_SET 0
#define MODE_TMP_MIN 1
#define MODE_TMP_MAX 2


typedef struct {
    uint8_t  state;  // name of current state
    uint16_t timer;  // time in current state (semiseconds)
    int8_t  tmp[3];  // place to store temporary data
} mode_t;


extern volatile mode_t mode;


void mode_init(void);

void mode_tick(void);
void mode_semitick(void);

void mode_wake(void);
inline void mode_sleep(void) {};

void mode_alarmset(void);
void mode_alarmoff(void);
void mode_snoozing(void);

#endif  // MODE_H
