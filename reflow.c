# include <stdlib.h>
# include <avr/interrupt.h>
# include <avr/power.h>

# include "common.h"
# include "settings.h"
# include "timer.h"
# include "menu.h"
# include "heater.h"
# include "pid.h"

////////////////////////////////////////////////////////////////////////

void timer_handler ()
{
	heater_update ();
	menu_update () ; 
}

int main(void) __attribute__((noreturn));
int main (void)
{
	cli ();
	clock_prescale_set (0);
	settings_load () ;
	timer_init ();
	sei();

  	heater_init();
	menu_init ();

	for (;;)
	{
    heaterproc();
	menuproc ();
	}
}

