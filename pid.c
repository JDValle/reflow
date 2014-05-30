# include "common.h"
# include "pid.h"
# include "timer.h"
# include <stdint.h>
# include <stdlib.h>
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

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

typedef struct
{
	float kp,ki,kd;

	uint32_t lastTime;
	uint32_t SampleTime;
	float ITerm, lastInput;
	float outMin, outMax;
	uint8_t inAuto ;
	uint8_t controllerDirection	;
} pid_t ;

pid_t pid_heater0;

inline void pid_update_iterm (void )
{
	if(pid_heater0.ITerm > pid_heater0.outMax) pid_heater0.ITerm = pid_heater0.outMax ;
	else
	if(pid_heater0.ITerm < pid_heater0.outMin) pid_heater0.ITerm = pid_heater0.outMin ;
}

inline void pid_reverse(void )
{
	pid_heater0.kp = (0.0 - pid_heater0.kp) ;
	pid_heater0.ki = (0.0 - pid_heater0.ki) ;
	pid_heater0.kd = (0.0 - pid_heater0.kd) ;
}

void pid_initialize( const float myInput , const float myOutput )
{
	if (myInput)  pid_heater0.lastInput = myInput ;
	if (myOutput) pid_heater0.ITerm     = myOutput ;

	pid_update_iterm () ;
}

uint8_t pid_compute (const float myInput , const float mySetpoint , uint8_t * output )
{
	if(!pid_heater0.inAuto) return 0 ;

	const uint32_t now = timer_ms () ;

	const uint32_t timeChange = abs (now - pid_heater0.lastTime) ;

	if(timeChange<pid_heater0.SampleTime) return  0 ;

	/*Compute all the working error variables*/
	const float input = myInput ;
	const float error = mySetpoint - input ;

	pid_heater0.ITerm += (pid_heater0.ki * error);
	pid_update_iterm () ;

	const float dInput = (input - pid_heater0.lastInput) ;

	/*Compute PID Output*/
	const float foutput = (pid_heater0.kp * error) + (pid_heater0.ITerm) - (pid_heater0.kd * dInput) ;

	*output = (uint8_t ) ( MAX ( pid_heater0.outMin , MIN ( pid_heater0.outMax , foutput ) ) ) ;

	/*Remember some variables for next time*/
	pid_heater0.lastInput = input ;
	pid_heater0.lastTime  = now ;

	return 1 ;
}

void pid_tune (const float Kp, const float Ki, const float Kd)
{
	if (Kp<0 || Ki<0 || Kd<0) return;

	//dispKp = Kp; dispKi = Ki; dispKd = Kd;

	const float SampleTimeInSec = ((float)pid_heater0.SampleTime)/1000.0;
	pid_heater0.kp = Kp;
	pid_heater0.ki = Ki * SampleTimeInSec;
	pid_heater0.kd = Kd / SampleTimeInSec;

	if(pid_heater0.controllerDirection ==PID_REVERSE)
	{
		pid_reverse () ;
	}
}

void pid_setsampletime (const uint32_t NewSampleTimeMS )
{
	if (NewSampleTimeMS <= 0) return ;

	const float ratio = (float)NewSampleTimeMS / (float)pid_heater0.SampleTime ;
	pid_heater0.ki *= ratio ;
	pid_heater0.kd /= ratio ;
	pid_heater0.SampleTime = NewSampleTimeMS ;
}

void pid_setlimits (const float Min, const float Max, float * myOutput )
{
	pid_heater0.outMin = MIN (Min , Max ) ;
	pid_heater0.outMax = MAX (Min , Max ) ;

	if(!pid_heater0.inAuto) return ;

	if (myOutput)
	{
		if(*myOutput > pid_heater0.outMax) *myOutput = pid_heater0.outMax;
		else
		if(*myOutput < pid_heater0.outMin) *myOutput = pid_heater0.outMin;			
	}

	pid_update_iterm () ;
}

void pid_setmode(const uint8_t Mode , const float myInput , const float myOutput )
{
    const uint8_t newAuto = (Mode == PID_AUTOMATIC) ;

    if(newAuto == pid_heater0.inAuto)
    { /*we just went from manual to auto*/
    	pid_initialize( myInput , myOutput ) ;
    }

    pid_heater0.inAuto = newAuto;
}

void pid_setdirection (const uint8_t Direction)
{
	if(pid_heater0.inAuto && Direction != pid_heater0.controllerDirection)
	{
		pid_reverse () ;
	}
	pid_heater0.controllerDirection = Direction;
}

///////////////////////////////////////////////////////////
// REAL TIME
///////////////////////////////////////////////////////////

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

	// PID
	{
		pid_heater0.inAuto = 0 ;
		pid_setlimits(0, 127 , NULL );
	    pid_heater0.SampleTime = 100; //0.1 seconds
		pid_setdirection(PID_DIRECT);
		pid_tune( 2 , 0.011 , 0.15 ) ;
		pid_heater0.lastTime = timer_ms() - pid_heater0.SampleTime;	
	}
}
