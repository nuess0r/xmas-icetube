// pizo.c  --  pizo element control
//
//    PB2 (OC1B)        first  buzzer pin
//    PB1 (OC1A)        second buzzer pin
//    counter/timer1    buzzer pin (OC1A & OC1B) control
//


#include <avr/io.h>      // for using avr register names
#include <avr/power.h>   // for enabling and disabling timer1
#include <avr/eeprom.h>  // for accessing eeprom

#include "pizo.h"
#include "system.h" // alarm behavior depends on power source


// extern'ed pizo data
volatile pizo_t pizo;


// macros for each note in an octave
#define Cn 0   // C  (C normal)
#define Cs 1   // C# (C sharp)
#define Df Cs  // Db (D flat)
#define Dn 2   // D  (D normal)
#define Ds 3   // D# (D sharp)
#define Ef Ds  // Eb (E flat)
#define En 4   // E  (E normal)
#define Fn 5   // F  (F normal)
#define Fs 6   // F# (F sharp)
#define Gf Fs  // Gb (G flat)
#define Gn 7   // G  (G normal)
#define Gs 8   // G# (G sharp)
#define Af Gs  // Ab (A flat)
#define An 9   // A  (A normal)
#define As 10  // A# (A sharp)
#define Bf As  // Bb (B flat)
#define Bn 11  // B  (B normal)


// macro for placing each note in an octave: a note
// is stored in the lower nibble; an octave, the upper
#define N(note, octave) ((octave << 4) | note)
#define NOTE_MASK   0x0F
#define OCTAVE_MASK 0xF0

// other possible "sounds"
// (since a note must have an octave of at least three,
// any value, less than 48 (3 * 2^8) is not a valid note)
#define PAUSE  0  // silence instead of note
#define BEEP   1  // arbitrary beep sound


// The table below is used to convert alarm volume (0 to 10) into timer
// settings.  The values were derived by ear.  With the exception of the
// first two (2 and 7), perceived volume seems roughly proportional to
// the log of the values below.  (cm = compare match)
const uint8_t pizo_vol2cm[] PROGMEM = {2,7,11,15,21,28,38,51,69,93,128};


// timer1 top values for the third octave;
// timer values for other octaves can be derived
// by dividing by powers of two (right bitshifts)
const uint16_t third_octave[] PROGMEM = {
    F_CPU / 130.81,  // C
    F_CPU / 138.59,  // C#, Db
    F_CPU / 146.83,  // D
    F_CPU / 155.56,  // D#, Eb
    F_CPU / 164.81,  // E
    F_CPU / 174.61,  // F
    F_CPU / 185.00,  // F#, Gb
    F_CPU / 196.00,  // G
    F_CPU / 207.65,  // G#, Ab
    F_CPU / 220.00,  // A
    F_CPU / 233.08,  // A#, Bb
    F_CPU / 246.94,  // B
};


// the notes of "merry christmas"
const uint8_t merry_xmas_notes[] PROGMEM = {
    N(Dn,6),
    N(Gn,6), N(Gn,6), N(An,6), N(Gn,6), N(Fs,6),
    N(En,6), N(En,6), N(En,6),
    N(An,6), N(An,6), N(Bn,6), N(An,6), N(Gn,6),
    N(Fs,6), N(Dn,6), N(Dn,6),
    N(Bn,6), N(Bn,6), N(Cn,7), N(Bn,6), N(An,6),
    N(Gn,6), N(En,6), N(En,6), N(En,6),
    N(En,6), N(An,6), N(Fs,6),
    N(Gn,6),

    N(Dn,6),
    N(Gn,6), N(Gn,6), N(Gn,6),
    N(Fs,6), N(Fs,6),
    N(Gn,6), N(Fs,6), N(En,6),
    N(Dn,6), N(Bn,6),
    N(Cn,7), N(Bn,6), N(An,6),
    N(Dn,7), N(Dn,6), N(Dn,6), N(Dn,6),
    N(Dn,6), N(An,6), N(Fs,6),
    N(Gn,6), PAUSE,
0};


