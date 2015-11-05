/* ---------------------------------------------------------------------------
 * File: ui.h
 * Copyright(C) 2015 Mircom Group
 * ---------------------------------------------------------------------------
 */

#ifndef __UI_H__
#define __UI_H__

/* Key Switch Id */
enum ui_key {
	UI_KEY_ACK = 1 /* Ack Push Button */
};

/* LED Id */
enum ui_led {
	UI_LED1 = 1,
	UI_LED2 = 2,
	UI_LED3 = 3,
	UI_LED4 = 4,
	UI_LED5 = 5,
	UI_LED6 = 6,
	UI_LED7 = 7,
	UI_LED8 = 8
};

#ifdef __cplusplus
extern "C" {
#endif

/* Turn a LED steady on */
void ui_led_on(unsigned int id);

/* Turn a LED off */
void ui_led_off(unsigned int id);

/* Blinks a LED at a rate of 2 flashes per second */
void ui_led_blink(unsigned int id);

/* Wait for a key press and returns the Key Id */
int ui_key_get(void);

/* Initializes the User Interface */
void ui_init(void);

#ifdef __cplusplus
}
#endif	

#endif /* __UI_H__ */

