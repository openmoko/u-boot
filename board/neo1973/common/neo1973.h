#ifndef _NEO1973_H
#define _NEO1973_H

enum wakeup_reason {
	NEO1973_WAKEUP_NONE,
	NEO1973_WAKEUP_RESET,
	NEO1973_WAKEUP_POWER_KEY,
	NEO1973_WAKEUP_CHARGER,
	NEO1973_WAKEUP_ALARM,
};

enum neo1973_charger_cmd {
	NEO1973_CHGCMD_NONE,
	NEO1973_CHGCMD_AUTOFAST,
	NEO1973_CHGCMD_NO_AUTOFAST,
	NEO1973_CHGCMD_OFF,
	NEO1973_CHGCMD_FAST,
	NEO1973_CHGCMD_FASTER,
};

extern unsigned int neo1973_wakeup_cause;

void neo1973_poweroff(void);
void neo1973_backlight(int on);
void neo1973_vibrator(int on);
void neo1973_gsm(int on);
void neo1973_gps(int on);
void neo1973_led(int led, int on);

int neo1973_911_key_pressed(void);

const char *neo1973_get_charge_status(void);
int neo1973_set_charge_mode(enum neo1973_charger_cmd cmd);

int neo1973_new_second(void);
int neo1973_on_key_pressed(void);
int neo1973_aux_key_pressed(void);

void neo1973_bootmenu(void);

#endif
