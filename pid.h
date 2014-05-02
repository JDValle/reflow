# ifndef REFLOW_PID_INCLUDE 
# define REFLOW_PID_INCLUDE

# include <stdint.h>

extern void pid_init   (void ) ;
extern void pid_update (void ) ;

extern void pid_setheat (const uint8_t heat) ;

# endif // REFLOW_PID_INCLUDE