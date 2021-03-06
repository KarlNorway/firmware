/*
 * $Id: config.c,v 1.7 2009-01-21 22:26:53 la7eca Exp $
 *
 * Code for reading/writing paramerers in EEPROM, their default values in
 * program memory plus trace info in noinit part of RAM.
 */

#define __CONFIG_C__   /* IMPORTANT */
#include "config.h"
#include <stdio.h>



void show_trace(char* buf, uint8_t run, PGM_P pre, PGM_P post)
{
   uint8_t n=0, i;
   n+= sprintf_P(buf, pre);
   for (i=0; i<TRACE_LENGTH; i++)
   {
       n += sprintf_P(buf+n, PSTR("%3u"), GET_TRACE(run, i));
       if (i<TRACE_LENGTH-1)
          n+= sprintf_P(buf+n, PSTR(", "));
   }
   sprintf_P(buf+n, post);
}



void reset_param(void* ee_addr, uint8_t size)
{   
   while (!eeprom_is_ready())
      t_yield();
   eeprom_write_byte(ee_addr+size, 0xff);
}


void reset_all_param()
{   while (!eeprom_is_ready())
      t_yield();
    for (void* eeptr = &FIRST_PARAM;  eeptr <= &LAST_PARAM; eeptr += 2)
       eeprom_write_byte(eeptr, 0xff);
}


/************************************************************************
 * Write config parameter into EEPROM. 
 ************************************************************************/
  
void set_param(void* ee_addr, const void* ram_addr, uint8_t size)
{
   register uint8_t byte, checksum = 0x0f;
   
   while (!eeprom_is_ready())
      t_yield();
   for (int i=0; i<size; i++)
   {
       byte = *((uint8_t*) ram_addr++);
       checksum ^= byte;
       eeprom_write_byte(ee_addr++, byte);
   }
   eeprom_write_byte(ee_addr, checksum);
}
 
 
/************************************************************************
 * Get config parameter from EEPROM. 
 * If checksum does not match, use default value from flash memory 
 * instead and return 0. Value is copied into ram_addr.
 ************************************************************************/
   
int get_param(const void* ee_addr, void* ram_addr, uint8_t size, PGM_P default_val)
{
   register uint8_t byte, checksum = 0x0f, s_checksum;
   register void* dest = ram_addr;
   
   while (!eeprom_is_ready())
      t_yield();
   for (int i=0;i<size; i++)
   {
       byte = eeprom_read_byte(ee_addr++);
       checksum ^= byte;
       *((uint8_t*) dest++) = byte;
   }
   s_checksum = eeprom_read_byte(ee_addr);
   if (s_checksum != checksum) {
      memcpy_P(ram_addr, default_val, size);
      return 1;
   }
   else 
      return 0;
} 


/************************************************************************
 * Write single byte config parameter into EEPROM. 
 ************************************************************************/
 
void set_byte_param(uint8_t* ee_addr, uint8_t byte)
{
    while (!eeprom_is_ready())
      t_yield();
    eeprom_write_byte(ee_addr, byte);
    eeprom_write_byte(ee_addr+1, (0x0f ^ byte));
}

 
/************************************************************************
 * Get (and return) single byte config parameter from EEPROM. 
 * If checksum does not match, use default value from flash memory 
 * instead.
 ************************************************************************/
 
uint8_t get_byte_param(const uint8_t* ee_addr, PGM_P default_val)
{
    while (!eeprom_is_ready())
      t_yield();
    register uint8_t b1 = eeprom_read_byte(ee_addr);
    register uint8_t b2 = eeprom_read_byte(ee_addr+1);
    if ((0x0f ^ b1) == b2)
       return b1;
    else
       return pgm_read_byte(default_val);
}



