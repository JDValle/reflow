# include <avr/eeprom.h>

typedef struct
{
	uint8_t		version ;
} settings_t ;

settings_t eeprom_settings EEMEM = ( settings_t )
    {
      .version = 0 ,
    } ;

static settings_t settings ;

void settings_load (void )
{
	eeprom_read_block( &settings , &eeprom_settings , sizeof(settings_t) ) ;
}

/*
int main(void) __attribute__((noreturn));
int main (void )
{

  cli ();
  uart_init ( UART_INIT_INTERRUPT_ENABLED );
  sei ();
  
  const uint8_t b = eeprom_read_byte ( &byte0 ) ;
*/

// FAN MIN / FAN MAX
// Kp Ki Kd
