# include <stdio.h>
# include <string.h>
# include <avr/pgmspace.h>
# include "common.h"
# include "timer.h"
# include "lcd.h"
# include "encoder.h"
# include "menu.h"
# include "heater.h"

void menuset (const uint8_t action , const uint8_t param ) ;


# define MENU_IDLE_TICKS		5 * 1000
# define MENU_MAXITEMSPERMENU	16
# define MENU_MAXPAGES			8

// NITEMS , PARENT , (ITEM VALUE,ITEM STRING,ITEM ACTION, ITEM ACTION PARAM) ...

typedef struct
{
	uint8_t	value;
	uint8_t	label;
	uint8_t action;
	uint8_t param;
} menuitem_t ;

# define MENUPAGESIZE(n)		(2+(sizeof(menuitem_t)*(n)))	// calculate total size from nitems
# define MENUPAGENITEMS(n)		((n-2)/sizeof(menuitem_t) )		// from size , get the nitems
# define MENUPAGEITEM(n)		(2+(sizeof(menuitem_t)*(n)))	// offset to the item value

static uint8_t tmpmenupage [MENUPAGESIZE(MENU_MAXITEMSPERMENU)];
static uint8_t tmpmenupage_nitems[MENU_MAXPAGES];

static uint8_t menuaction ;
static uint8_t menuactionparam ;

static uint8_t menupage_cursor ;
static uint8_t menupage_base ;
static uint8_t menupage_current;
static uint32_t menuidleticks ;
static uint8_t menuidle ;

static int16_t inputvalue, inputmin, inputmax ;

//                                "                "
const char mstr00 [] PROGMEM = "SET TEMP" ;
const char mstr01 [] PROGMEM = "SET FAN" ;
const char mstr02 [] PROGMEM = "RUN PROGRAM" ;
const char mstr03 [] PROGMEM = "STOP PROGRAM" ;
const char mstr04 [] PROGMEM = "ABOUT" ;

# define MSTRMAIN	mstr00,mstr01,mstr02,mstr03,mstr04

const char * const menustrings[] PROGMEM = { MSTRMAIN } ;

# define MENU_ACTION_IDLE 				0x00
# define MENU_ACTION_NOTIMPLEMENTED		0x01
# define MENU_ACTION_DISPLAYMENU		0x02

# define MENU_ACTION_SETTEMP			0x11
# define MENU_ACTION_SETFAN				0x12
# define MENU_ACTION_RUN				0x13
# define MENU_ACTION_STOP				0x14
# define MENU_ACTION_ABOUT				0x15

# define MENUITEM_MAIN_SETTEMP			0
# define MENUITEM_MAIN_SETFAN			1
# define MENUITEM_MAIN_RUN				2
# define MENUITEM_MAIN_STOP				3
# define MENUITEM_MAIN_ABOUT			4

const uint8_t menu_main [] PROGMEM = { MENUPAGESIZE(5) , -1
// ITEM ID , STRING INDEX , ACTION INDEX , ACTION PARAM
										, MENUITEM_MAIN_SETTEMP			, 0    , MENU_ACTION_SETTEMP        , -1
										, MENUITEM_MAIN_SETFAN			, 1    , MENU_ACTION_SETFAN         , -1
										, MENUITEM_MAIN_RUN				, 2    , MENU_ACTION_RUN            , -1
										, MENUITEM_MAIN_STOP			, 3    , MENU_ACTION_STOP           , -1
										, MENUITEM_MAIN_ABOUT			, 4    , MENU_ACTION_ABOUT          , -1
									} ;

const uint8_t * const menupages[] PROGMEM = { menu_main } ;
# define MENUPAGES_NPAGES	1
# define MENU_MAIN		0

void menu_loadstr (char * dst , const uint8_t i)
{
	strcpy_P ( dst , (PGM_P)pgm_read_word(&(menustrings[i])) ) ;
}

