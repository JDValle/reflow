# include "common.h"
# include "pid.h"
# include "timer.h"
# include <stdint.h>
# include <avr/io.h>

# define PINS_WRITE_PWMFAN			PORTD
# define PINS_DDR_PWMFAN			DDRD
# define PINS_OFFSET_PWMFAN			3

volatile static uint8_t pwm_count ;
volatile static uint8_t pwm_heat ;

void pid_init   (void )
{
	pwm_count = 0 ;
	pwm_heat = 0 ;

	SetBit   ( PINS_DDR_PWMFAN   , PINS_OFFSET_PWMFAN ) ;
	ClearBit ( PINS_WRITE_PWMFAN , PINS_OFFSET_PWMFAN ) ;
}

void pid_setheat (const uint8_t heat )
{
	pwm_heat = heat ;
}

void pid_update (void )
{
	if ( pwm_count == 0 )
	{
		if (pwm_heat>0)	SetBit ( PINS_WRITE_PWMFAN , PINS_OFFSET_PWMFAN ) ;
	}

	if (pwm_heat<pwm_count)
	{
		ClearBit ( PINS_WRITE_PWMFAN , PINS_OFFSET_PWMFAN ) ;
	}

	++ pwm_count ;
	pwm_count &= 0x7f ;
}
