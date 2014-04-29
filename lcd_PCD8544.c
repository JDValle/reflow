# include <avr/io.h>
# include <string.h>

# include "common.h"
# include "timer.h"
# include "lcd_PCD8544.h"

# define PINS_READ_PCD8544_RST		PIND
# define PINS_WRITE_PCD8544_RST		PORTD		
# define PINS_DDR_PCD8544_RST		DDRD
# define PINS_OFFSET_PCD8544_RST	0

# define PINS_READ_PCD8544_CE		PIND
# define PINS_WRITE_PCD8544_CE		PORTD
# define PINS_DDR_PCD8544_CE		DDRD
# define PINS_OFFSET_PCD8544_CE		1

# define PINS_READ_PCD8544_DC		PIND
# define PINS_WRITE_PCD8544_DC		PORTD
# define PINS_DDR_PCD8544_DC		DDRD
# define PINS_OFFSET_PCD8544_DC		2

# define PINS_READ_PCD8544_DIN		PINB
# define PINS_WRITE_PCD8544_DIN		PORTB
# define PINS_DDR_PCD8544_DIN		DDRB
# define PINS_OFFSET_PCD8544_DIN	3

# define PINS_READ_PCD8544_CLK		PINB
# define PINS_WRITE_PCD8544_CLK		PORTB
# define PINS_DDR_PCD8544_CLK		DDRB
# define PINS_OFFSET_PCD8544_CLK	5

# define LCDPCD8544_ROWS            (LCDPCD8544_HEIGHT/LCDPCD8544_ROWHEIGHT)
# define LCDPCD8544_CHARWIDTH       5
# define LCDPCD8544_CHARHEIGHT      LCDPCD8544_ROWHEIGHT

# define PCD8544_MEMSIZE			      (LCDPCD8544_WIDTH*LCDPCD8544_ROWS)

# define PCD8544_DIRTYMSK_ROWS    (LCDPCD8544_ROWS)
# define PCD8544_DIRTYMSK_ROWSIZE (ROUNDNEAR(LCDPCD8544_WIDTH,8)>>3)
# define PCD8544_DIRTYMSK_MEMSIZE (PCD8544_DIRTYMSK_ROWSIZE*LCDPCD8544_ROWS)

static uint8_t pcd8544_buffer[PCD8544_MEMSIZE] ;
static uint8_t pcd8544_dirtymask [ PCD8544_DIRTYMSK_MEMSIZE ] ;
static uint8_t pcd8544_tmpmask [ LCDPCD8544_WIDTH * 2 + 1  ] ;

static const uint8_t ASCII[][LCDPCD8544_CHARWIDTH] = {
  {0x00, 0x00, 0x00, 0x00, 0x00} // 20  
  ,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
  ,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
  ,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
  ,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
  ,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
  ,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
  ,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
  ,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
  ,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
  ,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
  ,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
  ,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
  ,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
  ,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
  ,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
  ,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
  ,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
  ,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
  ,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
  ,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
  ,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
  ,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
  ,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
  ,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
  ,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
  ,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
  ,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
  ,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
  ,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
  ,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
  ,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
  ,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
  ,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
  ,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
  ,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
  ,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
  ,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
  ,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
  ,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
  ,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
  ,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
  ,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
  ,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
  ,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
  ,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
  ,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
  ,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
  ,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
  ,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
  ,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
  ,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
  ,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
  ,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
  ,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
  ,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
  ,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
  ,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
  ,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
  ,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
  ,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c 'back slash'
  ,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
  ,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
  ,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
  ,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
  ,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
  ,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
  ,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
  ,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
  ,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
  ,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
  ,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
  ,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
  ,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
  ,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j 
  ,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
  ,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
  ,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
  ,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
  ,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
  ,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
  ,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
  ,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
  ,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
  ,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
  ,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
  ,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
  ,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
  ,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
  ,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
  ,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
  ,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
  ,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
  ,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
  ,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ~
  ,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f DEL
};

///// BUFFER ///////

void lcdPCD8544_clear (void )
{
  uint8_t row , col ;
  for (row = 0 ; row < LCDPCD8544_ROWS ; ++ row)
  {
    lcdPCD8544_send_byte ( PCD8544_SETYADDR | row , LCDPCD8544_CMD ) ;
    for ( col = 0 ; col < LCDPCD8544_WIDTH ; ++ col )
    {
      lcdPCD8544_send_byte ( PCD8544_SETXADDR | col , LCDPCD8544_CMD ) ;
      lcdPCD8544_send_byte ( 0x0 , LCDPCD8544_DATA ) ;
    }
  }

  lcdPCD8544_send_byte ( PCD8544_SETYADDR | 0 , LCDPCD8544_CMD ) ;
}

