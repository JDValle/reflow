# include <avr/io.h>
# include "common.h"

# define EXTERNAL_VCCREF

void adc_init_singlemode(void)
{
  ADCSRA = 0x0 ;

  // Set ADC prescalar to 128 - 125KHz sample rate @ 16MHz
  // No MUX values needed to be changed to use ADC0
  SetBit ( ADCSRA , ADPS2 ) ;
  SetBit ( ADCSRA , ADPS1 ) ;
  SetBit ( ADCSRA , ADPS0 ) ;

  ADMUX= 0x0; // control of multiplexir
  
  // reference voltage
# ifdef INTERNAL_VCCREF
  SetBit ( ADMUX , REFS0 ) ;
  SetBit ( ADMUX , REFS1 ) ;
# endif
  
# ifdef EXTERNAL_VCCREF
  SetBit ( ADMUX , REFS0 ) ;
  ClearBit ( ADMUX , REFS1 ) ;
# endif  

  // disable left read
  ClearBit ( ADMUX , ADLAR ) ;

  // disable digital input
  DIDR0 = _BV(ADC5D) | _BV(ADC4D) | _BV(ADC3D) | _BV(ADC2D) | _BV(ADC1D) | _BV(ADC0D) ;

  ADCSRA |= _BV(ADEN);  // Enable ADC
}

uint16_t adc_single_read ( const uint8_t ch )
{
  ADMUX = (ADMUX & 0b11111000) | (ch & 0b00000111) ;	// channel

  SetBit ( ADCSRA , ADSC ) ;    // start conversion
  while ( BitIsSet(ADCSRA,ADSC) ) ; // wait for conversion

  return ADC ;
}

uint16_t adc_filter_read ( const uint8_t ch )
{
    uint8_t i ;
    uint32_t sum ;

    for( i=0 , sum = 0 ; i<32; i++)
    {
        sum += adc_single_read(ch);
    }

    return (uint16_t)((sum>>5));
}
