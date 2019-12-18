#include "defines.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "descriptor.h"

#ifdef DEBUG
	#warning "DEBUG is enabled"
#endif

#ifdef PROTEUS
	#warning "PROTEUS SIM is enabled"
#endif

uchar report_buf[REPORT_SIZE] = {0x7F, 0x7F, 0x7F, 0x7F, 0x00, 0x00};

uchar delay_idle = INIT_IDLE_TIME; // step - 4ms
uchar cnt_idle = 0;
uchar flag_idle = 0; // shows that idle time is over and can send report

uchar flag_report = 0; // shows that information from gamepad is ready to form like in descriptor

// PS var and protocol:
	uchar shift_report_buf[REPORT_SIZE];

	uchar cnt_byte = 0;
	uchar cnt_edge = 0;
	uchar cnt_rep_buf = 5;
	
	uchar offset;
	
/*********************************************************************************/
/* CLK ~ 7 kHz, issue data LSB on MISO and MOSI on falling edge, read on front   */
/* seq from MC:  0x01 | 0x42 | 0xFF | 0xFF | 0xFF | 0xFF | 0xFF | 0xFF | 0xFF    */
/* seq from JOY: 0xFF | 0x73 | 0x5A | DAT1 | DAT2 | RJX  | RJY  | LJX  | LJY     */
/*		       ___														   _____ */
/*		   CS:	  |_______________________________________________________|	     */
/*             _____   _   _   _   _   _   _   _   _____   _  		 _	 _______ */
/*		  CLK:      |_| |_| |_| |_| |_| |_| |_| |_|     |_| |_ ... _| |_|		 */
/*					  r   r   r   r   r   r   r   r   r  						 */
/* MOSI, MISO: -----.000.111.222.333.444.555.666.777.---.000.1 ... 666.777.----- */
/*			   _____________________________________1CLK______ ... _____________ */
/*		  ACK:									    |__|						 */
/*********************************************************************************/

USB_PUBLIC uchar usbFunctionDescriptor(usbRequest_t * rq)
{
	if (rq->bRequest == USBRQ_GET_DESCRIPTOR)
	{
		switch(rq -> wValue.bytes[1])
		{
			case USBDESCR_DEVICE:
				usbMsgPtr = (usbMsgPtr_t)desc_dev;
				return sizeof(desc_dev);
			case USBDESCR_CONFIG:
				usbMsgPtr = (usbMsgPtr_t)desc_conf;
				return sizeof(desc_conf);
			case USBDESCR_STRING:
				if(rq -> wValue.bytes[0] == 2) // device name
				{
					usbMsgPtr = (usbMsgPtr_t)desc_prod_str;
					return sizeof(desc_prod_str);
				}
		}
	}
	
	return 0x00;
}

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
	usbRequest_t *rq = (usbRequest_t*)data;
	
	if((rq -> bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) // class request type
	{    
		switch (rq -> bRequest)
		{
			case USBRQ_HID_GET_REPORT:
				usbMsgPtr = (usbMsgPtr_t)report_buf;
				return REPORT_SIZE;
			case USBRQ_HID_GET_IDLE:
				if (rq -> wValue.bytes[0] > 0)
				{
					usbMsgPtr = (usbMsgPtr_t)&delay_idle;
					return 1;
				}
				break;
			case USBRQ_HID_SET_IDLE: return USB_NO_MSG; // call "usbFunctionWrite" ("OUT" token)
			/*
			case USBRQ_HID_SET_IDLE:
				// mb required reset "cnt_idle" and enable interrupt timer 0, cause 
				// "flag_idle" can be set during "delay_idle" change in large way
				if(rq -> wValue.bytes[1] != 0) delay_idle = rq -> wValue.bytes[1];
				else delay_idle = INIT_IDLE_TIME; // when the upper byte of "wValue" = 0, the duration is indefinite
				*/
		}
	}
	
	return 0; // ignore data from host ("OUT" token)
}

USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len)
{
	usbRequest_t *rq = (usbRequest_t*)data;
	
	if(rq -> wValue.bytes[1] != 0) delay_idle = rq -> wValue.bytes[1];
	else delay_idle = INIT_IDLE_TIME; // when the upper byte of "wValue" = 0, the duration is indefinite
	
	return 1;
}

inline void hardwareInit()
{
	DDR_LED = (1 << LED0) | (1 << LED1);

	// for idle time:
	TCCR1B = (1 << CS10);
	TIMSK1 = (1 << TOIE1);
	
// this func call after "hardware_SEGA_init" when PS mode is activating
	DDR_PS &= ~(1 << PS_MISO) & ~(1 << PS_ACK); // inputs
#ifdef DEBUG
	DDR_PS |= (1 << PS_CS) | (1 << PS_MOSI) | (1 << PS_CLK) | (1 << PS_DEBUG); // outputs
#else 
	DDR_PS |= (1 << PS_CS) | (1 << PS_MOSI) | (1 << PS_CLK);
#endif
	
// add pullup on inputs and issue one on outputs:
// 				***** ATTENTION *****
// on MISO PULLUP external and must be turn off on mc
	PORT_PS &= ~(1 << PS_ACK) & ~(1 << PS_MISO); // no pullup
	PORT_PS = (1 << PS_CS) | (1 << PS_CLK);
	
// duplicate main timer:
	TCCR0A = (1 << WGM01);
	TCCR0B = (1 << CS01); // presc = 8 => 70 us <=> 140 cnt
	TIMSK0 = (1 << OCIE0A);
	OCR0A = CLK_HALF_PER + DELTA;
	
// for PS CLK ~ 7 kHz, DO NOT forget to approve with CPU freq:
	TCCR2A = (1 << WGM21); // CTC mode with OCR2A 
	TCCR2B = (1 << CS21); // presc = 8 => half period CLK 70 us <=> 140 cnt
	OCR2A = CLK_HALF_PER;
	TIMSK2 = (1 << OCIE2A);
}

ISR(TIMER1_OVF_vect)
{
	TIMSK0 &= ~(1 << OCIE0A);
	TIMSK2 &= ~(1 << OCIE2A);
	
		sei();
		
		if(cnt_idle < delay_idle) cnt_idle++;
		else
		{
			TIMSK1 &= ~(1 << TOIE1);
			flag_idle = 1;
		}
	
	TIMSK0 |= (1 << OCIE0A);
	TIMSK2 |= (1 << OCIE2A);
}

#define END_ONE_BYTE (cnt_edge == 18)
#define ACT_TRANS (cnt_edge < 16)
#define IDLE_STATE (cnt_edge == 15)
#define LAST_BYTE (cnt_byte == 9)

ISR(TIMER2_COMPA_vect)
{	
	TCNT0 = 5; // reset duplicate timer
	TIMSK0 &= ~(1 << OCIE0A); // "cli"
	
	sei();
	
	if((TIFR0 & (1 << OCF0A)) == (1 << OCF0A)) clearShiftBuf();
	
	// CLK:
	if((cnt_byte > 0) & ACT_TRANS) PORT_PS ^= (1 << PS_CLK);
	
	// MISO:
	if((cnt_byte > 3) & ACT_TRANS & ((cnt_edge & 0x01) == 1)) // odd "cnt_edge"
	{
		offset = (cnt_edge - 1) >> 1;
		shift_report_buf[cnt_rep_buf] |= (PIN_PS & 0x01) << offset;
	}
	
	// MOSI:
	if(((cnt_byte == 1) & (cnt_edge < 2)) |
	  ((cnt_byte == 2) & ((cnt_edge == 2) | (cnt_edge == 3) | (cnt_edge == 12) | (cnt_edge == 13)))) PORT_PS |= (1 << PS_MOSI);
	else PORT_PS &= ~(1 << PS_MOSI);
	
	// CS:
	if((cnt_byte == 0) & IDLE_STATE) PORT_PS &= ~(1 << PS_CS);
	else if(LAST_BYTE & END_ONE_BYTE) PORT_PS |= (1 << PS_CS);
	
	// counters - required go to ASM:
	if(cnt_byte == 0) // between byte transmit
	{
		if(IDLE_STATE)
		{
			cnt_edge = 0;
			cnt_byte++;
		}
		else cnt_edge++;
	}
	else if((cnt_byte > 0) & (cnt_byte < 4)) 
	{
		if(END_ONE_BYTE)
		{
			cnt_edge = 0;
			cnt_byte++;
		}
		else cnt_edge++;
	}
	else
	{
		if(END_ONE_BYTE)
		{
			cnt_edge = 0;
			cnt_rep_buf--;
			
			if(LAST_BYTE)
			{
				flag_report = 1;
				cnt_byte = 0;
				
				//TIMSK1 &= ~(1 << OCIE1A);
			}
			else cnt_byte++;
		}
		else cnt_edge++;
	}
	
	TIMSK0 |= (1 << OCIE0A); // "sei"
}

