# ifndef REFLOW_PID_INCLUDE 
# define REFLOW_PID_INCLUDE

# include <stdint.h>

# define PID_AUTOMATIC	1
# define PID_MANUAL		0
# define PID_DIRECT		0
# define PID_REVERSE	1

extern void pid_init   (void ) ;
extern void pid_update (void ) ;

extern void pid_initialize( const float myInput , const float myOutput ) ;
extern uint8_t pid_compute (const float myInput , const float mySetpoint , float * myOutput ) ;
extern void pid_tune (const float Kp, const float Ki, const float Kd) ;
extern void pid_setsampletime (const uint32_t NewSampleTimeMS ) ;
extern void pid_setlimits (const float Min, const float Max, float * myOutput ) ;
extern void pid_setmode(const uint8_t Mode , const float myInput , const float myOutput ) ;
extern void pid_setdirection (const uint8_t Direction) ;

extern void pid_setfan (const uint8_t value) ;
extern void pid_setheater0 (const uint8_t value) ;
extern void pid_beep (void) ;

# endif // REFLOW_PID_INCLUDE