// the timing of "merry christmas"
const uint8_t merry_xmas_times[] PROGMEM = {
    2,
    2, 1, 1, 1, 1,
    2, 2, 2,
    2, 1, 1, 1, 1,
    2, 2, 2,
    2, 1, 1, 1, 1,
    2, 2, 1, 1,
    2, 2, 2,
    4,

    2,
    2, 2, 2,
    4, 2,
    2, 2, 2,
    4, 2,
    2, 2, 2,
    2, 2, 1, 1,
    2, 2, 2,
    4, 2,
0};


uint8_t ee_pizo_sound EEMEM = PIZO_DEFAULT_SOUND;


void pizo_init(void) {
    // configure buzzer pins
    DDRB  |=  _BV(PB2) |  _BV(PB1);  // set as outputs
    PORTB &= ~_BV(PB2) & ~_BV(PB1);  // clamp to ground

    // if any timer is disabled during sleep, the system locks up sporadically
    // and nondeterministically, so enabled timer1 in PRR and leave it alone!
    power_timer1_enable();

    pizo_loadsound();
}


// load alarm sound from eeprom
void pizo_loadsound(void) {
    pizo.status = eeprom_read_byte(&ee_pizo_sound) & PIZO_SOUND_MASK;
    pizo_configsound();
}


// save alarm sound to eeprom
void pizo_savesound(void) {
    eeprom_write_byte(&ee_pizo_sound, pizo.status & PIZO_SOUND_MASK);
}


// configure needed settings for current sound;
// also ensures valid current sound
void pizo_configsound(void) {
    switch(pizo.status & PIZO_SOUND_MASK) {
	case PIZO_SOUND_MERRY_XMAS:
	    pizo.notes = merry_xmas_notes;
	    pizo.times = merry_xmas_times;
	    break;
	default:
	    pizo.status &= ~PIZO_SOUND_MASK;
	    pizo.status |=  PIZO_SOUND_BEEPS;
	    pizo.notes  = merry_xmas_notes;
	    pizo.times  = merry_xmas_times;
	    break;
    }
}


// change alarm sound
void pizo_nextsound(void) {
    switch(pizo.status & PIZO_SOUND_MASK) {
	case PIZO_SOUND_BEEPS:
	    pizo.status &= ~PIZO_SOUND_MASK;
	    pizo.status |=  PIZO_SOUND_MERRY_XMAS;
	    break;
	case PIZO_SOUND_MERRY_XMAS:
	    pizo.status &= ~PIZO_SOUND_MASK;
	    pizo.status |=  PIZO_SOUND_BEEPS;
	    break;
	default:
	    pizo.status &= ~PIZO_SOUND_MASK;
	    pizo.status |=  PIZO_SOUND_BEEPS;
	    break;
    }

    pizo_configsound();
}


// set pizo volume to vol is [0-10]
// (interpolation between vol and vol+1 using interp)
void pizo_setvolume(uint8_t vol, uint8_t interp) {
    pizo.cm_factor = pgm_read_byte(pizo_vol2cm + vol);

    if(vol < 10 && interp) {
	pizo.cm_factor    = pgm_read_byte(pizo_vol2cm+vol);
	uint16_t cm_slope = pgm_read_byte(pizo_vol2cm+vol+1) - pizo.cm_factor;
	pizo.cm_factor   += ((cm_slope * interp) >> 8);
    }

    // if the buzzer is active, adjust volume immediately
    if(TCCR1A) {
	OCR1A = (ICR1 >> 8) * pizo.cm_factor;
	OCR1B = ICR1 - OCR1A;
    }
}


// configure pizo control for full-power mode
void pizo_wake(void) {
    if((pizo.status & PIZO_STATE_MASK) == PIZO_ALARM_BEEPS) {
	if(TCCR1A) {  // if the buzzer is active,
	    // compensate for 4x faster clock
	    ICR1  <<= 2;
	    OCR1A <<= 2;
	    OCR1B = ICR1 - OCR1A;
	}
    }
}


// silence any nonalarm noises during sleep
void pizo_sleep(void) {
    switch(pizo.status & PIZO_STATE_MASK) {
	case PIZO_ALARM_MUSIC:
	    // switch to beeps to reduce power consumption
	    pizo.status &= ~PIZO_STATE_MASK;
	    pizo.status |=  PIZO_ALARM_BEEPS;
	    pizo.timer   = 0;
	    pizo_buzzeroff();
	    break;

	case PIZO_ALARM_BEEPS:
	    if(TCCR1A) {  // if the buzzer is active,
		// compensate for 4x slower clock
		ICR1  >>= 2;
		OCR1A >>= 2;
		OCR1B = ICR1 - OCR1A;
	    }

	case PIZO_INACTIVE:
	    break;

	default:
	    pizo_stop();
	    break;
    }
}