void lcdPCD8544_send_buffer (void )
{
  uint8_t row , x ;
  uint8_t bit , rdirty;
  uint8_t start, stop ;

  for ( row=0; row < LCDPCD8544_ROWS ; ++ row )
  {
    uint8_t *rowp   = pcd8544_buffer    + (row * LCDPCD8544_WIDTH ) ;
    uint8_t *dirtyp = pcd8544_dirtymask + (row * PCD8544_DIRTYMSK_ROWSIZE) ;
    uint8_t head   = 0 ;

    memset ( pcd8544_tmpmask , 0xff , sizeof(pcd8544_tmpmask) ) ;

    // generate tmp list
    for ( rdirty = 0 , x = 0 ; rdirty < PCD8544_DIRTYMSK_ROWSIZE ; ++ rdirty )
    {
      const uint8_t d = *dirtyp++ ;

      for(bit = 1<<7 ; bit; bit >>= 1 , ++ x)
      {
        if (x>=LCDPCD8544_WIDTH) continue ;
        if ( (bit & d) == 0 ) continue ;

        // if root, push a new dirty segment
        if (head==0)
        {
          pcd8544_tmpmask [head++] = x ; // push start
          pcd8544_tmpmask [head++] = x ; // push stop                
        }
        else
        {
          stop = pcd8544_tmpmask [head-1];
          // if we are on a dirty segment, grow
          if ((stop+1) == x)
          {
            pcd8544_tmpmask [head-1] = x ;  // update end
          }
          else
          // post a new dirty segment
          {
            pcd8544_tmpmask [head++] = x ; // push start
            pcd8544_tmpmask [head++] = x ; // push stop                
          }            
        }

      } // endfor
    }

    dirtyp = pcd8544_tmpmask ;
    start = *dirtyp++ ;
    stop  = *dirtyp++ ;

    if (start != 0xff) lcdPCD8544_send_byte ( PCD8544_SETYADDR | row , LCDPCD8544_CMD ) ;

    while ( start != 0xff )
    {
      const uint8_t len = (stop + 1) - start ;
      lcdPCD8544_send_byte ( PCD8544_SETXADDR | start , LCDPCD8544_CMD ) ;
      lcdPCD8544_send_bytes ( rowp + start , len , LCDPCD8544_DATA ) ;
      start = *dirtyp++ ;
      stop  = *dirtyp++ ;
    }

  } // end rows

  lcdPCD8544_send_byte ( PCD8544_SETYADDR | 0 , LCDPCD8544_CMD ) ;
  memset ( pcd8544_dirtymask , 0x00 , PCD8544_DIRTYMSK_MEMSIZE ) ;
}

void lcdPCD8544_cls (void )
{
  uint8_t row , x ;

  for (row = 0 ; row < LCDPCD8544_ROWS ; ++ row )
  {

    // get buffer position
    uint8_t *dst = pcd8544_buffer + (row*LCDPCD8544_WIDTH);
    uint8_t *dirtyp = pcd8544_dirtymask + (row * PCD8544_DIRTYMSK_ROWSIZE) ;

    for (x=0;x<LCDPCD8544_WIDTH;++x)
    {
      const uint8_t prev = *dst ;
      // copy only if different
      if (prev!=0x0)
      {          
        *dst = 0x0 ;
        // mark pixel as dirty
        SetBit ( dirtyp [x/8] , (7 - x%8) ) ;
      }
      ++ dst ;      
    } // end for x

  } // end for row
}

void lcdPCD8544_print ( const char * str , const uint8_t x , const uint8_t y)
{
  uint8_t i , j ;

  // skip lines out of the screen
  if (y>=LCDPCD8544_NTEXTLINES) return ;
  if (x>=LCDPCD8544_TEXTLINE_WIDTH) return ;

  // get pixel position
  uint8_t xpixel = x*LCDPCD8544_CHARWIDTH ;
  const uint8_t ypixel = ((y*LCDPCD8544_CHARHEIGHT)) / LCDPCD8544_ROWHEIGHT ;

  // get buffer position
  uint8_t *dst = pcd8544_buffer + (ypixel*LCDPCD8544_WIDTH) + xpixel ;
  uint8_t *dirtyp = pcd8544_dirtymask + (y * PCD8544_DIRTYMSK_ROWSIZE) ;

  // calculate maxlen
  const uint8_t len = MIN ( strlen (str) , MAX(0,LCDPCD8544_TEXTLINE_WIDTH - x) ) ;

  for (i=0;i<len;++i)
  {
    // validate char
    const uint8_t c = *str++;
    if (c < 0x20 || c >0x7f ) continue ;

    const uint8_t * bitmap = ASCII[c-0x20];

    // copy bitmap
    for (j=0;j<LCDPCD8544_CHARWIDTH;++j)
    {
      const uint8_t prev = *dst ;
      const uint8_t src  = *bitmap ;
      // copy only if different
      if (prev!=src)
      {          
        *dst = src ;
        // mark pixel as dirty
        SetBit ( dirtyp [xpixel/8] , (7 - xpixel%8) ) ;
      }
      ++ dst ; ++ bitmap ; ++ xpixel ;      
    } // end for copy bitmap

  } // end for string
}

///// IO ///////