inline void clearShiftBuf()
{
	if(!flag_report)
	{
		shift_report_buf[0] = 0;
		shift_report_buf[1] = 0;
		shift_report_buf[2] = 0;
		shift_report_buf[3] = 0;
		shift_report_buf[4] = 0;
		shift_report_buf[5] = 0;	
	}
	
	cnt_rep_buf = 5;
	
	cnt_edge = 0;
	cnt_byte = 0;
	
	PORT_PS |= (1 << PS_CS) | (1 << PS_CLK);
	TIFR0 |= (1 << OCF0A);
}

void main()
{
	char flag_report_rdy = 0;
	
	hardwareInit();
	
	#ifndef DEBUG
		usbDeviceConnect();
		usbInit();
	#endif
	
// full reset timers:
	TCNT0 = 0;
	TCNT1 = 0;
	TCNT2 = 0;
	
	TIFR0 |= (1 << OCF0A);
	TIFR1 |= (1 << TOV1);
	TIFR2 |= (1 << OCF2A);
	
	sei();
    while (1) 
    {
		if(flag_idle & flag_report_rdy) // send report immediately after "idle" time has passed:
		{
			if(usbInterruptIsReady())
			{
				#ifndef DEBUG
					usbSetInterrupt(report_buf, REPORT_SIZE);  // ~ 31.5 us
				#endif
				
				#ifdef PROTEUS
					usbSetInterrupt(report_buf, REPORT_SIZE);
				#endif
				
				//clearShiftBuf();
				flag_report_rdy = 0;
				
			// full reset idle timer then enable interrupt:
				TCNT1 = 0;
				TIFR1 |= (1 << TOV1); 
				TIMSK1 |= (1 << TOIE1);
			}
		}
		
		if(flag_report) // build report:
		{
			report_buf[0] = shift_report_buf[1]; // mb required "turn over" descriptor
			report_buf[1] = shift_report_buf[0];
			report_buf[2] = shift_report_buf[3];
			report_buf[3] = shift_report_buf[2];
			report_buf[4] = ~shift_report_buf[4];
			report_buf[5] = ~shift_report_buf[5];
			
			shift_report_buf[0] = 0;
			shift_report_buf[1] = 0;
			shift_report_buf[2] = 0;
			shift_report_buf[3] = 0;
			shift_report_buf[4] = 0;
			shift_report_buf[5] = 0;
			
			cnt_rep_buf = 5;
			
			PORT_PS |= (1 << PS_CS) | (1 << PS_CLK);
			
			flag_report = 0;
			flag_report_rdy = 1;
		}
		
		if((report_buf[4] != 0x00) | (report_buf[5] != 0x00)) PORT_LED ^= (1 << LED0);
		
		if((cnt_byte == 0) & (cnt_edge == 10))
		{
			 usbPoll();
		}
    }
}