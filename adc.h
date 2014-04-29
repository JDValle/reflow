# ifndef REFLOW_ADC_INCLUDE
# define REFLOW_ADC_INCLUDE

# include <stdint.h>

extern uint16_t adc_single_read ( const uint8_t ch ) ;
extern void adc_init_singlemode(void) ;

# endif // REFLOW_ADC_INCLUDE