inline void lcdPCD8544_spi_byte (const uint8_t data)
{
	uint8_t bit ;
	for(bit = 1<<7 ; bit; bit >>= 1)
	{
		ClearBit ( PINS_WRITE_PCD8544_CLK , PINS_OFFSET_PCD8544_CLK ) ;

		if(data & bit)	SetBit   ( PINS_WRITE_PCD8544_DIN , PINS_OFFSET_PCD8544_DIN) ;
		else			ClearBit ( PINS_WRITE_PCD8544_DIN , PINS_OFFSET_PCD8544_DIN) ;

		SetBit ( PINS_WRITE_PCD8544_CLK , PINS_OFFSET_PCD8544_CLK ) ;
	}	
}

void lcdPCD8544_send_byte (const uint8_t data , const uint8_t dc)
{
	// SETUP DATA/COMMAND
	if (dc)	SetBit     ( PINS_WRITE_PCD8544_DC  , PINS_OFFSET_PCD8544_DC ) ;
	else	ClearBit   ( PINS_WRITE_PCD8544_DC  , PINS_OFFSET_PCD8544_DC ) ;

	// LOW SIGNAL START
	ClearBit ( PINS_WRITE_PCD8544_CE  , PINS_OFFSET_PCD8544_CE ) ;

	// stream byte
	lcdPCD8544_spi_byte (data);

	// HIGH SIGNAL START
	SetBit   ( PINS_WRITE_PCD8544_CE  , PINS_OFFSET_PCD8544_CE ) ;
}

void lcdPCD8544_send_bytes ( const uint8_t * src , const uint16_t size , const uint8_t dc )
{
	// SETUP DATA/COMMAND
	if (dc)	SetBit     ( PINS_WRITE_PCD8544_DC  , PINS_OFFSET_PCD8544_DC ) ;
	else	ClearBit   ( PINS_WRITE_PCD8544_DC  , PINS_OFFSET_PCD8544_DC ) ;

	// LOW SIGNAL START
	ClearBit ( PINS_WRITE_PCD8544_CE  , PINS_OFFSET_PCD8544_CE ) ;

	// SEND SPI DATA FAST
	uint8_t l = MIN (size , PCD8544_MEMSIZE ) ;

	while (l--) lcdPCD8544_spi_byte (*src++) ;

	// HIGH SIGNAL START
	SetBit   ( PINS_WRITE_PCD8544_CE  , PINS_OFFSET_PCD8544_CE ) ;
}

void lcdPCD8544_init (void )
{
	// setup io
	SetBit ( PINS_DDR_PCD8544_RST  , PINS_OFFSET_PCD8544_RST ) ;
	SetBit ( PINS_DDR_PCD8544_CE   , PINS_OFFSET_PCD8544_CE  ) ;
	SetBit ( PINS_DDR_PCD8544_DC   , PINS_OFFSET_PCD8544_DC  ) ;
	SetBit ( PINS_DDR_PCD8544_DIN  , PINS_OFFSET_PCD8544_DIN ) ;
	SetBit ( PINS_DDR_PCD8544_CLK  , PINS_OFFSET_PCD8544_CLK ) ;

	// RESET
	ClearBit ( PINS_WRITE_PCD8544_CE  , PINS_OFFSET_PCD8544_CE ) ;
	timer_wait_ms (5) ;
	ClearBit ( PINS_WRITE_PCD8544_RST  , PINS_OFFSET_PCD8544_RST ) ;
	asm volatile("nop");
	asm volatile("nop");
	SetBit   ( PINS_WRITE_PCD8544_RST  , PINS_OFFSET_PCD8544_RST ) ;

	// Enable clock
	SetBit ( PINS_WRITE_PCD8544_CE  , PINS_OFFSET_PCD8544_CE ) ;
	SetBit ( PINS_WRITE_PCD8544_CLK , PINS_OFFSET_PCD8544_CLK ) ;

	lcdPCD8544_send_byte ( PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION , LCDPCD8544_CMD ) ;
	timer_wait_ms (5) ;
	lcdPCD8544_send_byte ( PCD8544_SETVOP | 0x38 , LCDPCD8544_CMD ) ;
	timer_wait_ms (5) ;
	lcdPCD8544_send_byte ( PCD8544_SETTEMP , LCDPCD8544_CMD ) ;
	timer_wait_ms (5) ;
	lcdPCD8544_send_byte ( PCD8544_SETBIAS | 0x04 , LCDPCD8544_CMD ) ;
	timer_wait_ms (5) ;
	lcdPCD8544_send_byte ( PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL , LCDPCD8544_CMD ) ;
	timer_wait_ms (5) ;
	lcdPCD8544_send_byte ( PCD8544_FUNCTIONSET | 0 , LCDPCD8544_CMD ) ;
	timer_wait_ms (5) ;
	lcdPCD8544_send_byte ( PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL , LCDPCD8544_CMD ) ;
	timer_wait_ms (5) ;

  memset ( pcd8544_buffer , 0x0 , PCD8544_MEMSIZE ) ;
  memset ( pcd8544_dirtymask , 0xff , PCD8544_DIRTYMSK_MEMSIZE ) ;

	lcdPCD8544_send_buffer () ;
}
