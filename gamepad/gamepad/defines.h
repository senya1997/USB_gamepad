#define F_CPU 16000000L

// SEGA:
	#define PORT_SEGA PORTB
	#define DDR_SEGA DDRB

	// 0..5 - PORTB, 7 - PORTD
	#define SEGA_UP_Z		0
	#define SEGA_DOWN_Y		1
	#define SEGA_LEFT_X		2
	#define SEGA_RIGHT_MODE	3
	#define SEGA_A_B		4
	#define SEGA_ST_C		5 /* "start" or "C" */
	#define SEGA_SEL		7 /* select */

// PS:
	#define PORT_PS PORTC
	#define DDR_PS DDRC

// LED:
	#define PORT_LED PORTD
	#define DDR_LED DDRD

	#define LED0 5
	#define LED1 6