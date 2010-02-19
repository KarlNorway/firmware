
#include "usb.h"
#include "kernel/kernel.h"
#include "kernel/stream.h"
#include "kernel/timer.h"
#include "defines.h"
#include "ui.h"
#include <avr/sleep.h>


#define CDC_BUF_SIZE 64



CDC_Line_Coding_t LineCoding = { BaudRateBPS: 9600,
                                 CharFormat:  OneStopBit,
                                 ParityType:  Parity_None,
                                 DataBits:    8            };


Semaphore cdc_run;    
Stream cdc_instr; 
Stream cdc_outstr;
Timer usb_startup; 


bool usb_con()
   { return USB_VBUS_GetStatus(); }



EVENT_HANDLER(USB_Connect)
{
  set_sleep_mode(SLEEP_MODE_IDLE); 
  timer_set(&usb_startup, 300);
}

void usb_disable()
{
   Endpoint_SelectEndpoint(CDC_RX_EPNUM);
   Endpoint_DisableEndpoint();
   USB_INT_Disable( ENDPOINT_INT_OUT ); 
   Endpoint_SelectEndpoint(CDC_TX_EPNUM);
   Endpoint_DisableEndpoint();   	 
   USB_INT_Disable( ENDPOINT_INT_IN ); 
}


/* OBS: Det kan se ut som at vi mister dette av og til. */
EVENT_HANDLER(USB_Disconnect)
{
   usb_disable();
   led_usb_off();
}





EVENT_HANDLER(USB_Reset)
{
	/* Select the control endpoint */
	Endpoint_SelectEndpoint(ENDPOINT_CONTROLEP);

	/* Enable the endpoint SETUP interrupt ISR for the control endpoint */
	USB_INT_Enable(ENDPOINT_INT_SETUP);
}



EVENT_HANDLER(USB_ConfigurationChanged)
{  CONTAINS_CRITICAL;
	/* Setup CDC Notification, Rx and Tx Endpoints */
	Endpoint_ConfigureEndpoint(CDC_NOTIFICATION_EPNUM, EP_TYPE_INTERRUPT,
		                        ENDPOINT_DIR_IN, CDC_NOTIFICATION_EPSIZE,
	                           ENDPOINT_BANK_SINGLE);

	Endpoint_ConfigureEndpoint(CDC_TX_EPNUM, EP_TYPE_BULK,
		                        ENDPOINT_DIR_IN, CDC_TXRX_EPSIZE,
	                           ENDPOINT_BANK_SINGLE);

	Endpoint_ConfigureEndpoint(CDC_RX_EPNUM, EP_TYPE_BULK,
		                        ENDPOINT_DIR_OUT, CDC_TXRX_EPSIZE,
	                           ENDPOINT_BANK_SINGLE);

	/* LED to indicate USB connected and ready */
	led_usb_on();
        enter_critical();
        Endpoint_SelectEndpoint(CDC_RX_EPNUM);
        Endpoint_EnableEndpoint();
        USB_INT_Enable( ENDPOINT_INT_OUT ); 
   
        Endpoint_SelectEndpoint(CDC_TX_EPNUM);
        Endpoint_EnableEndpoint();   	 
        USB_INT_Enable( ENDPOINT_INT_IN );    
        sem_up(&cdc_run);    
        leave_critical();    
}



EVENT_HANDLER(USB_UnhandledControlPacket)
{
	uint8_t* LineCodingData = (uint8_t*)&LineCoding;

	/* Process CDC specific control requests */
	switch (USB_ControlRequest.bRequest)
	{
		case REQ_GetLineEncoding:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{	
				/* Acknowedge the SETUP packet, ready for data transfer */
				Endpoint_ClearSETUP();

				/* Write the line coding data to the control endpoint */
				Endpoint_Write_Control_Stream_LE(LineCodingData, sizeof(CDC_Line_Coding_t));
				
				/* Finalize the stream transfer to send the last packet or clear the host abort */
				Endpoint_ClearOUT();
			}
			
			break;
		case REQ_SetLineEncoding:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				/* Acknowedge the SETUP packet, ready for data transfer */
				Endpoint_ClearSETUP();

				/* Read the line coding data in from the host into the global struct */
				Endpoint_Read_Control_Stream_LE(LineCodingData, sizeof(CDC_Line_Coding_t));

				/* Finalize the stream transfer to clear the last packet from the host */
				Endpoint_ClearIN();
			}
	
			break;
		case REQ_SetControlLineState:
			if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
#if 0
				/* NOTE: Here you can read in the line state mask from the host, to get the current state of the output handshake
				         lines. The mask is read in from the wValue parameter, and can be masked against the CONTROL_LINE_OUT_* masks
				         to determine the RTS and DTR line states using the following code:
				*/

				uint16_t wIndex = Endpoint_Read_Word_LE();
					
				// Do something with the given line states in wIndex
#endif
				
				/* Acknowedge the SETUP packet, ready for data transfer */
				Endpoint_ClearSETUP();
				
				/* Send an empty packet to acknowedge the command */
				Endpoint_ClearIN();
			}
	
			break;
	}
}




void usb_kickout(void)
{
   CONTAINS_CRITICAL;
   enter_critical();
   Endpoint_SelectEndpoint(CDC_TX_EPNUM);	     
   while ( !stream_empty(&cdc_outstr) && Endpoint_IsReadWriteAllowed())   
       Endpoint_Write_Byte( stream_get_nb(&cdc_outstr) );  
   Endpoint_ClearIN (); //Endpoint_ClearCurrentBank();
   leave_critical();    
}




ISR(ENDPOINT_PIPE_vect, ISR_BLOCK)
{ 
    if (Endpoint_HasEndpointInterrupted(ENDPOINT_CONTROLEP))
    {
	    Endpoint_ClearEndpointInterrupt(ENDPOINT_CONTROLEP);
            USB_USBTask();
  	    USB_INT_Clear(ENDPOINT_INT_SETUP);
    }     
    if (Endpoint_HasEndpointInterrupted(CDC_RX_EPNUM))
    {
            Endpoint_ClearEndpointInterrupt(CDC_RX_EPNUM);
            Endpoint_SelectEndpoint(CDC_RX_EPNUM);	
            if ( USB_INT_HasOccurred(ENDPOINT_INT_OUT) ) {
                USB_INT_Clear(ENDPOINT_INT_OUT);
                if (Endpoint_IsReadWriteAllowed()){
                    while (Endpoint_BytesInEndpoint() && !stream_full(&cdc_instr) ) 
                    stream_put_nb(&cdc_instr, Endpoint_Read_Byte());
                    Endpoint_ClearIN (); //Endpoint_ClearCurrentBank();
               }   
           }
   }
   else if (Endpoint_HasEndpointInterrupted(CDC_TX_EPNUM)) 
   {
	   Endpoint_ClearEndpointInterrupt(CDC_TX_EPNUM);
   	   Endpoint_SelectEndpoint(CDC_TX_EPNUM);	
	   if ( USB_INT_HasOccurred(ENDPOINT_INT_IN) ) {  
              USB_INT_Clear(ENDPOINT_INT_IN);
              usb_kickout(); 
           }
   }   
}

  
    

void usb_init()
{ 
   /* Initialize USB Subsystem */
   /* See makefile for mode constraints */
   USB_Init();
   sem_init(&cdc_run, 0);

   STREAM_INIT( cdc_instr, CDC_BUF_SIZE);
   STREAM_INIT( cdc_outstr, CDC_BUF_SIZE);
   cdc_outstr.kick = usb_kickout;
   /* Turn off pad regulator */
//   clear_bit(UHWCON, UVREGE);
}

      
