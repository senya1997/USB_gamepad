#define F_CPU 16000000L

#define DEBUG
//#define DEBUG_SEGA
//#define DEBUG_PS

#ifdef DEBUG_SEGA
	#undef DEBUG_PS
#endif

#define REPORT_SIZE 6

#define STEP_IDLE_CONF	250	/* 4 ms step for calculate idle time in cnt of timer 0 with presc */
#define INIT_IDLE_TIME	4	/* 100 <=> 400 ms in cnt of timer 0 */

// for descriptors:
	#define UNUSED 0x00
	#define TOTAL_LEN_DESCR (9 + 9 + 9 + 7)

// LED:
	#define PORT_LED PORTD
	#define DDR_LED DDRD

	#define LED0 5
	#define LED1 6

// SEGA/PS choose controller:
	#define PORT_CTRL PORTD
	#define DDR_CTRL DDRD
	#define PIN_CTRL PIND
	#define CTRL 4 /* 0 - SEGA, 1 - PS */
	
/************************************************************************************************************************/
/*                                                        SEGA:                                                         */
/************************************************************************************************************************/

#define PORT_SEGA1 PORTB /* 1st player */
#define DDR_SEGA1 DDRB
#define PIN_SEGA1 PINB

#define PORT_SEGA2 PORTC /* 2nd player */
#define DDR_SEGA2 DDRC
#define PIN_SEGA2 PINC

#define PORT_SEGA_AUX PORTD /* one SEGA PIN on PORTD: bind by scheme */
#define DDR_SEGA_AUX DDRD

/******************************************************* ATTENTION ******************************************************/
/* SEGA PIN 1..4, 6, 9 must go sequentially on PORT and must fit PORT 1st and 2nd players (remember that it use for PS) */
/*						e.g. "PORT MC"-"PIN SEGA" match 2-1, 3-2, 4-3, 5-4, 6-6, 7-9								    */
/*												  or	0-1, 1-2, 2-3, 3-4, 4-6, 5-9							   	    */
/*						  do not remember upd "SEGA_PIN_MASK" for new bind "PORT MC"-"PIN SEGA"							*/
/************************************************************************************************************************/

// 0..5 - PORTB, 7 - PORTD (AUX)
	#define SEGA_UP_Z	0 /* PIN 1 */
	#define SEGA_DW_Y	1 /* PIN 2 */
	#define SEGA_LF_X	2 /* PIN 3 */
	#define SEGA_RG_MD	3 /* PIN 4 */
	#define SEGA_A_B	4 /* PIN 6 */
	#define SEGA_ST_C	5 /* PIN 9 */

	#define SEGA_SEL	7 /* PIN 7 this PIN must not coincide with PS controller PINs */

// mask data from "state" table on spec SEL state (see "state" comment in "main.c"):
	#define LF_RG_MASK	0b00000011
	#define UP_DW_MASK	0b00001100
	#define ZYX_MD_MASK	0b00001111
	#define C_B_MASK	0b00110000
	#define ST_A_MASK	0b00110000

	#define LF_MASK 0b00000010
	#define RG_MASK 0b00000001
	#define UP_MASK 0b00001000
	#define DW_MASK 0b00000100

#define SEGA_PIN_MASK 0b00111111	/* e.g. if SEGA buttons PIN match "PORTB 0..6" => MASK = 0b00111111, */
									/* because last 2 bits on PORTB - TOSC 1,2 */

#define PER_POLL_GP		30	/* period of SEL signal for gamepad in cnt of timer 2 with presc */
#define DELAY_BTW_POLL	255	/* delay between packets 0..7 of SEL signal in cnt of timer 2 with presc, */
							/* for reset internal cnt in gamepad (minimum required 1.6 ms) */
#define DELAY_BEF_POLL	10	/* delay after front of SEL signal (before polling buttons) in cnt of timer 2 with presc */

/************************************************************************************************************************/
/*                                                         PS:                                                          */
/************************************************************************************************************************/

#define PORT_PS PORTB
#define DDR_PS DDRB

// controllers use 3.3 V
// no one pins number must not coincide with SEL pin SEGA controller
	#define PS_MISO 4 /* PS pin 1: "DATA", always "0" pin MC, must be pull up to 3.3 or 5 V through 1kOhm */
	#define PS_MOSI 3 /* PS pin 2: "CMD" */
	#define PS_CS	2 /* PS pin 6: "ATT", attention new packet */
	#define PS_CLK	5 /* PS pin 7: ~7 kHz */
	
	//#define PS_ACK	4 /* Pin 9: acknowledge, must be pullup to 3.3 or 5 V through 1kOhm */
	
#define SPI_FROZE 10000 /* in tact, while wait SPI ready anti frozen counter */