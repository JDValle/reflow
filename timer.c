# include <stdint.h>
# include <avr/io.h>
# include <avr/interrupt.h>
# include "common.h"
# include "timer.h"

# define TIMER_PRESCALER	8
# define TIMER_MS		( ( ( F_CPU / TIMER_PRESCALER ) / 1000 ) - 1 )

volatile uint32_t timer_milliseconds ;

void timer_init (void )
{
  timer_milliseconds = 0 ;
  TCCR1B =  ( 1 << WGM12 ) | ( 0 << CS12 ) | ( 1 << CS11 ) | ( 0 << CS10 ) ;
  OCR1A  =  TIMER_MS ;

  SetBit ( TIMSK1 , OCIE1A ) ;
}

void timer_cs_start (void )
{
  ClearBit ( TIMSK1, OCIE1A ) ;
}

void timer_cs_end   (void )
{
  SetBit ( TIMSK1 , OCIE1A ) ;  
}

const uint32_t timer_ms (void )
{
  uint32_t t;

  timer_cs_start () ;
  t = timer_milliseconds ;
  timer_cs_end () ;

  return t ;
}

void timer_wait_ms ( uint32_t ms )
{
  uint32_t prev = timer_ms() ;

  for (;;)
    {
      const uint32_t v = timer_ms() ;
      
      if ( prev==v ) continue;
      prev = v ;
      
      if ( ( -- ms ) == 0 ) return ;
    }  
}

ISR(TIMER1_COMPA_vect)
{
  ++ timer_milliseconds ;  
  timer_handler () ;
}

//////////////////////////////////////////////////////////////////////////////
