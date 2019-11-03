#include "defines.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "descriptor.h"

uchar report_buf[3] = {0x00, 0x00, 0x00};
uchar delay_idle = INIT_IDLE_TIME; // step - 4ms
uchar cnt_idle = 0;

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
			case USBDESCR_HID_REPORT:
				usbMsgPtr = (usbMsgPtr_t)desc_sega_hidreport;
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
				usbMsgPtr = (usbMsgPtr_t)report_buf;
				return REPORT_SIZE;
			case USBRQ_HID_GET_IDLE:
				usbMsgPtr = (usbMsgPtr_t)delay_idle;
				return 1;
			case USBRQ_HID_SET_IDLE: return USB_NO_MSG; // call "usbFunctionWrite" ("OUT" token)
		}
	}
	
	return 0; // ignore data from host ("OUT" token)
}

USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len)
{
	usbRequest_t *rq = (usbRequest_t*)data;
	
	if(rq -> wValue.bytes[1] != 0) delay_idle = rq -> wValue.bytes[1];
	else delay_idle = 0; // when the upper byte of "wValue" = 0, the duration is indefinite
	
	return 1;
}

/*
void setReport()
{
	
}
*/

ISR(TIMER0_COMPA_vect)
{
	sei();
	if(cnt_idle < delay_idle) cnt_idle++;
	else TIMSK0 &= ~(1 << OCIE0A);
}

void hardwareInit(void)
{
	DDR_SEGA = (1 << SEGA_SEL);
	PORT_SEGA = (1 << SEGA_LEFT_X) | (1 << SEGA_RIGHT_MODE) | (1 << SEGA_UP_Z) | (1 << SEGA_DOWN_Y) | 
				(1 << SEGA_A_B) | (1 << SEGA_ST_C) | (1 << SEGA_SEL);
	
	//DDR_PS = 
	//PORT_PS = 
	
	DDR_LED = (1 << LED0) | (1 << LED1); 
	
	TCCR0A = (1 << WGM01); // CTC mode with OCRA
	TCCR0B = (1 << CS02); // presc = 256 => 4 ms <=> 250 cnt
	OCR0A = 250;
	
	TIMSK0 = (1 << OCIE0A);
}

void main(void)
{
	
	
	hardwareInit();
	usbDeviceConnect();
	usbInit();
	
	sei();
    while (1) 
    {
		usbPoll();
		
		if(cnt_idle >= delay_idle)
		{
			if(usbInterruptIsReady())
			{
				usbSetInterrupt(report_buf, REPORT_SIZE);
				TIMSK0 = (1 << OCIE0A);
				cnt_idle = 0;
				
				PORT_LED ^= (1 << LED1);
			}
		}
		
		PORT_LED ^= (1 << LED0);
    }
}

