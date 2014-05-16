# include <avr/eeprom.h>

typedef struct
{
	uint8_t		version ;
} settings_t ;

const settings_t eeprom_settings EEMEM = ( settings_t )
    {
      .version = 0 ,
    } ;

static settings_t settings ;

void settings_load (void )
{
//	eeprom_read_block( &settings , &eeprom_settings , sizeof(settings_t) ) ;
}

// FAN MIN / FAN MAX
// Kp Ki Kd
// buzzer on/off