void menu_loadpage (uint8_t * dst , const uint8_t i)
{
	uint8_t pagesize;
	const char * page =(PGM_P)pgm_read_word(&(menupages[i])) ;
	memcpy_P ( &pagesize , page , sizeof(uint8_t) ) ;
	memcpy_P ( dst , page , pagesize ) ;
}

void menu_action_heater (void )
{
	heater_display () ;

	const uint8_t click = encoder_click_read();
	if (click != ENCODER_AFTERCLICK) return ;
	// change action

	menupage_cursor = 0 ;
	menupage_base = 0 ;
	menupage_current = 0 ;
	menuidleticks = 0 ;
	menuidle = 0 ;

	menuset (MENU_ACTION_DISPLAYMENU , MENU_MAIN ) ;
}

# define MENUITEM(idx)			(( const menuitem_t *) (tmpmenupage + MENUPAGEITEM(idx)))

void menu_displaymenu ()
{
	uint8_t i , current;

	lcd_cls () ;

	const uint8_t nitems = MENUPAGENITEMS(tmpmenupage[0]);
	current = menupage_base ;
	char * tmpmenustring = lcd_tmpstring() ;

	for (i=0;i<LCD_NTEXTLINES;++i)
	{
		memset ( lcd_tmpstring () , ' ' , LCD_TEXTLINE_WIDTH ) ;

		if (current<nitems)
		{
			const menuitem_t * item = MENUITEM(current) ;

			menu_loadstr (tmpmenustring+1, item->label );
			tmpmenustring[strlen(tmpmenustring+1)+1] = ' ' ;
			if (current == menupage_cursor)
			{
				tmpmenustring[0] = '>';
			}
			else
			{
				tmpmenustring[0] = ' ';
			}
			lcd_print ( tmpmenustring     , 0 , i ) ;
		}

		++ current ;
	}
}

void menu_action_displaymenu (const uint8_t menu )
{
	// load menu
	menu_loadpage (tmpmenupage , menu) ;

	menu_displaymenu () ;

	// read input
	const uint8_t click = encoder_click_read();
	if (click != ENCODER_AFTERCLICK) return ;
	
	// if we have pending clicks, switch action
	const menuitem_t * item = MENUITEM(menupage_cursor) ;
	menuset (item->action , item->param) ;
}

void menu_action_settemp (void )
{
	lcd_cls ();
	lcd_print ("TEMP :"   , 0 , 0 ) ;

	timer_cs_start ();
	const int16_t temp = inputvalue ;
	timer_cs_end ();

	char * dst = lcd_tmpstring();
	sprintf ( dst , "%3d" , temp ) ;
	lcd_print (dst, 7 , 0 ) ;

	// read input
	const uint8_t click = encoder_click_read();
	if (click != ENCODER_AFTERCLICK) return ;
	
	heater_settemp (temp) ;

	// if we have pending clicks, switch action
	menuset (MENU_ACTION_IDLE , 0 ) ;
}

void menu_action_setfan (void )
{
	lcd_cls ();
	lcd_print ("FAN  :"   , 0 , 0 ) ;

	timer_cs_start ();
	const int16_t fan = inputvalue ;
	timer_cs_end ();

	char * dst = lcd_tmpstring();
	sprintf ( dst , "%3d" , fan ) ;
	lcd_print (dst, 7 , 0 ) ;

	// read input
	const uint8_t click = encoder_click_read();
	if (click != ENCODER_AFTERCLICK) return ;
	
	heater_setfan (fan) ;

	// if we have pending clicks, switch action
	menuset (MENU_ACTION_IDLE , 0 ) ;
}


void menu_action_about (void )
{
	lcd_cls ();
	lcd_print ("REFLOW CTRL  "   , 0 , 0 ) ;
	lcd_print ("@staticboards"   , 0 , 1 ) ;

	const uint8_t click = encoder_click_read();
	if (click != ENCODER_AFTERCLICK) return ;

	menuset (MENU_ACTION_DISPLAYMENU , MENU_MAIN ) ;
}

