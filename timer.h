# ifndef REFLOW_TIMER_INCLUDE 
# define REFLOW_TIMER_INCLUDE

# include <stdint.h>

extern const uint32_t timer_ms (void ) ;
extern void timer_init  (void ) ;
extern void timer_wait_ms ( uint32_t ms ) ;
extern void timer_handler (void);
extern void timer_cs_start (void );
extern void timer_cs_end   (void );

# endif // REFLOW_TIMER_INCLUDE
