
#ifndef _CDC_H_
#define _CDC_H_
      
	/* Includes: */
		#include <avr/io.h>
		#include <avr/pgmspace.h>

		#include "usb_descriptors.h"
		#include <USB.h>                // USB Functionality


	/* Macros: */
		#define GET_LINE_CODING              0x21
		#define SET_LINE_CODING              0x20
		#define SET_CONTROL_LINE_STATE       0x22

	/* Macros: */
		/** CDC Class specific request to get the current virtual serial port configuration settings. */
		#define REQ_GetLineEncoding          0x21

		/** CDC Class specific request to set the current virtual serial port configuration settings. */
		#define REQ_SetLineEncoding          0x20

		/** CDC Class specific request to set the current virtual serial port handshake line states. */
		#define REQ_SetControlLineState      0x22
		



	/* Event Handlers: */
      HANDLES_EVENT(USB_Reset);
		HANDLES_EVENT(USB_Connect);
		HANDLES_EVENT(USB_Disconnect);
		HANDLES_EVENT(USB_ConfigurationChanged);
		HANDLES_EVENT(USB_UnhandledControlPacket);
		
	/* Type Defines: */
		typedef struct
		{
			uint32_t BaudRateBPS;
			uint8_t  CharFormat;
			uint8_t  ParityType;
			uint8_t  DataBits;
		} CDC_Line_Coding_t;
		
	/* Enums: */
		enum
		{
			OneStopBit          = 0,
			OneAndAHalfStopBits = 1,
			TwoStopBits         = 2,
		} CDC_Line_Coding_Format;
		
		enum
		{
			Parity_None         = 0,
			Parity_Odd          = 1,
			Parity_Even         = 2,
			Parity_Mark         = 3,
			Parity_Space        = 4,
		} CDC_Line_Codeing_Parity;


   /* Function prototypes: */
      void usb_init(void);
      void USB_USBTask(void);
      bool usb_con(void);
      
#endif