// toggles buzzer each second
void pizo_tick(void) {
    switch(pizo.status & PIZO_STATE_MASK) {
	case PIZO_ALARM_BEEPS:
	    ++pizo.timer;

	    if(pizo.timer & 0x0001) {
		pizo_buzzeron(BEEP);
		system.status |= SYSTEM_ALARM_SOUNDING;
	    } else {
		pizo_buzzeroff();
		system.status &= ~SYSTEM_ALARM_SOUNDING;
	    }
	    break;

	default:
	    break;
    }
}


// controls pizo element depending on current state
void pizo_semitick(void) {
    switch(pizo.status & PIZO_STATE_MASK) {
	case PIZO_BEEP:
	    // stop buzzer if beep has timed out
	    if(!pizo.timer) pizo_stop();

	    --pizo.timer;

	    break;

	case PIZO_CLICK:
	    if(pizo.timer == PIZO_CLICKTIME / 2) {
		// flip from +5v to -5v on pizo
		PORTB |=  _BV(PB2);
		PORTB &= ~_BV(PB1);
	    }

	    if(!pizo.timer) pizo_stop();

	    --pizo.timer;

	    break;

	case PIZO_TRYALARM_BEEPS:
	    if(!pizo.timer) {
		pizo_buzzeron(BEEP);
		pizo.timer = 2020;
	    }

	    if(pizo.timer == 1010) {
		pizo_buzzeroff();
	    }

	    --pizo.timer;

	    break;

	case PIZO_TRYALARM_MUSIC:
	case PIZO_ALARM_MUSIC:
	    // when timer expires, play next note or pause
	    if(!pizo.timer) {
		pizo.timer = pgm_read_byte(&(pizo.times[pizo.pos]));

		if(!pizo.timer) {  // zero indicates end-of-tune,
		    pizo.pos = 0;  // so repeat from beginning
		    pizo.timer = pgm_read_byte(&(pizo.times[pizo.pos]));
		}

		pizo.timer <<= 8;  // 256 semiticks per time unit

		// play required note
		pizo_buzzeron(pgm_read_byte(&(pizo.notes[pizo.pos])));

		++pizo.pos; // increment note index
	    }

	    // brief pause to make notes distinct
	    if(pizo.timer == 32) pizo_buzzeroff();

	    --pizo.timer;

	    break;

	default: // PIZO_INACTIVE, PIZO_TRYALARM_BEEPS, or PIZO_ALARM_BEEPS
	    break;
    }
}


// enables buzzer with given sound: PAUSE, BEEP, or N(a,b)
void pizo_buzzeron(uint8_t sound) {
    uint16_t top_value; uint8_t top_shift;

    if(sound == PAUSE) {
	pizo_buzzeroff();
	return;
    } else if(sound == BEEP) {
	top_value = 2048;
	top_shift = 0;
    } else {
	// calculate the number of octaves above the third
	// (the upper nibble of a note specifies the octave)
	top_shift = ((sound & OCTAVE_MASK) >> 4) - 3;

	// find TOP for desired note in the third octave
	// (the lower nibble of a note specifies the index)
	top_value = pgm_read_word(&(third_octave[sound & NOTE_MASK]));
    }

    // shift counter top from third octave to desired octave
    top_value >>= top_shift;

    if(system.status & SYSTEM_SLEEP) {
	// compensate frequency for 4x slower clock
	top_value >>= 2;  // divide by four
    }

    // determine compare match value for given top
    uint16_t compare_match = (top_value >> 8) * pizo.cm_factor;


    // configure Timer/Counter1 to generate buzzer sound
    // COM1A1 = 10, clear OC1A on Compare Match, set OC1A at BOTTOM
    // COM1B1 = 11, set OC1B on Compare Match, clear OC1B at BOTTOM
    TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(COM1B0) | _BV(WGM11);

    // WGM1 = 1110, fast PWM, TOP is ICR1
    // CS1  = 001,  clock timer with system clock
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);

    // set TOP for desired frequency
    ICR1 = top_value;

    // set compare match registers for desired volume
    OCR1A = compare_match;
    OCR1B = top_value - compare_match;
}


