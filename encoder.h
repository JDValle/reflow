# ifndef REFLOW_ENCODER_INCLUDE
# define REFLOW_ENCODER_INCLUDE

# include <stdint.h>

extern void encoder_init   (void ) ;
extern uint8_t encoder_update (void ) ;
extern const uint8_t encoder(void) ;
extern int8_t encoder_increment (void );
extern uint8_t encoder_click_read (void ) ;
extern void encoder_click_reset (void ) ;

# define ENCODER_BEFORECLICK	0<<4
# define ENCODER_ONCLICKCLICK	1<<4
# define ENCODER_AFTERCLICK		3<<4

# endif // REFLOW_ENCODER_INCLUDE