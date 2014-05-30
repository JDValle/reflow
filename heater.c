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
# include "uart.h"

# define TIME2SECS(min,sec)           (((min)*60) + (sec) )
# define LERP(a,b,t)     (((b) - (a)) * (t) + (a))

# define FAN_PWM_MIN  50
# define FAN_PWM_MAX  127
# define FAN_RAMP_WIDTH_MS   20000

# define THISTORYSIZE       LCDPCD8544_WIDTH
# define THISTORY_UPDATEMS  5000

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
  float       tambient;
  float       ttarget;
  
  uint8_t     fan_min ;
  uint8_t     fan_max ;
  uint8_t     pwm_fan ;

  uint8_t     heater0 ;

  uint32_t    prevms;
  uint32_t    lastupdatems;
  uint32_t    elapsedms;  

  uint16_t    total_seconds;

  heatsetup_t stages [HEATER_NSTAGES];
  uint8_t     thistory [THISTORYSIZE];
} heaterstate_t;

static heaterstate_t heaterstate;

void heaterstages_setup (void )
{
  heaterstate.stages [HEATER_STAGE_PREHEATER_NONE].tmin = 0 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_NONE].tmax = 0 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_NONE].seconds = 0 ;

  heaterstate.stages [HEATER_STAGE_PREHEATER_START].tmin = heaterstate.tambient ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_START].tmax = 150 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_START].seconds = 200 ;

  heaterstate.stages [HEATER_STAGE_PREHEATER_KEEP].tmin = 150 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_KEEP].tmax = 150 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_KEEP].seconds = 100 ;

  heaterstate.stages [HEATER_STAGE_REFLOW_START].tmin = 150 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_START].tmax = 220 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_START].seconds = 150 ;

  heaterstate.stages [HEATER_STAGE_REFLOW_KEEP].tmin = 220 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_KEEP].tmax = 220 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_KEEP].seconds = 100 ;

  heaterstate.stages [HEATER_STAGE_COOLDOWN].tmin = 220 ;
  heaterstate.stages [HEATER_STAGE_COOLDOWN].tmax = heaterstate.tambient ;
  heaterstate.stages [HEATER_STAGE_COOLDOWN].seconds = 350 ;

  heaterstate.stages [HEATER_STAGE_READY].tmin = 0 ;
  heaterstate.stages [HEATER_STAGE_READY].tmax = 0 ;
  heaterstate.stages [HEATER_STAGE_COOLDOWN].seconds = 0 ;

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

void fan (void)
{
  switch (heaterstate.status)
  {
    case HEATER_STATUS_IDLE :
    {
    } break ;

    case HEATER_STATUS_RUNNING :
    {
      switch (heaterstate.stage)
      {
        case HEATER_STAGE_PREHEATER_NONE :
        {
          heaterstate.fan_min = 0 ;
          heaterstate.fan_max = 0 ;
        } break ;
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
          if ( heaterstate.tcurrent > ( heaterstate.tambient + 30.0 ) )
          {
            heaterstate.fan_min = 127 ;
            heaterstate.fan_max = 127 ;            
          }
          else
          {
            heaterstate.fan_min = 0 ;
            heaterstate.fan_max = 0 ;            
          }

        } break ;
      }
    } break ; // end HEATER_STATUS_RUNNING
  }

  fan_update () ;

}

////////////////////////////////////////////////////////////////////////////
// HEAT
////////////////////////////////////////////////////////////////////////////

