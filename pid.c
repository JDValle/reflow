# include "common.h"
# include "pid.h"
# include "timer.h"
# include <stdint.h>
# include <avr/io.h>

volatile static uint8_t pwm_count ;
volatile static uint8_t pwm_heat ;

void pid_init   (void )
{
	pwm_count = 0 ;
	pwm_heat = 0 ;

	SetBit ( DDRD , PD3 ) ;
	ClearBit ( PORTD , PD3 ) ;
}

void pid_setheat (const uint8_t heat )
{
	pwm_heat = heat ;
}

void pid_update (void )
{
	if ( pwm_count == 0 )
	{
		if (pwm_heat>0)		SetBit ( PORTD , PD3 ) ;
	}

	if (pwm_heat<pwm_count)
	{
		ClearBit ( PORTD , PD3 ) ;
	}

	++ pwm_count ;
	pwm_count &= 0x7f ;

}
