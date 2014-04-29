# ifndef REFLOW_LCD_INCLUDE
# define REFLOW_LCD_INCLUDE

# include <stdint.h>

# include "lcd_PCD8544.h"
# define LCD_TEXTLINE_WIDTH		LCDPCD8544_TEXTLINE_WIDTH
# define LCD_NTEXTLINES			LCDPCD8544_NTEXTLINES
# define LCD_WIDTH 				LCDPCD8544_WIDTH
# define LCD_HEIGHT 			LCDPCD8544_HEIGHT
# define LCD_CHARWIDTH 			LCDPCD8544_CHARWIDTH
# define LCD_CHARHEIGHT 		LCDPCD8544_CHARHEIGHT

extern void lcd_init (void );
extern void lcd_update (void ) ;

extern void lcd_cls (void ) ;
extern void lcd_print (const char * str , const uint8_t x , const uint8_t y ) ;

// get a pointer to a temp string, dont waste mem
extern char * lcd_tmpstring (void ) ;

# endif // REFLOW_LCD_INCLUDE