void heater0 (void )
{
  switch (heaterstate.status)
  {
    case HEATER_STATUS_IDLE :
    {
    } break ;

    case HEATER_STATUS_RUNNING :
    {
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
          heaterstate.status = HEATER_STATUS_IDLE ;
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
        else
        {
          const float ftmin = (float)(tmin);
          const float ftmax = (float)(tmax);
          const float felapsed =  (float) (heaterstate.elapsedms / 1000);
          const float fseconds =  (float) (heaterstate.stages[heaterstate.stage].seconds);
          const float k = felapsed / fseconds ;

          heaterstate.ttarget = LERP(ftmin,ftmax,k);          
        }

      }

      switch (heaterstate.stage)
      {
        case HEATER_STAGE_PREHEATER_NONE :
        {
        } break ;
        case HEATER_STAGE_PREHEATER_START :
        {
          pid_tune( 2 , 0.0 , 0.0 ) ;
        } break ;
        case HEATER_STAGE_PREHEATER_KEEP  :
        {
          pid_tune( 2 , 0.010 , 0.15 ) ;
        } break ;
        case HEATER_STAGE_REFLOW_START    :
        {
          pid_tune( 5 , 0.0 , 0.0 ) ;
        } break ;
        case HEATER_STAGE_REFLOW_KEEP     :
        {
          pid_tune( 2 , 0.011 , 0.15 ) ;
        } break ;
        case HEATER_STAGE_COOLDOWN :
        {
          pid_tune( 2 , 0.011 , 0.15 ) ;
        } break ;
        case HEATER_STAGE_READY :
        {
          pid_tune( 2 , 0.011 , 0.15 ) ;
        } break ;
      }

    } break ; // end HEATER_STATUS_RUNNING
  }

  pid_compute ( heaterstate.tcurrent , heaterstate.ttarget , &heaterstate.heater0 ) ;
}

void thistory (void )
{
  const uint32_t ms = heaterstate.elapsedms - heaterstate.lastupdatems ;
  if ( ms < THISTORY_UPDATEMS ) return ;

  const float k = ( heaterstate.tcurrent / 300.0) ;
  heaterstate.thistory [THISTORYSIZE-1] =  (uint8_t ) ( (LCDPCD8544_ROWHEIGHT * 4.0 - 2) * k );

  uint8_t i ;
  for ( i = 0 ; i < (THISTORYSIZE-1) ; ++ i )
  {
    heaterstate.thistory [i] = heaterstate.thistory [i+1];
  }

  heaterstate.lastupdatems = heaterstate.elapsedms ;
}

////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////

void heater_setstage (const uint8_t stage )
{
    heaterstate.stage = stage ;
    heaterstate.elapsedms =  0 ;
    heaterstate.total_seconds = heaterstate.stages [stage].seconds ;
    heaterstate.prevms = timer_ms() ;
    pid_beep () ;
}

////////////////////////////////////////////////////////////////////////////
// DISPLAY
////////////////////////////////////////////////////////////////////////////

void heat_display_line0 (void )
{
  char * dst = lcd_tmpstring();

  const uint16_t tcurrent = (uint16_t)(heaterstate.tcurrent);
  const uint16_t ttarget = (uint16_t)(heaterstate.ttarget);
  const uint8_t fan = (uint16_t) fan2pwm ( heaterstate.pwm_fan );

  sprintf ( dst , "T%3d/%3d -  F%3d" , tcurrent , ttarget , fan ) ;

  lcd_print ( dst , 0 , 0) ;
}

void heat_display_status (void )
{
  char * dst = lcd_tmpstring();
  
  // STATUS
  {
    switch ( heaterstate.status )
    {
      case HEATER_STATUS_IDLE :
      {
				sprintf ( dst , "%04d" , heaterstate.heater0 ) ;       
      } break ;
      case HEATER_STATUS_RUNNING :
      {
        switch (heaterstate.stage)
        {
        case HEATER_STAGE_PREHEATER_NONE  : { strcpy ( dst , "") ; } break ;
        case HEATER_STAGE_PREHEATER_START : { strcpy ( dst , "PREHEAT A") ; } break ;
        case HEATER_STAGE_PREHEATER_KEEP  : { strcpy ( dst , "PREHEAT B") ; } break ;
        case HEATER_STAGE_REFLOW_START    : { strcpy ( dst , "REFLOW  A") ; } break ;
        case HEATER_STAGE_REFLOW_KEEP     : { strcpy ( dst , "REFLOW  B") ; } break ;
        case HEATER_STAGE_COOLDOWN        : { strcpy ( dst , "COOLDOWN ") ; } break ;
        }
      } break ;
    }
    lcd_print ( dst , 0 , 5) ;
  }

  // TIME
  if ( heaterstate.total_seconds > 0 )    
  {
    const uint16_t elapsed_seconds = heaterstate.elapsedms / 1000 ;
    const uint16_t seconds = heaterstate.total_seconds - elapsed_seconds ;
    const uint16_t m = (seconds / 60) % 60;
    const uint16_t s = seconds % 60;
    sprintf ( dst , "%02d:%02d" , m , s ) ;
  }
  else
  {
    const uint16_t seconds = heaterstate.elapsedms / 1000 ;
    const uint16_t m = (seconds / 60) % 60;
    const uint16_t s = seconds % 60;
    sprintf ( dst , "%02d:%02d" , m , s ) ;
  }
  lcd_print ( dst , 11 , 5) ;   
}

