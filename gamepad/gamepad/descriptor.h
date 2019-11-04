#include <avr/pgmspace.h>

#define UNUSED 0x00
#define TOTAL_LEN_DESCR (18 + 7 + 9)

const int PROGMEM desc_prod_str[] = {
	USB_STRING_DESCRIPTOR_HEADER(10),
	's', 's', '_', 'g', 'a', 'm', 'e', 'p', 'a', 'd'
};

const uchar PROGMEM desc_dev[] = {
	0x12,					/* length of descriptor in bytes */
	USBDESCR_DEVICE,		/* descriptor type */
	0x10, 0x01,				/* USB version supported */
	USB_CFG_DEVICE_CLASS,
	USB_CFG_DEVICE_SUBCLASS,
	UNUSED,					/* protocol: defined at interface level */
	0x08,					/* max packet size (for EP0) */
	USB_CFG_VENDOR_ID,
	USB_CFG_DEVICE_ID,
	USB_CFG_DEVICE_VERSION,
	0x01,					/* manufacturer string index */
	0x02,					/* product string index */
	UNUSED, //3,			/* ??? serial number string index */				
	0x01,					/* number of configurations */
};

const char PROGMEM usbDescriptorConfiguration[] = { 0 };
const uchar PROGMEM desc_conf[] = {
	
				/****************** Configuration descriptor ******************/
	
	0x09,          
    USBDESCR_CONFIG,    
    TOTAL_LEN_DESCR, 0x00,  /* total length of data returned (including inlined descriptors) */
    0x01,					/* number of interfaces in this configuration */
    0x01,					/* index of this configuration */
    UNUSED,					/* configuration name string index */
    USBATTR_BUSPOWER,
    USB_CFG_MAX_BUS_POWER / 2,
	
				/************** Interface descriptor follows inline **************/
    
	// Standard interface descriptor:
	0x09,
    USBDESCR_INTERFACE,
    0x00,					/* index of this interface */
    UNUSED,					/* alternate setting for this interface */
    0x01,					/* amount endpoints that this interface is use, excl 0 */
    USB_CFG_INTERFACE_CLASS,
    USB_CFG_INTERFACE_SUBCLASS,
    USB_CFG_INTERFACE_PROTOCOL,
    UNUSED,					/* index of string descriptor for this interface */
	
	// Class-specific interface descriptor:
    0x09,
    USBDESCR_HID,
    0x01, 0x01,				/* BCD representation of HID version */
    0x00,					/* target country code (if needed) */
    0x01,					/* number of HID Report (or other HID class) Descriptor infos to follow */
    0x22,					/* descriptor type: report */
    USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH, 0x00, // ???
	
				/***************** Bulk IN endpoint descriptors *****************/
	
    0x07,
    USBDESCR_ENDPOINT,
    0x81,					/* endpoint address: IN endpoint number 1 */
    0x03,					/* bmAttributes: 0: Control, 1: Isochronous 2: Bulk, 3: Interrupt endpoint */
    0x08, 0x00,				/* max packet size */
    USB_CFG_INTR_POLL_INTERVAL,
};

const uchar PROGMEM desc_sega_hidreport[] = {
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,                    // USAGE (Game Pad)
	
	0xa1, 0x01,                    //	COLLECTION (Application)
	
	0x09, 0x01,                    //		USAGE (Pointer)
	0xa1, 0x00,                    //		COLLECTION (Physical)
	0x09, 0x30,                    //			USAGE (X)
	0x09, 0x31,                    //			USAGE (Y)
	0x15, 0x00,                    //			LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //			LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //			REPORT_SIZE (8)
	0x95, 0x02,                    //			REPORT_COUNT (2)
	0x81, 0x02,                    //			INPUT (Data,Var,Abs)
	0xc0,                          //		END_COLLECTION
	
	0x05, 0x09,                    //		USAGE_PAGE (Button)
	0x19, 0x01,                    //		USAGE_MINIMUM (Button 1)
	0x29, 0x08,                    //		USAGE_MAXIMUM (Button 8)
	0x15, 0x00,                    //		LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //		LOGICAL_MAXIMUM (1)
	0x75, 0x01,                    //		REPORT_SIZE (1)
	0x95, 0x08,                    //		REPORT_COUNT (8)
	0x81, 0x02,                    //		INPUT (Data,Var,Abs)
	
	0xc0                           //	END_COLLECTION
};

const char PROGMEM usbDescriptorHidReport[43] = {
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,                    // USAGE (Game Pad)
	
	0xa1, 0x01,                    //	COLLECTION (Application)
	
	0x09, 0x01,                    //		USAGE (Pointer)
	0xa1, 0x00,                    //		COLLECTION (Physical)
	0x09, 0x30,                    //			USAGE (X)
	0x09, 0x31,                    //			USAGE (Y)
	0x15, 0x00,                    //			LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //			LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //			REPORT_SIZE (8)
	0x95, 0x02,                    //			REPORT_COUNT (2)
	0x81, 0x02,                    //			INPUT (Data,Var,Abs)
	0xc0,                          //		END_COLLECTION
	
	0x05, 0x09,                    //		USAGE_PAGE (Button)
	0x19, 0x01,                    //		USAGE_MINIMUM (Button 1)
	0x29, 0x08,                    //		USAGE_MAXIMUM (Button 8)
	0x15, 0x00,                    //		LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //		LOGICAL_MAXIMUM (1)
	0x75, 0x01,                    //		REPORT_SIZE (1)
	0x95, 0x08,                    //		REPORT_COUNT (8)
	0x81, 0x02,                    //		INPUT (Data,Var,Abs)
	
	0xc0                           //	END_COLLECTION
};