// disable the buzzer
void pizo_buzzeroff(void) {
    // disable timer/counter1 (buzzer timer)
    TCCR1A = 0; TCCR1B = 0;

    // pull speaker pins low
    PORTB &= ~_BV(PB2) & ~_BV(PB1);
}


// make a clicking sound
void pizo_click(void) {
    // only click if pizo inactive
    if((pizo.status & PIZO_STATE_MASK) == PIZO_INACTIVE) {
	// set state and timer, so click can be completed
	// in subsequent calls to pizo_semitick()
	pizo.status &= ~PIZO_STATE_MASK;
	pizo.status |=  PIZO_CLICK;
	pizo.timer   =  PIZO_CLICKTIME;

	// apply +5v to buzzer
	PORTB |=  _BV(PB1);
	PORTB &= ~_BV(PB2);
    }
}


// beep for the specified duration (semiseconds)
void pizo_beep(uint16_t duration) {
    switch(pizo.status & PIZO_STATE_MASK) {
	// a beep interrupts anything except alarm sounds
	case PIZO_ALARM_MUSIC:
	case PIZO_ALARM_BEEPS:
	case PIZO_TRYALARM_MUSIC:
	case PIZO_TRYALARM_BEEPS:
	    return;

	default:
	    // override any existing noise
	    pizo_buzzeroff();

	    // set timer and flag, so beep routine can be
	    // completed in subsequent calls to pizo_semitick()
	    pizo.status &= ~PIZO_STATE_MASK;
	    pizo.status |=  PIZO_BEEP;
	    pizo.timer   =  duration;

	    pizo_buzzeron(BEEP);
	    break;
    }
}


// start alarm sounding
void pizo_alarm_start(void) {
    // override any existing noise
    pizo_buzzeroff();

    // set state
    pizo.status &= ~PIZO_STATE_MASK;
    pizo.status |= ((pizo.status & PIZO_SOUND_MASK) == PIZO_SOUND_BEEPS 
	    	        || system.status & SYSTEM_SLEEP ?
		    PIZO_ALARM_BEEPS : PIZO_ALARM_MUSIC);

    // reset music poisition and timer
    pizo.pos   = 0;
    pizo.timer = 0;
}


// stop alarm sounding
void pizo_alarm_stop(void) {
    switch(pizo.status & PIZO_STATE_MASK) {
	// only stop a tryalarm state
	case PIZO_ALARM_MUSIC:
	case PIZO_ALARM_BEEPS:
	    // override any existing noise
	    pizo_stop();
	    break;

	default:
	    break;
    }
}


// start alarm sounding demo
void pizo_tryalarm_start(void) {
    switch(pizo.status & PIZO_STATE_MASK) {
	// a tryalarm interrupts anything except an alarm
	case PIZO_ALARM_MUSIC:
	case PIZO_ALARM_BEEPS:
	    return;

	default:
	    pizo_buzzeroff();

	    // set state
	    pizo.status &= ~PIZO_STATE_MASK;
	    pizo.status |= ((pizo.status & PIZO_SOUND_MASK) == PIZO_SOUND_BEEPS ?
			     PIZO_TRYALARM_BEEPS : PIZO_TRYALARM_MUSIC);

	    // reset music poisition and timer
	    pizo.pos   = 0;
	    pizo.timer = 0;
	    break;
    }
}


// stop tryalarm noise
void pizo_tryalarm_stop(void) {
    switch(pizo.status & PIZO_STATE_MASK) {
	// only stop a tryalarm state
	case PIZO_TRYALARM_MUSIC:
	case PIZO_TRYALARM_BEEPS:
	    // override any existing noise
	    pizo_stop();
	    break;

	default:
	    break;
    }
}


// silences pizo and changes state to inactive
void pizo_stop(void) {
    // override any existing noise
    pizo_buzzeroff();
    pizo.status &= ~PIZO_STATE_MASK;
    pizo.status |=  PIZO_INACTIVE;
    system.status &= ~SYSTEM_ALARM_SOUNDING;
}