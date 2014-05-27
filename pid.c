# include "common.h"
# include "pid.h"
# include "timer.h"
# include <stdint.h>
# include <avr/io.h>

# define BUZZ_TICKS					126

# define PINS_WRITE_PWMFAN			PORTD
# define PINS_DDR_PWMFAN			DDRD
# define PINS_OFFSET_PWMFAN			5

# define PINS_WRITE_PWMHEATER0		PORTD
# define PINS_DDR_PWMHEATER0		DDRD
# define PINS_OFFSET_PWMHEATER0		6

# define PINS_WRITE_BUZZ			PORTB
# define PINS_DDR_BUZZ				DDRB
# define PINS_OFFSET_BUZZ			4

volatile static uint8_t pwm_count ;
volatile static uint8_t pwm_fan ;
volatile static uint8_t pwm_heater0 ;
volatile static uint8_t pwm_buzz ;

void pid_init   (void )
{
	pwm_count = 0 ;

	pwm_fan = 0 ;
	pwm_heater0 = 0 ;
	pwm_buzz = 0 ;

	SetBit   ( PINS_DDR_PWMFAN       , PINS_OFFSET_PWMFAN ) ;
	SetBit   ( PINS_DDR_PWMHEATER0   , PINS_OFFSET_PWMHEATER0 ) ;
	SetBit   ( PINS_DDR_BUZZ         , PINS_OFFSET_BUZZ ) ;

	ClearBit ( PINS_WRITE_PWMFAN     , PINS_OFFSET_PWMFAN ) ;
	ClearBit ( PINS_WRITE_PWMHEATER0 , PINS_OFFSET_PWMHEATER0 ) ;
	ClearBit ( PINS_WRITE_BUZZ       , PINS_OFFSET_BUZZ ) ;
}

void pid_setfan (const uint8_t value )
{
	timer_cs_start () ;
	pwm_fan = value ;
	timer_cs_end () ;
}
void pid_setheater0 (const uint8_t value )
{
	timer_cs_start () ;
	pwm_heater0 = value ;
	timer_cs_end () ;
}

void pid_beep (void)
{
	timer_cs_start () ;
	pwm_buzz = BUZZ_TICKS ;
	timer_cs_end () ;
}

void pid_update (void )
{
	if ( pwm_count == 0 )
	{
		if (pwm_heater0>0) SetBit ( PINS_WRITE_PWMHEATER0 , PINS_OFFSET_PWMHEATER0 ) ;
		if (pwm_fan>0  ) SetBit ( PINS_WRITE_PWMFAN     , PINS_OFFSET_PWMFAN ) ;
	}
	if (pwm_buzz>0 ) SetBit ( PINS_WRITE_BUZZ       , PINS_OFFSET_BUZZ ) ;

	if (pwm_heater0<pwm_count)
	{
		ClearBit ( PINS_WRITE_PWMHEATER0 , PINS_OFFSET_PWMHEATER0 ) ;
	}

	if (pwm_fan<pwm_count)
	{
		ClearBit ( PINS_WRITE_PWMFAN , PINS_OFFSET_PWMFAN ) ;
	}

	if (pwm_buzz<pwm_count)
	{
		ClearBit ( PINS_WRITE_BUZZ , PINS_OFFSET_BUZZ ) ;
		pwm_buzz = 0 ; // reset
	}

	++ pwm_count ;
	pwm_count &= 0x7f ;
}
