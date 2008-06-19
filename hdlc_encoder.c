/*
 * $Id: hdlc_encoder.c,v 1.16 2008-06-19 18:40:49 la7eca Exp $
 * AFSK Modulator/Transmitter
 */
 
#include "defines.h"
#include <avr/io.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include "fbuf.h"
#include "hdlc.h"
#include "kernel.h"
#include "timer.h"
#include "stream.h"
#include "config.h"
#include <util/crc16.h>
#include "transceiver.h"
		          

// Buffers
FBQ encoder_queue; 
static FBUF buffer;                                 
static stream_t* outstream;

static Semaphore test; 
static bool test_active;
static uint8_t testbyte;

#define BUFFER_EMPTY (fbuf_eof(&buffer))            


/* FIXME: Those should be parameters stored in EEPROM ! */
#define PERSISTENCE 100
#define SLOTTIME    50

static void hdlc_txencoder(void);
static void hdlc_testsignal(void);
static void hdlc_encode_frames(void);
static void hdlc_encode_byte(uint8_t, bool);
static void wait_channel_ready(void);

   
fbq_t* hdlc_init_encoder(stream_t* os)
{
   outstream = os;
   FBQ_INIT( encoder_queue, HDLC_ENCODER_QUEUE_SIZE ); 
   THREAD_START( hdlc_txencoder, 120 );
   
   sem_init(&test, 0);
   THREAD_START( hdlc_testsignal, 100);
   return &encoder_queue;
}		


/*******************************************************
 * Code for generating a test signal
 *******************************************************/

void hdlc_test_on(uint8_t b)
{ 
    testbyte = b;
    test_active = true;
    sem_up(&test);
}

void hdlc_test_off()
    { test_active=false; }

    
static void hdlc_testsignal()
{
    while (true)
    {
        sem_down(&test);
        while(test_active) 
           putch(outstream, testbyte);
    }
}




/*******************************************************************************
 * Transmit a frame.
 *
 * This function gets a frame from buffer-queue, and starts the transmitter
 * as soon as the channel is free. It should typically be called repeatedly
 * from a loop in a thread.  
 *******************************************************************************/
 
static void hdlc_txencoder()
{ 
   sleep(200);

   while (true)  
   {
      /* Get frame from buffer-queue when available. 
       * This is a blocking call.
       */ 
      buffer = fbq_get(&encoder_queue); 
         
      /* Wait until channel is free 
       * P-persistence algorithm 
       */
      for (;;) {
        wait_channel_ready(); 
        int r  = rand(); 
        if (r > PERSISTENCE * 255)
            sleep(SLOTTIME); 
        else
            break;
      }
      hdlc_encode_frames();
   }
}
   

static void wait_channel_ready()
{
    double sqlevel; 
    GET_PARAM(TRX_SQUELCH, &sqlevel);
    adf7021_wait_enabled();
    while (adf7021_read_rssi() > sqlevel) 
       sleep(5);
}



/*******************************************************************************
 * HDLC encode and transmit one or more frames (one single transmission)
 * It is responsible for computing checksum, bit stuffing and for adding 
 * flags at start and end of frames.
 *******************************************************************************/
 
static void hdlc_encode_frames()
{
     uint16_t crc = 0xffff;
     uint8_t txbyte, i;
     uint8_t txdelay = GET_BYTE_PARAM(TXDELAY);
     uint8_t txtail  = GET_BYTE_PARAM(TXTAIL);
     
     fbuf_reset(&buffer);
     for (i=0; i<txdelay; i++)
         hdlc_encode_byte(HDLC_FLAG, true);

     while(!BUFFER_EMPTY)
     {
         txbyte = fbuf_getChar(&buffer);        
         crc = _crc_ccitt_update (crc, txbyte);
         hdlc_encode_byte(txbyte, false);
     }
     fbuf_release(&buffer);
     hdlc_encode_byte(crc^0xFF, false);       // Send FCS, LSB first
     hdlc_encode_byte((crc>>8)^0xFF, false);  // MSB
       
     for (i=0; i<txtail; i++)
         hdlc_encode_byte(HDLC_FLAG, true);
}



/*******************************************************************************
 * HDLC encode and transmit a single byte. Includes bit stuffing if not flag
 *******************************************************************************/

static void hdlc_encode_byte(uint8_t txbyte, bool flag)
{    
     static uint8_t outbits = 0;
     static uint8_t outbyte;
     
     for (uint8_t i=1; i<8+1; i++)
     { 
        if (!flag && (outbyte & 0x7c) == 0x7c) 
            i--;

        else {
            outbyte |= ((txbyte & 0x01) << 7);
            txbyte >>= 1;  
        }
     
        if (++outbits == 8) {
            putch(outstream, outbyte);
            outbits = 0;
        }
        outbyte >>= 1;      
     }   
}

