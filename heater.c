# include <stdio.h>
# include <stdint.h>
# include <stdlib.h>
# include <string.h>
# include "common.h"
# include "lcd.h"
# include "adc.h"
# include "temp.h"
# include "pid.h"
# include "heater.h"
# include "timer.h"

# define TIME2SECS(min,sec)           (((min)*60) + (sec) )
# define LERP(a,b,t)     (((b) - (a)) * (t) + (a))

# define FAN_PWM_MIN  50
# define FAN_PWM_MAX  127
# define FAN_RAMP_WIDTH_MS   20000

typedef struct
{
  uint16_t   tmin ;
  uint16_t   tmax ;
  uint16_t   seconds ;
}
heatsetup_t;

typedef struct
{
  uint8_t     status;
  uint8_t     stage;

  float       tcurrent;
  float       ttarget;
  
  uint8_t     fan_min ;
  uint8_t     fan_max ;
  uint8_t     pwm_fan ;

  uint8_t     heater0 ;

  uint32_t    prevms;
  uint32_t    elapsedms;
  heatsetup_t stages [HEATER_NSTAGES];
} heaterstate_t;

static heaterstate_t heaterstate;

void heaterstages_setup (void )
{
  heaterstate.stages [HEATER_STAGE_PREHEATER_START].tmin = 0 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_START].tmax = 150 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_START].seconds = TIME2SECS(0,10) ;

  heaterstate.stages [HEATER_STAGE_PREHEATER_KEEP].tmin = 150 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_KEEP].tmax = 150 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_KEEP].seconds = TIME2SECS(1,10) ;

  heaterstate.stages [HEATER_STAGE_REFLOW_START].tmin = 150 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_START].tmax = 260 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_START].seconds = TIME2SECS(0,10) ;

  heaterstate.stages [HEATER_STAGE_REFLOW_KEEP].tmin = 260 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_KEEP].tmax = 260 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_KEEP].seconds = TIME2SECS(0,10) ;

  heaterstate.stages [HEATER_STAGE_COOLDOWN].tmin = 260 ;
  heaterstate.stages [HEATER_STAGE_COOLDOWN].tmax = 0 ;
  heaterstate.stages [HEATER_STAGE_COOLDOWN].seconds = TIME2SECS(0,20) ; ;
}

////////////////////////////////////////////////////////////////////////////
// FAN
////////////////////////////////////////////////////////////////////////////

uint8_t fan2pwm (const uint8_t fan )
{
  if (!fan) return 0 ;

  const uint16_t r = (fan * (FAN_PWM_MAX - FAN_PWM_MIN) + 1) ;
  return ((uint8_t) (r >> 7)) + (FAN_PWM_MIN + 1) ;
}

void fan_update (void )
{
  const uint8_t fan_max = heaterstate.fan_max ;
  if (fan_max == 0 )
  {
    heaterstate.pwm_fan = 0 ;
    return ;
  }

  const uint8_t fan_min = heaterstate.fan_min ;
  if (fan_min == fan_max )
  {
    heaterstate.pwm_fan = fan_min ;
    return ;
  }

  const uint32_t elapsed = heaterstate.elapsedms % FAN_RAMP_WIDTH_MS ;
  const float felapsed =  (float) (elapsed / 1000) ;
  const float k = (felapsed / (FAN_RAMP_WIDTH_MS/1000)) ;

  const float ftmin = (float)( heaterstate.fan_min );
  const float ftmax = (float)( heaterstate.fan_max );

  if ( elapsed >= (FAN_RAMP_WIDTH_MS/2) )
  {
    heaterstate.pwm_fan = (uint8_t ) LERP(ftmin,ftmax, ( (k*2.0)       - 1.0 ) );
  }
  else
  {
    heaterstate.pwm_fan = (uint8_t ) LERP(ftmin,ftmax, ( ((1-k) * 2.0) - 1.0) );
  }  
}

