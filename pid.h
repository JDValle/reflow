# ifndef REFLOW_PID_INCLUDE 
# define REFLOW_PID_INCLUDE

# include <stdint.h>

extern void pid_init   (void ) ;
extern void pid_update (void ) ;

extern void pid_setfan (const uint8_t value) ;
extern void pid_setheater0 (const uint8_t value) ;
extern void pid_beep (void) ;

# endif // REFLOW_PID_INCLUDE