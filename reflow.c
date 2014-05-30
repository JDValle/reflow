# include <stdlib.h>
# include <avr/interrupt.h>
# include <avr/power.h>

# include "common.h"
# include "settings.h"
# include "timer.h"
# include "menu.h"
# include "heater.h"
# include "pid.h"

# ifdef UART_ENABLE
# include <stdio.h>
# include <string.h>
# include "uart.h"
# include "lcd.h"
# endif

////////////////////////////////////////////////////////////////////////

void timer_handler ()
{
	heater_update ();
	menu_update () ; 
}

int main(void) __attribute__((noreturn));
int main (void)
{
	// do this before disabling any interrupt
	settings_load () ;

	cli ();
	clock_prescale_set (0);
	timer_init ();
# ifdef UART_ENABLE
	uart_init (UART_INIT_INTERRUPT_ENABLED) ;
# endif
	sei();

# ifdef UART_ENABLE
	{
		char *dst = lcd_tmpstring () ;
		strcpy (  dst , "== BEGIN ==\n\r") ;
		uart_send_async_wait (dst , strlen(dst) ) ;		
	}
# endif

	heater_init();
	menu_init ();

	for (;;)
	{
		heaterproc();
		menuproc ();
	}
}

