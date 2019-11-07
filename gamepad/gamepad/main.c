#include "defines.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "descriptor.h"

uchar report_buf[REPORT_SIZE] = {0x7F, 0x7F, 0x7F, 0x7F, 0x00, 0x00}; // OX, OY, RX, RY, but 1st plr, but 2nd plr
	
uchar delay_idle = INIT_IDLE_TIME; // step - 4ms
uchar cnt_idle = 0;

uchar state = 0; // 0..7 states
/*  _____________________________
	|Sel |D0 |D1 |D2 |D3 |D4 |D5 |
	+----+---+---+---+---+---+---+
	| L  |UP |DW |LO |LO |A  |ST |
	| H  |UP |DW |LF |RG |B  |C  |
	| L  |UP |DW |LO |LO |A  |ST |
	| H  |UP |DW |LF |RG |B  |C  |
	| L  |LO |LO |LO |LO |A  |ST |
	| H  |Z  |Y  |X  |MD |HI |HI |
	| L  |HI |HI |HI |HI |A  |ST |
	| H  |UP |DW |LF |RG |B  |C  |
*/

uchar flag_ch_gp = 1; // shows that required save buttons state
uchar flag_report = 0;
uchar flag_idle = 0; // shows that idle time is over and can send report

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
			//case USBRQ_HID_SET_IDLE: return USB_NO_MSG; // call "usbFunctionWrite" ("OUT" token)
			case USBRQ_HID_SET_IDLE:
				// mb required reset "cnt_idle" and enable interrupt timer 0, cause 
				// "flag_idle" can be set during "delay_idle" change in large way
				if(rq -> wValue.bytes[1] != 0) delay_idle = rq -> wValue.bytes[1];
				else delay_idle = 3; // when the upper byte of "wValue" = 0, the duration is indefinite
		}
	}
	
	return 0; // ignore data from host ("OUT" token)
}

/*
USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len)
{
	usbRequest_t *rq = (usbRequest_t*)data;
	
	if(rq -> wValue.bytes[1] != 0) delay_idle = rq -> wValue.bytes[1];
	else delay_idle = 0; // when the upper byte of "wValue" = 0, the duration is indefinite
	
	return 1;
}
*/

uchar *updReportBuf(uchar offset, uchar *gp_state_ptr) // offset defines by player number: 1st - "0", 2nd - "8"
{
	static uchar int_report_buf[3]; // internal report buf: on exit of function - OX[0], OY[1], buttons[2]
	uchar temp;
	
			// 2,3,5 - SEL number at which data were polling in protocol (see "state" comment)
	temp = (~(*(gp_state_ptr + 2 + offset))) & ((1 << SEGA_A_B) | (1 << SEGA_ST_C));	// 0b00110000
	int_report_buf[2] = temp << 2;

	temp = (~(*(gp_state_ptr + 3 + offset))) & ((1 << SEGA_A_B) | (1 << SEGA_ST_C));	// 0b00110000
	int_report_buf[2] |= temp;

	temp = (~(*(gp_state_ptr + 5 + offset))) & ((1 << SEGA_UP_Z) | (1 << SEGA_DW_Y) |
												(1 << SEGA_LF_X) | (1 << SEGA_RG_MD));	// 0b00001111
	int_report_buf[2] |= temp;

	temp = (~(*(gp_state_ptr + 3 + offset))) & ((1 << SEGA_UP_Z) | (1 << SEGA_DW_Y));	// 0b00000011
		if(temp == (1 << SEGA_UP_Z)) int_report_buf[1] = 0x00;
		else if(temp == (1 << SEGA_DW_Y)) int_report_buf[1] = 0xFF;
		else int_report_buf[1] = 0x7F;

	temp = (~(*(gp_state_ptr + 3 + offset))) & ((1 << SEGA_LF_X) | (1 << SEGA_RG_MD));	// 0b00001100
		if(temp == (1 << SEGA_RG_MD)) int_report_buf[0] = 0xFF;
		else if(temp == (1 << SEGA_LF_X)) int_report_buf[0] = 0x00;
		else int_report_buf[0] = 0x7F;
	
	return int_report_buf; // return pointer on massive
}

