/*
 * $Id: config.h,v 1.34 2009-05-15 22:48:52 la7eca Exp $
 *
 * Definition of parameters to be stored in EEPROM, their default 
 * values in program memory plus trace info in noinit part of RAM.
 * Macros for defining and accessing these values. 
 */
 
#if !defined __CONFIG_H__
#define __CONFIG_H__

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "ax25.h"

#if defined __CONFIG_C__

#define DEFINE_PARAM(x, t) \
     EEMEM t PARAM_##x;       \
     EEMEM uint8_t PARAM_##x##_CRC
     
#define DEFAULT_PARAM(x) const PROGMEM __typeof__(PARAM_##x) PARAM_DEFAULT_##x

#else

#define DEFINE_PARAM(x, t); \
     extern EEMEM t PARAM_##x; \
     extern const PROGMEM t PARAM_DEFAULT_##x 
     
#endif


#define COMMENT_LENGTH 40
#define TRACE_LENGTH 12
#define OBJID_LENGTH 10

typedef addr_t __digilist_t[7];  
typedef char comment[COMMENT_LENGTH];
typedef char obj_id_t[OBJID_LENGTH];
typedef uint8_t __trace_t[TRACE_LENGTH][2];

/***************************************************************
 * Define parameters:
 *            Name     Type     
 ***************************************************************/ 
 
DEFINE_PARAM( VERSION_KEY,        uint8_t      );     
DEFINE_PARAM( MYCALL,             addr_t       );
DEFINE_PARAM( DEST,               addr_t       );
DEFINE_PARAM( DIGIS,              __digilist_t );
DEFINE_PARAM( NDIGIS,             uint8_t      );
DEFINE_PARAM( TXDELAY,            uint8_t      );
DEFINE_PARAM( TXTAIL,             uint8_t      );
DEFINE_PARAM( MAXFRAME,           uint8_t      ); 
DEFINE_PARAM( TRX_FREQ,           uint32_t     );
DEFINE_PARAM( TRX_CALIBRATE,      int16_t      );
DEFINE_PARAM( TRX_TXPOWER,        double       );
DEFINE_PARAM( TRX_AFSK_DEV,       uint16_t     );
DEFINE_PARAM( TRX_SQUELCH,        double       );
DEFINE_PARAM( TRX_AFC,            uint16_t     );
DEFINE_PARAM( TRACKER_ON,         uint8_t      );
DEFINE_PARAM( TRACKER_SLEEP_TIME, uint8_t      ); 
DEFINE_PARAM( SYMBOL,             uint8_t      );
DEFINE_PARAM( SYMBOL_TABLE,       uint8_t      );
DEFINE_PARAM( TIMESTAMP_ON,       uint8_t      );
DEFINE_PARAM( COMPRESS_ON,        uint8_t      );
DEFINE_PARAM( ALTITUDE_ON,        uint8_t      );
DEFINE_PARAM( REPORT_COMMENT,     comment      ); 
DEFINE_PARAM( GPS_BAUD,           uint16_t     );
DEFINE_PARAM( TRACKER_TURN_LIMIT, uint16_t     );
DEFINE_PARAM( TRACKER_MAXPAUSE,   uint8_t      );
DEFINE_PARAM( TRACKER_MINDIST,    uint8_t      ); 
DEFINE_PARAM( TRACKER_MINPAUSE,   uint8_t      ); 
DEFINE_PARAM( STATUS_TIME,        uint8_t      );
DEFINE_PARAM( REPORT_BEEP,        uint8_t      );
DEFINE_PARAM( GPS_POWERSAVE,      uint8_t      );
DEFINE_PARAM( TXMON_ON,           uint8_t      ); 
DEFINE_PARAM( AUTOPOWER,          uint8_t      );
DEFINE_PARAM( OBJ_SYMBOL,         uint8_t      );
DEFINE_PARAM( OBJ_SYMBOL_TABLE,   uint8_t      );
DEFINE_PARAM( OBJ_ID,             obj_id_t     );
DEFINE_PARAM( BOOT_SOUND,         uint8_t      );
DEFINE_PARAM( FAKE_REPORTS,       uint8_t      );
DEFINE_PARAM( REPEAT,             uint8_t      );
DEFINE_PARAM( EXTRATURN,          uint8_t      ); 
DEFINE_PARAM( DIGIPEATER_ON,      uint8_t      );
DEFINE_PARAM( DIGIPEATER_WIDE1,   uint8_t      );
DEFINE_PARAM( DIGIPEATER_SAR,     uint8_t      );

extern __trace_t trace           __attribute__ ((section (".noinit")));
extern uint8_t   trace_index[]   __attribute__ ((section (".noinit")));

#if defined __CONFIG_C__


/***************************************************************
 * Default values for parameters to be stored in program
 * memory. This MUST be done for each parameter defined above
 * or the linker will complain.
 ***************************************************************/

