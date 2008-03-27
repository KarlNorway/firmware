#include "usb.h"
#include "kernel.h"
#include "stream.h"
#include "defines.h"

#define CDC_BUF_SIZE 16

/* MyUSB Bug fix */
#define ENDPOINT_INT_IN     UEIENX, (1 << TXINE) , UEINTX, (1 << TXINI)
#define ENDPOINT_INT_OUT    UEIENX, (1 << RXOUTE), UEINTX, (1 << RXOUTI)



CDC_Line_Coding_t LineCoding = { BaudRateBPS: 9600,
                                 CharFormat:  OneStopBit,
                                 ParityType:  Parity_None,
                                 DataBits:    8            };


Semaphore cdc_run;    
Stream cdc_instr; 
Stream cdc_outstr;



EVENT_HANDLER(USB_Connect)
{
}


EVENT_HANDLER(USB_Disconnect)
{
}



EVENT_HANDLER(USB_CreateEndpoints)
{
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

	/* Double green to indicate USB connected and ready */
	set_bit(LED3);
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

	Endpoint_Ignore_Word();
   
	/* Process CDC specific control requests */
	switch (Request)
	{
		case GET_LINE_CODING:
			if (RequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSetupReceived();

				for (uint8_t i = 0; i < sizeof(LineCoding); i++)
				  Endpoint_Write_Byte(*(LineCodingData++));	
				
				Endpoint_Setup_In_Clear();
				while (!(Endpoint_Setup_In_IsReady()));
				
				while (!(Endpoint_Setup_Out_IsReceived()));
				Endpoint_Setup_Out_Clear();
			}
			
			break;
		case SET_LINE_CODING:
			if (RequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSetupReceived();

				while (!(Endpoint_Setup_Out_IsReceived()));

				for (uint8_t i = 0; i < sizeof(LineCoding); i++)
				  *(LineCodingData++) = Endpoint_Read_Byte();

				Endpoint_Setup_Out_Clear();

				Endpoint_Setup_In_Clear();
				while (!(Endpoint_Setup_In_IsReady()));
			}
	
			break;
		case SET_CONTROL_LINE_STATE:
			if (RequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSetupReceived();
				
				Endpoint_Setup_In_Clear();
				while (!(Endpoint_Setup_In_IsReady()));
			}
	
			break;
	}
}





void usb_kickout(void)
{
   enter_critical();
   Endpoint_SelectEndpoint(CDC_TX_EPNUM);	      
   while ( !_stream_empty(&cdc_outstr) && Endpoint_ReadWriteAllowed())   
       Endpoint_Write_Byte( _stream_get(&cdc_outstr, true) );  

   Endpoint_FIFOCON_Clear();
   leave_critical();    
}






ISR(ENDPOINT_PIPE_vect)
{   
   enter_critical();           
	if (Endpoint_HasEndpointInterrupted(CDC_RX_EPNUM))
	{
		Endpoint_ClearEndpointInterrupt(CDC_RX_EPNUM);
		Endpoint_SelectEndpoint(CDC_RX_EPNUM);	
   
		if ( USB_INT_HasOccurred(ENDPOINT_INT_OUT) ) {
         USB_INT_Clear(ENDPOINT_INT_OUT);
         if (Endpoint_ReadWriteAllowed()){
             while (Endpoint_BytesInEndpoint() && !_stream_full(&cdc_instr) )
                 _stream_put(&cdc_instr, Endpoint_Read_Byte(), true);
             Endpoint_FIFOCON_Clear();
         }   
      }
   }
   if (Endpoint_HasEndpointInterrupted(CDC_TX_EPNUM))
	{	
		Endpoint_ClearEndpointInterrupt(CDC_TX_EPNUM);
		Endpoint_SelectEndpoint(CDC_TX_EPNUM);	
		if ( USB_INT_HasOccurred(ENDPOINT_INT_IN) ) {  
           USB_INT_Clear(ENDPOINT_INT_IN);
           usb_kickout(); 
      }
   } 
   leave_critical();  
}

  
    

void usb_init()
{ 
   /* Initialize USB Subsystem */
	USB_Init(USB_MODE_DEVICE, USB_DEV_OPT_HIGHSPEED | USB_OPT_REG_ENABLED);
   sem_init(&cdc_run, 0);
  
   STREAM_INIT( cdc_instr, CDC_BUF_SIZE);
   STREAM_INIT( cdc_outstr, CDC_BUF_SIZE);
   cdc_outstr.kick = usb_kickout;
}

      
