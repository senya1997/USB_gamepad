#include "defines.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "descriptors.h"

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
	return 1;
}

void hardwareInit(void)
{
	DDR_LED = (1 << LED0) | (1 << LED1); 
}

void main(void)
{
	
	hardwareInit();
	usbDeviceConnect();
	usbInit();
	
	asm("SEI");
    while (1) 
    {
		usbPoll();
		
		_delay_ms(300);
		PORT_LED ^= (1 << LED0);
		
		_delay_ms(300);
		PORT_LED ^= (1 << LED1);
    }
}

