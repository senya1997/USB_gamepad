#define F_CPU 16000000L

// SEGA:
	#define PORT_SEGA PORTB
	#define DDR_SEGA DDRB
	#define PIN_SEGA PINB
	
	// 0..5 - PORTB, 7 - PORTD
	#define SEGA_UP_Z	0 /* PIN 1 */
	#define SEGA_DW_Y	1 /* PIN 2 */
	#define SEGA_LF_X	2 /* PIN 3 */
	#define SEGA_RG_MD	3 /* PIN 4 */
	#define SEGA_A_B	4 /* PIN 6 */
	#define SEGA_ST_C	5 /* PIN 9 */
	
	#define SEGA_SEL	7 /* PIN 7 */

// PS:
	#define PORT_PS PORTC
	#define DDR_PS DDRC

// LED:
	#define PORT_LED PORTD
	#define DDR_LED DDRD

	#define LED0 5
	#define LED1 6

#define STEP_IDLE_CONF	250	/* 4 ms step for calculate idle time in count of timer 0 */

#define PER_POLL_GP		62	/* period of SEL signal for gamepad in count of timer 2  */
#define DELAY_BTW_POLL	250	/* delay between packets of 0..7 SEL signals in count of timer 2  */
#define DELAY_BEF_POLL	4	/* delay before polling buttons in count of timer 2 */

#define REPORT_SIZE		3	/* defined by HID report */
#define INIT_IDLE_TIME	120	/* 100 <=> 400 ms */