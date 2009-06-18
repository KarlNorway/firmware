/*
 * $Id: afsk_tx.c,v 1.24 2009-04-04 20:43:36 la7eca Exp $
 * AFSK Modulator/Transmitter
 */
 
#include "defines.h"
#include <avr/io.h>
#include <inttypes.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "afsk.h"
#include "kernel/kernel.h"
#include "kernel/stream.h"
#include "transceiver.h"
#include "config.h"



/* Internal config */

#define _PRESCALER3  1
#define _PRESCALER3_SETTING 0x01
#define _TXI_MARK   ((SCALED_F_CPU / _PRESCALER3 / (AFSK_TXTONE_MARK) ) /16 - 1 )
#define _TXI_SPACE  ((SCALED_F_CPU / _PRESCALER3 / (AFSK_TXTONE_SPACE) ) /16 - 1 )


stream_t afsk_tx_stream;
static uint16_t timertop, start_tone;

bool     transmit;     /* True when transmitter(modulator) is active. */        
extern BCond mon_ok;   /* since activity on USB/serial port disturbs modulation,
                          there must be synchronisation to prevent those two things
                          from running at the same time */   
   
   
stream_t* afsk_init_encoder(void) 
{
    STREAM_INIT(afsk_tx_stream, AFSK_ENCODER_BUFFER_SIZE);   

    DAC_DDR |= DAC_MASK;
    DAC_PORT = 0;
    /* Clear TXDATA pin, even when using DAC method */
    make_output(TXDATA);
    clear_port(TXDATA);

    start_tone = _TXI_MARK;
    return &afsk_tx_stream;
}               


void afsk_high_tone(bool t)
{
   start_tone = (t ? _TXI_SPACE : _TXI_MARK);
}



/*******************************************************************************
 * Turn on transmitter (tone generator, ptt and led.)
 *******************************************************************************/
 
void afsk_ptt_on()
{        
    TCCR3B = _PRESCALER3_SETTING     /* Pre-scaler for timer3 */             
             | (1<<WGM32) ;          /* CTC mode */   
//    TCCR3A |= (1<<COM3A0);         /* Toggle OC3A on compare match. */
    OCR3A  = timertop = start_tone;
    TIMSK3 = 1<<OCIE3A;              /* Interrupt on compare match */ 
    adf7021_enable_tx();
    
    set_port(LED2);
    transmit = true; 
    bcond_clear(&mon_ok);
}




/*******************************************************************************
 * Turn off transmitter (tone generator, ptt and led.)
 *******************************************************************************/

void afsk_ptt_off(void)
{
    bcond_set(&mon_ok);
    transmit = false;    
    clear_port(LED2);                /* LED / PTT */
    
    TIMSK3 = 0x00;
    TCCR3A &= ~(1<<COM3A0);          /* Toggle OC3A on compare match: OFF. */
    DAC_PORT = 0;
    adf7021_disable_tx();
    start_tone = _TXI_MARK;
}



/**************************************************************************
 * Get next bit from stream
 * Note: see also get_bit() in hdlc_decoder.c 
 * Note: The next_byte must ALWAYS be called before get_bit is called
 * to get new bytes from stream. 
 *************************************************************************/
 
static uint8_t bits;
static uint8_t bit_count = 0;

static uint8_t get_bit(void)
{
  if (bit_count == 0) 
     return 0;
  uint8_t bit = bits & 0x01;
  bits >>= 1;
  bit_count--;
  return bit;
}


static void next_byte(void)
{
  if (bit_count == 0) 
  {
    /* Turn off TX if buffer is empty (have reached end of frame) */
    if (stream_empty(&afsk_tx_stream)) { 
        afsk_ptt_off();  
        return;
    }
    bits = stream_get_nb (&afsk_tx_stream); 
    bit_count = 8;    
  } 
}




/*******************************************************************************
 * If transmit flag is on, this function should be called periodically, 
 * at same rate as wanted baud rate.
 *
 * It is responsible for transmitting frames by toggling the frequency of
 * the tone generated by the interrupt handler below. 
 *******************************************************************************/ 

void afsk_txBitClock(void)
{
    if (!transmit) {
        if (stream_empty(&afsk_tx_stream))
           return;
        else {
           next_byte();
           afsk_ptt_on();
        }
    }       
    if ( ! get_bit() ) 
        /* Toggle TX frequency */ 
        timertop = ((timertop >= _TXI_MARK) ? _TXI_SPACE : _TXI_MARK); 
      
    /* Update byte from stream if necessary. We do this 
     * separately, after the get_bit to make the timing more precise 
     */  
    next_byte();   
}




/*********************************************************************************
 * Alternative method of generating a sine signal at 1200 and 2200 Hz (4 bit DAC). 
 * This is output to the 4 lowest bits of port C. 
 *********************************************************************************/

ISR(TIMER3_COMPA_vect)
{
#if defined USBKEY_TEST
   static uint8_t sine[16] = {8, 10, 13, 14, 15, 14, 13, 10, 7, 5, 2, 1, 0, 1, 2, 5};
#else
   static uint8_t sine[16] = {0x80,0xa0,0xd0,0xe0,0xf0,0xe0,0xd0,0xa0,0x70,0x50,0x20,0x10,0,0x10,0x20,0x50};
#endif
      
   static uint8_t index = 0;          // Index for the D-to-A sequence

   DAC_PORT = sine[index];            // Output next D-to-A sinewave value
   index++;                           // Increment index
   index &= 15;                       // And wrap to a max of 15   
   OCR3A = timertop;                  // reload counter top based on freq.
   if (TCNT3 > timertop) {
       TCNT3 = 0;
       index++;
   }
}               
 

