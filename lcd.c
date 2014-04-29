# include <stdlib.h>
# include <string.h>
# include "common.h"
# include "lcd.h"
static char    tmpstr [LCD_TEXTLINE_WIDTH+1] ;

void lcd_init (void )
{
	lcdPCD8544_init ();
}

void lcd_update (void )
{
	lcdPCD8544_send_buffer ();
}

void lcd_cls (void )
{
    lcdPCD8544_cls ();
}

void lcd_print (const char * str , const uint8_t x , const uint8_t y )
{
	lcdPCD8544_print ( str , x , y ) ;
}

char * lcd_tmpstring (void )
{
	return tmpstr ;
}

# if 0
void lcd_testprint0 (void )
{
    lcd_print ( "XXXXXXXXXXXXXXXX" , 0 , 0 ) ;
    lcd_print ( "XXXXXXXXXXXXXXXX" , 0 , 1 ) ;    
    lcd_print ( "XXXXXXXXXXXXXXXX" , 0 , 2 ) ;    
    lcd_print ( "XXXXXXXXXXXXXXXX" , 0 , 3 ) ;        
    lcd_print ( "XXXXXXXXXXXXXXXX" , 0 , 4 ) ;        
    lcd_print ( "XXXXXXXXXXXXXXXX" , 0 , 5 ) ;            
    lcd_print ( "abc123def456ghi7" , 0 , 0 ) ;
    lcd_print ( "abc123def456ghi7" , 1 , 1 ) ;
    lcd_print ( "abc123def456ghi7" , 2 , 2 ) ;
    lcd_print ( "abc123def456ghi7" , 15 , 3 ) ;
    lcd_print ( "abc123def456ghi7" , 18 , 5 ) ;

    lcd_update ();  
}
# endif
