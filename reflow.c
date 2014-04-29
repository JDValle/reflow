# include <stdlib.h>
# include <avr/interrupt.h>
# include <avr/power.h>

# include "common.h"
# include "timer.h"
# include "menu.h"
# include "heater.h"

////////////////////////////////////////////////////////////////////////

void lowrestimer_handler ()
{
  menu_update () ; 
}

int main(void) __attribute__((noreturn));
int main (void)
{
	cli ();
	clock_prescale_set (0);
	timer_init ();
	lowrestimer_init ();
	sei();

  	heater_init();
	menu_init ();

	for (;;)
	{
    heaterproc();
	menuproc ();
	}
}