void hardwareInit()
{
	DDR_LED = (1 << LED0) | (1 << LED1);
		
	// gamepads:
	DDR_SEGA_AUX = (1 << SEGA_SEL);
	PORT_SEGA_AUX &= ~(1 << SEGA_SEL); // necessarily down to zero SEL signal on start
		
	PORT_SEGA1 = (1 << SEGA_LF_X) | (1 << SEGA_RG_MD) | (1 << SEGA_UP_Z) | (1 << SEGA_DW_Y) | // add pull up (mb not required)
				 (1 << SEGA_A_B) | (1 << SEGA_ST_C);
	PORT_SEGA2 = (1 << SEGA_LF_X) | (1 << SEGA_RG_MD) | (1 << SEGA_UP_Z) | (1 << SEGA_DW_Y) |
				 (1 << SEGA_A_B) | (1 << SEGA_ST_C);
		
	//DDR_PS =
	//PORT_PS =
		
	// timers:
	TCCR0A = (1 << WGM01); // CTC mode with OCRA
	TCCR0B = (1 << CS02); // presc = 256 => 4 ms <=> 250 cnt
	OCR0A = STEP_IDLE_CONF;
		
	TCCR2A = (1 << WGM21); // CTC mode with OCRA
	TCCR2B = (1 << CS20) | (1 << CS22); // presc = 128 => 2 ms <=> 250 cnt; 500 us <=> 62.5
	OCR2A = PER_POLL_GP;
		
	TIMSK0 = (1 << OCIE0A);
	TIMSK2 = (1 << OCIE2A);
}

ISR(TIMER0_COMPA_vect)
{
	TIMSK2 &= ~(1 << OCIE2A); // "cli" for avoid nested interrupts
		sei();
		
		if(cnt_idle < delay_idle) cnt_idle++;
		else
		{
			TIMSK0 &= ~(1 << OCIE0A);
			flag_idle = 1;
		}
	TIMSK2 |= (1 << OCIE2A); // "sei"
}

ISR(TIMER2_COMPA_vect)
{
	TIMSK0 &= ~(1 << OCIE0A); // "cli"
		sei();
	
		if(state < 7) 
		{
			PORT_SEGA_AUX ^= (1 << SEGA_SEL);
		
			flag_ch_gp = 1;
			state++;
		}
		else if(state == 8)
		{ // after delay between "packets":
			OCR2A = PER_POLL_GP;
			flag_ch_gp = 1;
			state = 0;
		}
		else
		{ // after "packet":
			PORT_SEGA_AUX &= ~(1 << SEGA_SEL);
			OCR2A = DELAY_BTW_POLL;
		
			flag_report = 1;
			state = 8;
		}
	TIMSK0 |= (1 << OCIE0A); // "sei"
}

void main(void)
{
	uchar gp_state_buf[2][8];
	uchar *report_buf_ptr;

	hardwareInit();
	usbDeviceConnect();
	usbInit();
	
// full reset timers:
	TCNT0 = 0;
	TCNT2 = 0;
	
	// reset interrupt timers flags:
	TIFR0 |= (1 << OCF0A);
	TIFR2 |= (1 << OCF2A); 
	
	GTCCR |= (1 << PSRASY); // reset presc timers
	
	sei();
    while (1) 
    {
		usbPoll(); // ~ 9.63 us (all timings write in 16 MHz CPU freq)
		
		if(flag_idle) // send report immediately after "idle" time has passed:
		{
			if(usbInterruptIsReady())
			{
				usbSetInterrupt(report_buf, REPORT_SIZE);  // ~  us
				
				cnt_idle = 0;
				flag_idle = 0;
				
				PORT_LED ^= (1 << LED1);
				
			// full reset timer 0 then enable interrupt:
				TCNT0 = 0;
				TIFR0 |= (1 << OCF0A); 
				TIMSK0 |= (1 << OCIE0A);
			}
		}
		
		if(flag_report) // build report:
		{ 
			report_buf_ptr = updReportBuf(0, (uchar *)gp_state_buf); // var that defining the array is also a pointer to it
				report_buf[0] = *report_buf_ptr;
				report_buf[1] = *(report_buf_ptr + 1);
				report_buf[4] = *(report_buf_ptr + 2);
				
			report_buf_ptr = updReportBuf(8, (uchar *)gp_state_buf);
				report_buf[2] = *report_buf_ptr;
				report_buf[3] = *(report_buf_ptr + 1);
				report_buf[5] = *(report_buf_ptr + 2);
			
			flag_report = 0;
		}
		
		if(flag_ch_gp & (TCNT2 >= DELAY_BEF_POLL)) // upd gamepad status buffer:
		{
			gp_state_buf[0][state] = PIN_SEGA1 & SEGA_PIN_MASK;
			gp_state_buf[1][state] = PIN_SEGA2 & SEGA_PIN_MASK;
			flag_ch_gp = 0;
		}
    }
}