void heater0 (void )
{
  switch (heaterstate.status)
  {
    case HEATER_STATUS_IDLE :
    {
      heaterstate.heater0 = 0 ;
    } break ;

    case HEATER_STATUS_RUNNING :
    {
      switch (heaterstate.stage)
      {
        case HEATER_STAGE_PREHEATER_START :
        {
          heaterstate.heater0 = 127 ;
        } break ;
        case HEATER_STAGE_PREHEATER_KEEP  :
        {
          heaterstate.heater0 = 64 ;
        } break ;
        case HEATER_STAGE_REFLOW_START    :
        {
          heaterstate.heater0 = 127 ;
        } break ;
        case HEATER_STAGE_REFLOW_KEEP     :
        {
          heaterstate.heater0 = 127 ;
        } break ;

        case HEATER_STAGE_COOLDOWN :
        {
          heaterstate.heater0 = 0 ;
        } break ;
      }
    } break ; // end HEATER_STATUS_RUNNING

    case HEATER_STATUS_READY :
    {
      heaterstate.heater0 = 0 ;
    } break ;

  }
}

void fan (void)
{
  switch (heaterstate.status)
  {
    case HEATER_STATUS_IDLE :
    {
      heaterstate.fan_min = 0 ;
      heaterstate.fan_max = 0 ;
    } break ;

    case HEATER_STATUS_RUNNING :
    {

      switch (heaterstate.stage)
      {
        case HEATER_STAGE_PREHEATER_START :
        case HEATER_STAGE_PREHEATER_KEEP  :
        case HEATER_STAGE_REFLOW_START    :
        case HEATER_STAGE_REFLOW_KEEP     :
        {
          heaterstate.fan_min = 1 ;
          heaterstate.fan_max = 40 ;
        } break ;

        case HEATER_STAGE_COOLDOWN :
        {
          heaterstate.fan_min = 127 ;
          heaterstate.fan_max = 127 ;
        } break ;
      }
    } break ; // end HEATER_STATUS_RUNNING

    case HEATER_STATUS_READY :
    {
      heaterstate.fan_min = 127 ;
      heaterstate.fan_max = 127 ;
    } break ;

  }

  fan_update () ;

}

////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////

void heater_setstage (const uint8_t stage )
{
    heaterstate.stage = stage ;
    heaterstate.elapsedms =  0 ;
    heaterstate.prevms = timer_ms() ;
    pid_beep () ;
}

////////////////////////////////////////////////////////////////////////////
// DISPLAY
////////////////////////////////////////////////////////////////////////////

void heat_display_line0 (void )
{
  char * dst = lcd_tmpstring();

  // CURRENT STATUS   / TIME TO GO
  switch ( heaterstate.status )
  {
    case HEATER_STATUS_IDLE :
    case HEATER_STATUS_READY :
    {
      sprintf ( dst , "TEMP %3d" , (int )heaterstate.tcurrent ) ;
    } break ;
    case HEATER_STATUS_RUNNING :
    {

      const uint16_t seconds = heaterstate.elapsedms / 1000 ;
      const uint16_t m = (seconds / 60) % 60;
      const uint16_t s = seconds % 60;

      const uint16_t tcurrent = (uint16_t)(heaterstate.tcurrent);
      const uint16_t ttarget = (uint16_t)(heaterstate.ttarget);
      const uint8_t fan = (uint16_t) fan2pwm ( heaterstate.pwm_fan );

      switch (heaterstate.stage)
      {
      case HEATER_STAGE_PREHEATER_START : { sprintf ( dst , "T %3d/%3d F %3d" , tcurrent , ttarget , fan ) ; } break ;
      case HEATER_STAGE_PREHEATER_KEEP  : { sprintf ( dst , "T %3d/%3d F %3d" , tcurrent , ttarget , fan ) ; } break ;
      case HEATER_STAGE_REFLOW_START    : { sprintf ( dst , "T %3d/%3d F %3d" , tcurrent , ttarget , fan ) ; } break ;
      case HEATER_STAGE_REFLOW_KEEP     : { sprintf ( dst , "T %3d/%3d F %3d" , tcurrent , ttarget , fan ) ; } break ;
      case HEATER_STAGE_COOLDOWN        : { sprintf ( dst , "T %3d/%3d F %3d" , tcurrent , ttarget , fan ) ; } break ;
      }


    } break ;

  }

  lcd_print ( dst , 0 , 0) ;
}