////////////////////////////////////////////////////////////////////////

void menu_update (void )
{
	// update cursor
	const uint8_t changed = encoder_update ();

	// check for idle, use a 8bit var for atomic read later
	if (!changed)
	{
		if (menuidleticks < MENU_IDLE_TICKS ) ++ menuidleticks ;
		else menuidle = 1;
	}
	else
	{
		menuidleticks = 0 ;
		menuidle = 0 ;
	}

	switch (menuaction)
	{
		case MENU_ACTION_IDLE :
		{
			const int8_t diff = encoder_increment ();
			if (diff==0) return ;

			heater_menu_update (diff);
		}

		case MENU_ACTION_DISPLAYMENU :
		{
			const int8_t diff = encoder_increment ();
			if (diff==0) return ;

			// load menu nitems from cached copy (reading from flash is very slow on the timer)
			const uint8_t nitems = tmpmenupage_nitems[menupage_current];

			// calculate new cursor and menu page base
			menupage_cursor = MAX( 0 , MIN ( nitems - 1 , menupage_cursor + diff) ) ;

			const int8_t deltamenu = menupage_cursor - menupage_base ;

			if ( deltamenu>=LCD_NTEXTLINES)
			{
				menupage_base = MAX( 0 , MIN ( nitems - LCD_NTEXTLINES , menupage_base + ( deltamenu - LCD_NTEXTLINES + 1 ) ) ) ;
			}

			if ( deltamenu<0 )
			{
				menupage_base = MAX( 0 , MIN ( nitems - LCD_NTEXTLINES , menupage_base + deltamenu ) ) ;				
			}

		} break ;

		case MENU_ACTION_SETTEMP :
		case MENU_ACTION_SETFAN :
		{
			const int8_t diff = encoder_increment () ;
			if (diff==0) return ;
			inputvalue = MAX( inputmin , MIN ( inputmax , inputvalue + (diff*5) ) ) ;

		} break ;
	}

	// dont write code here, actions will leave the function before
}

void menu_init (void )
{
	lcd_init ();

	menuset(MENU_ACTION_IDLE , 0 );

	// cache page sizes to use them on the timer interrupt
	{
		uint8_t i;
		for (i=0;i<MENUPAGES_NPAGES;++i)
		{
			menu_loadpage (tmpmenupage , i);
			tmpmenupage_nitems [i] = MENUPAGENITEMS(tmpmenupage[0]);
		}	
	}
}

void menuset (const uint8_t action , const uint8_t param )
{
	menuaction = action ;
	menuactionparam = param ;
	encoder_click_reset () ;

	switch (action)
	{
		case MENU_ACTION_SETTEMP :
		{
			inputvalue = 100 ;
			inputmin = 0 ;
			inputmax = 350 ;
		} break ;

		case MENU_ACTION_SETFAN :
		{
			inputvalue = 0 ;
			inputmin = 0 ;
			inputmax = 127 ;
		} break ;

	}

}

void menuproc (void )
{
	// if we dont do anything, go to idle
	if (menuidle) menuset(MENU_ACTION_IDLE , 0 );

	// run menu
	switch (menuaction)
	{
		case MENU_ACTION_IDLE 			: menu_action_heater (); break;
		case MENU_ACTION_DISPLAYMENU 	: menu_action_displaymenu (menuactionparam); break;		
		case MENU_ACTION_SETTEMP 		: menu_action_settemp (); break;
		case MENU_ACTION_SETFAN 		: menu_action_setfan  (); break;
		case MENU_ACTION_RUN 			: { heater_run ();  menuset(MENU_ACTION_IDLE,0); } break;
		case MENU_ACTION_STOP 			: { heater_stop  (); menuset(MENU_ACTION_IDLE,0); }break;
		case MENU_ACTION_ABOUT 			: menu_action_about (); break;
		default: menu_action_heater();
	}
	lcd_update () ;
}