DEFAULT_PARAM( VERSION_KEY )         = 0;
DEFAULT_PARAM( MYCALL )              = {"NOCALL",0};
DEFAULT_PARAM( DEST )                = {"APPT10", 0};
DEFAULT_PARAM( DIGIS )               = {{"WIDE1",1}, {"WIDE2", 2}};
DEFAULT_PARAM( NDIGIS )              = 2;
DEFAULT_PARAM( TXDELAY )             = 20;
DEFAULT_PARAM( TXTAIL )              = 10;
DEFAULT_PARAM( MAXFRAME )            = 3;
DEFAULT_PARAM( TRX_FREQ )            = 144.800e6;
DEFAULT_PARAM( TRX_CALIBRATE )       = 0;
DEFAULT_PARAM( TRX_TXPOWER )         = 13.0;
DEFAULT_PARAM( TRX_AFSK_DEV )        = 0;
DEFAULT_PARAM( TRX_SQUELCH )         = -100;
DEFAULT_PARAM( TRX_AFC )             = 2000;
DEFAULT_PARAM( TRACKER_ON )          = 1;
DEFAULT_PARAM( TRACKER_SLEEP_TIME )  = 10; 
DEFAULT_PARAM( SYMBOL)               = '[';
DEFAULT_PARAM( SYMBOL_TABLE)         = '/';
DEFAULT_PARAM( TIMESTAMP_ON)         = 1;
DEFAULT_PARAM( COMPRESS_ON)          = 0;
DEFAULT_PARAM( ALTITUDE_ON)          = 0;
DEFAULT_PARAM( REPORT_COMMENT )      = "Polaric Tracker";
DEFAULT_PARAM( GPS_BAUD )            = 4800;
DEFAULT_PARAM( TRACKER_TURN_LIMIT )  = 35;
DEFAULT_PARAM( TRACKER_MAXPAUSE )    = 18;
DEFAULT_PARAM( TRACKER_MINDIST )     = 100;
DEFAULT_PARAM( TRACKER_MINPAUSE )    = 3;
DEFAULT_PARAM( STATUS_TIME )         = 30;
DEFAULT_PARAM( REPORT_BEEP )         = 0;
DEFAULT_PARAM( GPS_POWERSAVE )       = 0;
DEFAULT_PARAM( TXMON_ON )            = 0;
DEFAULT_PARAM( AUTOPOWER )           = 0;
DEFAULT_PARAM( OBJ_SYMBOL)           = 'c';
DEFAULT_PARAM( OBJ_SYMBOL_TABLE)     = '/';
DEFAULT_PARAM( OBJ_ID)               = "MARK-";
DEFAULT_PARAM( BOOT_SOUND )          = 1;
DEFAULT_PARAM( FAKE_REPORTS )        = 0;
DEFAULT_PARAM( REPEAT )              = 0;
DEFAULT_PARAM( EXTRATURN )           = 0;
DEFAULT_PARAM( DIGIPEATER_ON )       = 0;
DEFAULT_PARAM( DIGIPEATER_WIDE1 )    = 0;
DEFAULT_PARAM( DIGIPEATER_SAR)       = 1;

__trace_t trace            __attribute__ ((section (".noinit")));
uint8_t   trace_index[2]   __attribute__ ((section (".noinit")));

#endif
 
#define FIRST_PARAM PARAM_MYCALL
#define LAST_PARAM PARAM_FAKE_REPORTS


/***************************************************************
 * Functions/macros for accessing parameters
 ***************************************************************/

#define RESET_PARAM(x)         reset_param(&PARAM_##x, sizeof(PARAM_##x))
#define GET_PARAM(x, val)      get_param(&PARAM_##x, (val), sizeof(PARAM_##x),(PGM_P) &PARAM_DEFAULT_##x)
#define SET_PARAM(x, val)      set_param(&PARAM_##x, (val), sizeof(PARAM_##x))
#define GET_BYTE_PARAM(x)      get_byte_param(&PARAM_##x,(PGM_P) &PARAM_DEFAULT_##x)
#define SET_BYTE_PARAM(x, val) set_byte_param(&PARAM_##x, ((uint8_t) val))

void    reset_all_param(void);
void    reset_param(void*, uint8_t);
void    set_param(void*, const void*, uint8_t);
int     get_param(const void*, void*, uint8_t, PGM_P);
void    set_byte_param(uint8_t*, uint8_t);
uint8_t get_byte_param(const uint8_t*, PGM_P);


/* Tracing. 
 * Two vectors:  0 is current, 1 is saved from previous run 
 */
void show_trace(char*, uint8_t, PGM_P, PGM_P);

#define TRACE_INIT         for (uint8_t i=0; i<TRACE_LENGTH; i++) trace[i][1] = trace[i][0]; \
                           trace_index[1] = trace_index[0]; \
                           trace_index[0] = 0; \
                           for (uint8_t i=0; i<TRACE_LENGTH; i++) trace[i][0] = 0;
                           
#define TRACE(val)         trace[trace_index[0]++ % TRACE_LENGTH][0] = (val)    
#define GET_TRACE(s,i)     trace[(trace_index[(s)] + (i)) % TRACE_LENGTH][(s)]               
#define TRACE_LAST         TRACE_LENGTH-1
                   
#endif /* __CONFIG_H__ */
