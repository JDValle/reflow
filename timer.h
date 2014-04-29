# ifndef REFLOW_TIMER_INCLUDE 
# define REFLOW_TIMER_INCLUDE

# include <stdint.h>

extern const uint32_t timer_ms (void ) ;
extern void timer_init  (void ) ;
extern void timer_wait_ms ( uint32_t ms ) ;

extern void lowrestimer_init (void ) ;
extern void lowrestimer_wait_ticks ( uint32_t ticks ) ;

extern void lowrestimer_handler (void);

# endif // REFLOW_TIMER_INCLUDE
