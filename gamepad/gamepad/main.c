#include "defines.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "descriptor.h"

#ifdef DEBUG
	#warning "DEBUG is enabled"
#endif

uchar report_buf[REPORT_SIZE] = {0x00, 0x00, 0x7F, 0x7F, 0x7F, 0x7F};

// for USB:
	uchar delay_idle = INIT_IDLE_TIME; // step - 4ms
	uchar cnt_idle = 0;
	uchar flag_idle = 0; // shows that USB idle time is over and we can send report

//uchar flag_report = 0; // shows that information from gamepad is ready to form like in descriptor
	
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

uchar initHW() // return chosen gamepad code: 0 - SEGA, 1 - PS
{
	DDR_LED |= (1 << LED0) | (1 << LED1);

	DDR_CTRL &= ~(1 << CTRL);
	PORT_CTRL |= (1 << CTRL);

// idle timer:
	TCCR0A = (1 << WGM01); // CTC mode with OCRA
	TCCR0B = (1 << CS02); // presc = 256 => 4 ms <=> 250 cnt
	OCR0A = STEP_IDLE_CONF;
	
	TIMSK0 = (1 << OCIE0A);
	
// choose controller:
	if((PIN_CTRL & (1 << CTRL)) == (1 << CTRL))
		return 1; // PS
	else
		return 0; // SEGA
}

void initSPI()
{
// master SPI pins output:
	DDRB |= (1 << PS_CS) | (1 << PS_MOSI) | (1 << PS_CLK);
	PORTB |= (1 << PS_CS) | (1 << PS_MOSI) | (1 << PS_CLK);

// master SPI input:
	DDRB &= ~(1 << PS_MISO);
	PORTB |= (1 << PS_MISO);
	
// SPI config:
	SPCR |= (1 << SPE) | (1 << MSTR) | (1 << DORD); // enable SPI, master mode, LSB first mode
	SPCR |= (1 << CPOL) | (1 << CPHA); // issue on fall, read on front
	
// set presc for SCK: F_CPU/128 (16 MHz / 128 = 125 kHz)
	SPCR |= (1 << SPR0) | (1 << SPR1);
	SPSR &= ~(1 << SPI2X);
}

ISR(TIMER0_COMPA_vect)
{
	//TIMSK2 &= ~(1 << OCIE2A); // "cli" for avoid nested interrupts
	sei();
	
	if(cnt_idle < delay_idle) cnt_idle++;
	else
	{
		TIMSK0 &= ~(1 << OCIE0A);
		flag_idle = 1;
	}
	//TIMSK2 |= (1 << OCIE2A); // "sei"
}

uchar readSPI()
{
	uchar spsr_buf; // SPI status reg buf var
	int froze_cnt; // for avoid froze when wait SPI transmit flag
	
	PORTB &= ~(1 << PS_CS); // set low CS before transfer
	
	for(uchar i = 0; i < 9; i++)
	{
		spsr_buf = SPDR; // read SPI data reg to clear all flags and prepare SPI transmit
		
	// master SPI interface bytes:
		if(i == 0)
			SPDR = 0x01;
		else if(i == 1)
			SPDR = 0x42;
		else
			SPDR = 0xFF;
			
	// wait end of SPI transfer:
		froze_cnt = 0;
		spsr_buf = SPSR & (1 << SPIF); // read SPI status reg
		
		while(spsr_buf != (1 << SPIF))
		{
			if(froze_cnt >= SPI_FROZE)
			{
				PORTB |= (1 << PS_CS); // set high CS
				return 0; // failure: SPI is frozen
			}
			else
				froze_cnt++;
			
			spsr_buf = SPSR & (1 << SPIF);
		}
		
	// get gamepad state:
		if((i == 3) | (i == 4))
			report_buf[i - 3] = ~SPDR; // buttons must be inverted
		else if(i > 4)
			report_buf[i - 3] = SPDR; // analogs
	}
	
	PORTB |= (1 << PS_CS); // set high CS
	return 1; // successful: SPI packet complete
}

int main()
{
	uchar flag_report_rdy = 0;
	uchar flag_ctrl = 0;
	
	flag_ctrl = initHW();
	
	if(flag_ctrl)
		initSPI();
	//else
		//initSEGA();
	
	#ifndef DEBUG
		usbDeviceConnect();
		usbInit();
	#endif
	
// full reset timer:
	TCNT0 = 0;
	TCNT2 = 0;
	
	TIFR0 |= (1 << OCF0A);
	TIFR2 |= (1 << OCF2A);
	
	sei();
    while(1) 
    {
		#ifndef DEBUG
			usbPoll(); // ~ 9.63 us (all timings write in 16 MHz CPU freq)
		#endif
		
		if(flag_idle & flag_report_rdy) // send report immediately after "idle" time has passed:
		{
			if(usbInterruptIsReady())
			{
				#ifndef DEBUG
					usbSetInterrupt(report_buf, REPORT_SIZE);  // ~ 31.5 us
				#endif
				
				cnt_idle = 0;
				
				flag_idle = 0;
				flag_report_rdy = 0;
				
			// full reset timer 0 then enable interrupt:
				TCNT0 = 0;
				TIFR0 |= (1 << OCF0A);
				TIMSK0 |= (1 << OCIE0A);
				
				PORT_LED ^= (1 << LED1);
			}
		}
		
		if(flag_ctrl) // build report:
		{
			flag_report_rdy = readSPI();
			
			PORT_LED ^= (1 << LED0);
		}
		//else
		//{
		//	
		//	PORT_LED ^= (1 << LED0);
		//}
    }
}