void heater_graph (void )
{
  uint8_t x,y;
  for ( x = 0 ; x < THISTORYSIZE ; ++ x )
  {
    const uint8_t h = heaterstate.thistory [x] ;

    for ( y = 0; y < h ; ++ y )
    {
      lcdPCD8544_changepixel ( x , 38 - y , 1 ) ;
    }
  }
}

void heater_log (void )
{
# ifdef UART_ENABLE
  const uint16_t tcurrent = (uint16_t)(heaterstate.tcurrent);
  const uint16_t ttarget = (uint16_t)(heaterstate.ttarget);

  char * dst = lcd_tmpstring() ;
  sprintf ( dst , "%03d,%03d,%03d\n\r" , (int)tcurrent, (int)ttarget, (int)heaterstate.heater0 ) ;
  uart_send_async_wait ( dst , strlen (dst) ) ;
  timer_wait_ms (100);
# endif
}

void heater_display (void )
{
  lcd_cls ();
  heat_display_line0 () ;
  heater_graph () ;
  heat_display_status () ;
  heater_log () ;
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
  // update time
  {
    const uint32_t ms = timer_ms() ;
    heaterstate.elapsedms += abs ( heaterstate.prevms - ms ) ;
    heaterstate.prevms = ms ;       
  }

  // update temp
  temperature ( & heaterstate.tambient , & heaterstate.tcurrent ) ;

  // update
  heater0 () ;
  fan () ;
  thistory () ;
}

void heater_settemp (const uint8_t temp)
{
  heaterstate.ttarget = (float ) temp ;
  heater_setstage (HEATER_STAGE_PREHEATER_NONE) ;
  heaterstate.status = HEATER_STATUS_IDLE ;

  if (temp > 0 )
  {
    pid_initialize ( heaterstate.tcurrent , heaterstate.heater0 ) ;
    pid_setmode( PID_AUTOMATIC , heaterstate.tcurrent , heaterstate.heater0 ) ;
  }
  else
  {
    pid_setmode( PID_MANUAL , 0 , 0 ) ;    
    heaterstate.heater0 = 0 ;
  }
}

void heater_setfan (const uint8_t fan)
{
  heaterstate.fan_min = fan ;
  heaterstate.fan_max = fan ;
  heater_setstage (HEATER_STAGE_PREHEATER_NONE) ;
  heaterstate.status = HEATER_STATUS_IDLE ;
}

void heater_run (void )
{
  heater_setstage (HEATER_STAGE_PREHEATER_START);
  heaterstate.status = HEATER_STATUS_RUNNING ;

  pid_initialize ( heaterstate.tcurrent , heaterstate.heater0 ) ;
  pid_setmode( PID_AUTOMATIC , heaterstate.tcurrent , heaterstate.heater0 ) ;
}

void heater_stop (void )
{
  heater_settemp (0);
}

void heater_init (void )
{
  adc_init_singlemode ();
  temperature ( & heaterstate.tambient , & heaterstate.tcurrent ) ;
  heaterstate.ttarget = 0 ;  
  heaterstate.lastupdatems = 0 ;
  memset ( heaterstate.thistory , 0 , THISTORYSIZE ) ;
  heaterstate.pwm_fan = 0 ;
  heaterstages_setup ();
  pid_init ();
  heater_stop ();
}