void heat_display_line1 (void )
{
  char * dst = lcd_tmpstring();

  switch ( heaterstate.status )
  {
    case HEATER_STATUS_IDLE :
    {
      strcpy ( dst , "") ;
    } break ;
    case HEATER_STATUS_RUNNING :
    {

      const uint16_t seconds = heaterstate.stages[heaterstate.stage].seconds ;
      const uint16_t m = (seconds / 60) % 60;
      const uint16_t s = seconds % 60;

      switch (heaterstate.stage)
      {
      case HEATER_STAGE_PREHEATER_START : { sprintf ( dst , "PREHEAT A  %02d:%02d" , m , s ) ; } break ;
      case HEATER_STAGE_PREHEATER_KEEP  : { sprintf ( dst , "PREHEAT B  %02d:%02d" , m , s ) ; } break ;
      case HEATER_STAGE_REFLOW_START    : { sprintf ( dst , "REFLOW  A  %02d:%02d" , m , s ) ; } break ;
      case HEATER_STAGE_REFLOW_KEEP     : { sprintf ( dst , "REFLOW  B  %02d:%02d" , m , s ) ; } break ;
      case HEATER_STAGE_COOLDOWN        : { sprintf ( dst , "COOLDOWN   %02d:%02d" , m , s ) ; } break ;
      }


    } break ;
    case HEATER_STATUS_READY :
    {
      strcpy ( dst , "READY") ;
    } break ;
  }

  lcd_print ( dst , 0 , 1) ;
}

void heater_display (void )
{
  lcd_cls ();
  heat_display_line0 () ;
  heat_display_line1 () ;
}

////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////

void heater_update (void )
{
  pid_setheater0 ( heaterstate.heater0 ) ;
  pid_setfan ( fan2pwm (heaterstate.pwm_fan) ) ;
  pid_update () ;
}

void heater_menu_update (const int8_t diff)
{
// heaterstate.pwm_fan = MAX ( 0 , MIN(127 , heaterstate.pwm_fan + diff ) ) ;
}

void heaterproc (void )
{
  heaterstate.tcurrent = temperature () ;

  heater0 () ;
  fan () ;

  switch ( heaterstate.status )
  {
    case HEATER_STATUS_IDLE :
    {
      heaterstate.ttarget = 0 ;
      heaterstate.elapsedms = 0 ;
    } break ;

    case HEATER_STATUS_RUNNING :
    {
      // update time
      {
        const uint32_t ms = timer_ms() ;
        heaterstate.elapsedms += abs ( heaterstate.prevms - ms ) ;
        heaterstate.prevms = ms ;       
      }

      // check if we crossed the time
      {
        const uint16_t seconds = heaterstate.elapsedms / 1000 ;

        if ( seconds >= heaterstate.stages[heaterstate.stage].seconds )
        {
          heater_setstage (heaterstate.stage+1);
        }       
      }

      // check if we end up
      {
        if (heaterstate.stage == HEATER_STAGE_READY)
        {
          heaterstate.status = HEATER_STATUS_READY;
          return;
        }        
      }

      // calculate target temp
      {
        const uint16_t tmin = heaterstate.stages[heaterstate.stage].tmin;
        const uint16_t tmax = heaterstate.stages[heaterstate.stage].tmax;

        if (tmin==tmax)
        {
          heaterstate.ttarget = (float)(tmin);
        }

        const float ftmin = (float)(tmin);
        const float ftmax = (float)(tmax);
        const float felapsed =  (float) (heaterstate.elapsedms / 1000);
        const float fseconds =  (float) (heaterstate.stages[heaterstate.stage].seconds);
        const float k = felapsed / fseconds ;

        heaterstate.ttarget = LERP(ftmin,ftmax,k);
      }

    } break ;

    case HEATER_STATUS_READY :
    {
      heaterstate.ttarget = 0 ;
      heaterstate.elapsedms = 0 ;
    } break ;
  }
}

void heater_start (void )
{
  heater_setstage (HEATER_STAGE_PREHEATER_START);
  heaterstate.status = HEATER_STATUS_RUNNING ;
}

void heater_stop (void )
{
  heater_setstage (HEATER_STAGE_PREHEATER_START) ;
  heaterstate.status = HEATER_STATUS_IDLE ;
}

void heater_init (void )
{
  adc_init_singlemode ();
  heaterstate.tcurrent = temperature () ;
  heaterstate.pwm_fan = 0 ;
  heaterstages_setup ();
  pid_init ();
  heater_stop ();
}
