# include <avr/io.h>

# define EXTERNAL_VCCREF

void adc_init_singlemode(void)
{
  ADCSRA = 0x0 ;

  // Set ADC prescalar to 128 - 125KHz sample rate @ 16MHz
  // No MUX values needed to be changed to use ADC0
  ADCSRA |= _BV(ADPS2) ;
  ADCSRA |= _BV(ADPS1) ;
  ADCSRA |= _BV(ADPS0) ;

  ADMUX= 0x0; // control of multiplexir
  
  // reference voltage
# ifdef INTERNAL_VCCREF
  ADMUX |=  _BV(REFS0) ;
  ADMUX |=  _BV(REFS1) ;
# endif
  
# ifdef EXTERNAL_VCCREF
  ADMUX |=  _BV(REFS0) ;
  ADMUX &= ~_BV(REFS1) ;
# endif  

  // disable left read
  ADMUX &= ~_BV(ADLAR) ;

  // disable digital input
  DIDR0 = _BV(ADC5D) | _BV(ADC4D) | _BV(ADC3D) | _BV(ADC2D) | _BV(ADC1D) | _BV(ADC0D) ;
}

uint16_t adc_single_read ( const uint8_t ch )
{
  ADCSRA |= _BV(ADEN);  // Enable ADC

  ADMUX |= (ch & 0b00000111) ;	// channel

  ADCSRA |= (1 << ADSC);  // Start A2D Conversions
  while (ADCSRA & (1<<ADSC)); // wait for conversion
  
  return ADC ;
}
