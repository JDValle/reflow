# include <stdlib.h>
# include <avr/io.h>
# include "encoder.h"
# include "common.h"

# define PINS_READ_ENC			PINB
# define PINS_WRITE_ENC			PORTB
# define PINS_DDR_ENC			DDRB
# define PINS_OFFSET_ENC_A		1
# define PINS_OFFSET_ENC_B		0
# define PINS_OFFSET_ENC_BTN	2

static volatile uint8_t enc_value;
static volatile uint8_t enc_tests;
static volatile uint8_t enc_index;
static volatile uint8_t enc_previ;
static uint8_t enc_prevc;

# define ENC_STEPS			16

# define ENCODER_BTN(e) 		(BitIsSet ((e),6))
# define ENCODER_POS(e)			( (e) & 0x0f )
# define ENCODER_CLK(e)			( (e) & 0x30 )
# define ENCODER_DIR(e)			(BitIsSet ((e),7))

# define ENC_INDEX_INC		(enc_index = 0<<7 | ENCODER_CLK(enc_index) | ( ENCODER_POS(enc_index) +            1  ) % ENC_STEPS)
# define ENC_INDEX_DEC		(enc_index = 1<<7 | ENCODER_CLK(enc_index) | ( ENCODER_POS(enc_index) + (ENC_STEPS-1) ) % ENC_STEPS)

# define ENC_READ_AB		(read & 0x03)
# define ENC_READ_BTN		(read & 1<<2)

# define ENC_CLICK_READ		((enc_index & 0x30))
# define ENC_CLICK_WRITE(n)	((enc_index & ~0x30) | (n))

uint8_t encoder_update (void )
{
	// sure both reads same time
	const uint8_t input = PINS_READ_ENC;
	const uint8_t read = ( BitIsSet ( input , PINS_OFFSET_ENC_BTN ) << 2 ) | ( BitIsSet ( input , PINS_OFFSET_ENC_B ) << 1 ) | ( BitIsSet ( input , PINS_OFFSET_ENC_A ) << 0 ) ;

	if ( read != enc_value )
	{
		enc_tests = 2;		// 3ms
		enc_value = read;
		return 0;
	}

	if (enc_tests > 0)
	{
		-- enc_tests ;
		return 0;
	}

	if ( enc_tests == 0 )
	{
		const uint8_t before_enc_index = enc_index;

		if ( ENC_READ_AB == 0)
		{
		     if ( enc_previ == 1 ) ENC_INDEX_INC ;
		else if ( enc_previ == 2 ) ENC_INDEX_DEC ;
		}
 		enc_previ = ENC_READ_AB ;

 		if ( ENC_READ_BTN )
 		{
 			ClearBit (enc_index,6);

 			// on click -> after click
	 		if ( ENC_CLICK_READ == ENCODER_ONCLICKCLICK )
	 		{
	 			enc_index = ENC_CLICK_WRITE(ENCODER_AFTERCLICK);
	 		}
 		}
 		else
 		{
 			SetBit (enc_index,6);

 			// before click -> on click
	 		if ( ENC_CLICK_READ == ENCODER_BEFORECLICK )
	 		{
	 			enc_index = ENC_CLICK_WRITE(ENCODER_ONCLICKCLICK);
	 		}

 		}

 		return before_enc_index != enc_index ;
	}

	return 0;
}

void encoder_init (void )
{
	ClearBit ( PINS_DDR_ENC , PINS_OFFSET_ENC_A );
	ClearBit ( PINS_DDR_ENC , PINS_OFFSET_ENC_B );
	ClearBit ( PINS_DDR_ENC , PINS_OFFSET_ENC_BTN );

	SetBit ( PINS_WRITE_ENC , PINS_OFFSET_ENC_A );
	SetBit ( PINS_WRITE_ENC , PINS_OFFSET_ENC_B );
	SetBit ( PINS_WRITE_ENC , PINS_OFFSET_ENC_BTN );

	enc_index = 0 ;
	enc_tests = 0 ;
	enc_value = 0 ;
	enc_previ = 0 ;
	enc_prevc = 0 ;

	encoder_update ();
}

int8_t encoder_increment (void )
{
	const uint8_t enc = enc_index;

	if ( ENCODER_POS(enc) == ENCODER_POS(enc_prevc) ) return 0 ;

	const uint8_t minpos = MIN(ENCODER_POS(enc) , ENCODER_POS(enc_prevc) );
	const uint8_t maxpos = MAX(ENCODER_POS(enc) , ENCODER_POS(enc_prevc) );
	
	enc_prevc = enc;

	const uint8_t distance = maxpos - minpos;

	if ( distance <= (ENC_STEPS>>1) )
	{
		return (int8_t) ( ENCODER_DIR(enc) ? ~(distance-1) : distance ) ;
	}
	else
	{
		const uint8_t r = ( minpos + ENC_STEPS ) - maxpos ;
		return (int8_t) ( ENCODER_DIR(enc) ? ~(r-1) : r ) ;
	}
}

uint8_t encoder_click_read (void )
{
	return ENC_CLICK_READ ;
}

void encoder_click_reset (void )
{
	enc_index = ENC_CLICK_WRITE(ENCODER_BEFORECLICK);
}