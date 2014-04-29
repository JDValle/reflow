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

const uint32_t timer_ms (void )
{
  uint32_t t;

  ClearBit ( TIMSK1, OCIE1A ) ;
  t = timer_milliseconds ;
  SetBit ( TIMSK1 , OCIE1A ) ;

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
}

//////////////////////////////////////////////////////////////////////////////

# define F_FAST               125000
# define LOWRESTICKS_MS       F_FAST / 1000
# define LOWRESTIMER_RESET    (F_CPU / (2*F_FAST) ) - 1

volatile uint32_t lowrestimer_currentticks ;

void lowrestimer_init (void )
{
  lowrestimer_currentticks = 0 ;
  TCCR0A = (0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (1<<WGM01) | (0<<WGM00) ;
  TCCR0B = (0<<FOC0A)  | (0<<FOC0B)  | (0<<WGM02)  | (0<<CS02)  | (0<<CS01)   | (1<<CS00) ;

  OCR0A = LOWRESTIMER_RESET ;
  OCR0B = 0 ;

  TCNT0 = 0 ;

  SetBit ( TIMSK0 , OCIE0A ) ;
}

const uint32_t lowrestimer_ticks (void )
{
  uint32_t t;

  ClearBit ( TIMSK0, OCIE0A ) ;
  t = lowrestimer_currentticks ;
  SetBit ( TIMSK0 , OCIE0A ) ;

  return t ;
}

void lowrestimer_wait_ticks ( uint32_t ticks )
{
  uint32_t prev = lowrestimer_ticks() ;

  for (;;)
    {
      const uint32_t v = lowrestimer_ticks() ;
      
      if ( prev==v ) continue;
      prev = v ;
      
      if ( ( -- ticks ) == 0 ) return ;
    } 
}

ISR(TIMER0_COMPA_vect)
{
  ++ lowrestimer_currentticks;
  lowrestimer_handler ();
}
