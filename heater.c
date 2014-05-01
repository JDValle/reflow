# include <stdio.h>
# include <stdint.h>
# include <stdlib.h>
# include <string.h>
# include "lcd.h"
# include "adc.h"
# include "temp.h"
# include "pid.h"
# include "heater.h"
# include "timer.h"

# define TIME2SECS(min,sec)           (((min)*60) + (sec) )
# define LERP(a,b,t)     (((b) - (a)) * (t) + (a))

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
  uint32_t    prevms;
  uint32_t    elapsedms;
  heatsetup_t stages [HEATER_NSTAGES];
} heaterstate_t;

static heaterstate_t heaterstate;

void heaterstages_setup (void )
{
  heaterstate.stages [HEATER_STAGE_PREHEATER_START].tmin = 0 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_START].tmax = 150 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_START].seconds = TIME2SECS(0,22) ;

  heaterstate.stages [HEATER_STAGE_PREHEATER_KEEP].tmin = 150 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_KEEP].tmax = 150 ;
  heaterstate.stages [HEATER_STAGE_PREHEATER_KEEP].seconds = TIME2SECS(0,10) ;

  heaterstate.stages [HEATER_STAGE_REFLOW_START].tmin = 150 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_START].tmax = 260 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_START].seconds = TIME2SECS(0,50) ;

  heaterstate.stages [HEATER_STAGE_REFLOW_KEEP].tmin = 260 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_KEEP].tmax = 260 ;
  heaterstate.stages [HEATER_STAGE_REFLOW_KEEP].seconds = TIME2SECS(0,20) ;

  heaterstate.stages [HEATER_STAGE_COOLDOWN].tmin = 260 ;
  heaterstate.stages [HEATER_STAGE_COOLDOWN].tmax = 0 ;
  heaterstate.stages [HEATER_STAGE_COOLDOWN].seconds = TIME2SECS(0,20) ; ;
}

////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////

void heater_setstage (const uint8_t stage )
{
    heaterstate.stage = stage ;
    heaterstate.elapsedms =  0 ;
    heaterstate.prevms = timer_ms() ;
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
      sprintf ( dst , "%06d" , (int )heaterstate.tcurrent ) ;
    } break ;
    case HEATER_STATUS_RUNNING :
    {

      const uint16_t seconds = heaterstate.elapsedms / 1000 ;
      const uint16_t m = (seconds / 60) % 60;
      const uint16_t s = seconds % 60;

      const uint16_t tcurrent = (uint16_t)(heaterstate.tcurrent);
      const uint16_t ttarget = (uint16_t)(heaterstate.ttarget);

      switch (heaterstate.stage)
      {
      case HEATER_STAGE_PREHEATER_START : { sprintf ( dst , "%03d/%03d    %02d:%02d" , tcurrent , ttarget , m , s ) ; } break ;
      case HEATER_STAGE_PREHEATER_KEEP  : { sprintf ( dst , "%03d/%03d    %02d:%02d" , tcurrent , ttarget , m , s ) ; } break ;
      case HEATER_STAGE_REFLOW_START    : { sprintf ( dst , "%03d/%03d    %02d:%02d" , tcurrent , ttarget , m , s ) ; } break ;
      case HEATER_STAGE_REFLOW_KEEP     : { sprintf ( dst , "%03d/%03d    %02d:%02d" , tcurrent , ttarget , m , s ) ; } break ;
      case HEATER_STAGE_COOLDOWN        : { sprintf ( dst , "%03d/%03d    %02d:%02d" , tcurrent , ttarget , m , s ) ; } break ;
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
  pid_update () ;
}

void heaterproc (void )
{
  heaterstate.tcurrent = temperature () ;

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
  heaterstages_setup ();
  pid_init ();
  heater_stop ();
}
