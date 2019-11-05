#include "defines.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "descriptor.h"

uchar report_buf[3] = {0x00, 0x00, 0x00};
	
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

uchar flag_ch_gp = 1; // flag shows that required save buttons state
uchar flag_report = 0;

USB_PUBLIC uchar usbFunctionDescriptor(usbRequest_t * rq)
{
	if (rq->bRequest == USBRQ_GET_DESCRIPTOR)
	{
		switch(rq -> wValue.bytes[1])
		{
			case USBDESCR_DEVICE:
				usbMsgPtr = (int)desc_dev;
				return sizeof(desc_dev);
			case USBDESCR_CONFIG:
				usbMsgPtr = (int)desc_conf;
				return sizeof(desc_conf);
			case USBDESCR_STRING:
				if(rq -> wValue.bytes[0] == 2) // device name
				{
					usbMsgPtr = (int)desc_prod_str;
					return sizeof(desc_prod_str);
				}
			case USBDESCR_HID_REPORT:
				usbMsgPtr = (int)desc_sega_hidreport;
				return sizeof(desc_sega_hidreport);
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
				usbMsgPtr = (int)report_buf;
				return REPORT_SIZE;
			case USBRQ_HID_GET_IDLE:
				if (rq -> wValue.bytes[0] > 0)
				{
					usbMsgPtr = (int)&delay_idle;
					return 1;
				}
				break;
			//case USBRQ_HID_SET_IDLE: return USB_NO_MSG; // call "usbFunctionWrite" ("OUT" token)
			case USBRQ_HID_SET_IDLE:
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

ISR(TIMER0_COMPA_vect)
{
	sei();
	
	if(cnt_idle < delay_idle) cnt_idle++;
	else TIMSK0 &= ~(1 << OCIE0A);
}

ISR(TIMER2_COMPA_vect)
{
	sei();
	
	if(state < 8) 
	{
		PORT_SEGA ^= (1 << SEGA_SEL);
		
		flag_ch_gp = 1;
		state++;
	}
	else
	{
		OCR2A = DELAY_BTW_POLL;
		
		flag_report = 1;
		state = 0;
	}
}

void main(void)
{
	uchar temp;
	uchar gp_state_buf[8];
	
	DDR_LED = (1 << LED0) | (1 << LED1);
	
// gamepads:
	DDR_SEGA = (1 << SEGA_SEL);
	PORT_SEGA = (1 << SEGA_LF_X) | (1 << SEGA_RG_MD) | (1 << SEGA_UP_Z) | (1 << SEGA_DW_Y) |
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
	
	TIMSK0 = (1 << OCIE0A) | (1 << OCIE2A);
	
		usbDeviceConnect();
		usbInit();
	
	TIFR0 |= (1 << OCF0A);
	TIFR2 |= (1 << OCF2A); // reset interrupt timers flags
	GTCCR |= (1 << PSRASY); // reset presc timers
	
	sei();
    while (1) 
    {
		usbPoll(); // ~ 9.63 us (all timings write in 16 MHz CPU freq)
		
		if((cnt_idle >= delay_idle) & (flag_report))
		{
			if(usbInterruptIsReady())
			{
				temp = (~gp_state_buf[2]) & ((1 << SEGA_A_B) | (1 << SEGA_ST_C));	// 0b00110000
				report_buf[2] = temp << 2;
				
				temp = (~gp_state_buf[3]) & ((1 << SEGA_A_B) | (1 << SEGA_ST_C));	// 0b00110000
				report_buf[2] |= temp;
				
				temp = (~gp_state_buf[5]) & ((1 << SEGA_UP_Z) | (1 << SEGA_DW_Y) | 
											 (1 << SEGA_LF_X) | (1 << SEGA_RG_MD));	// 0b00001111
				report_buf[2] |= temp;
				
				temp = (~gp_state_buf[3]) & ((1 << SEGA_UP_Z) | (1 << SEGA_DW_Y));	// 0b00000011
				if(temp == (1 << SEGA_UP_Z)) report_buf[1] = 0xFF;
				else if(temp == (1 << SEGA_DW_Y)) report_buf[1] = 0x00;
				else report_buf[1] = 0x7F;
				
				temp = (~gp_state_buf[3]) & ((1 << SEGA_LF_X) | (1 << SEGA_RG_MD));	// 0b00001100
				if(temp == (1 << SEGA_RG_MD)) report_buf[0] = 0xFF;
				else if(temp == (1 << SEGA_LF_X)) report_buf[0] = 0x00;
				else report_buf[0] = 0x7F;
				
				usbSetInterrupt(report_buf, REPORT_SIZE);  // ~ 22.44 us
				TIMSK0 = (1 << OCIE0A) | (1 << OCIE2A);
				
				cnt_idle = 0;
				flag_report = 0;
				
				PORT_LED ^= (1 << LED1);
			}
		}
		
		if(flag_ch_gp & (TCNT2 >= DELAY_BEF_POLL))
		{
			gp_state_buf[state] = PIN_SEGA & 0b00111111; // last 2 bits - TOSC 1,2
			flag_ch_gp = 0;
		}
    }
}

