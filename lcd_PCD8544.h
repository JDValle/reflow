# ifndef REFLOW_LCDPCD8544_INCLUDE
# define REFLOW_LCDPCD8544_INCLUDE

# include <stdint.h>

# define LCDPCD8544_WIDTH 				84
# define LCDPCD8544_HEIGHT 				48
# define LCDPCD8544_ROWHEIGHT 			8

# define LCDPCD8544_TEXTLINE_WIDTH 		16
# define LCDPCD8544_NTEXTLINES			LCDPCD8544_HEIGHT/LCDPCD8544_ROWHEIGHT

# define LCDPCD8544_CMD			0
# define LCDPCD8544_DATA		1

# define PCD8544_POWERDOWN		0x04
# define PCD8544_ENTRYMODE		0x02
# define PCD8544_EXTENDEDINSTRUCTION 0x01

# define PCD8544_DISPLAYBLANK	0x0
# define PCD8544_DISPLAYNORMAL	0x4
# define PCD8544_DISPLAYALLON	0x1
# define PCD8544_DISPLAYINVERTED 0x5

// H = 0
# define PCD8544_FUNCTIONSET		0x20
# define PCD8544_DISPLAYCONTROL	0x08
# define PCD8544_SETYADDR		0x40
# define PCD8544_SETXADDR		0x80

// H = 1
# define PCD8544_SETTEMP			0x04
# define PCD8544_SETBIAS			0x10
# define PCD8544_SETVOP			0x80

extern void lcdPCD8544_init (void ) ;
extern void lcdPCD8544_send_bytes ( const uint8_t * src , const uint16_t size , const uint8_t dc ) ;
extern void lcdPCD8544_send_byte (const uint8_t data , const uint8_t dc) ;
extern void lcdPCD8544_send_buffer () ;

extern void lcdPCD8544_cls (void ) ;
extern void lcdPCD8544_print ( const char * str , const uint8_t x , const uint8_t y) ;
extern void lcdPCD8544_changepixel (const uint8_t xpixel , const uint8_t ypixel , const uint8_t setclear ) ;

# endif // REFLOW_LCDPCD8544_INCLUDE