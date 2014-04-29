# include <string.h>
# include <avr/pgmspace.h>
# include "common.h"
# include "lcd.h"
# include "encoder.h"
# include "menu.h"
# include "heater.h"

# define MENU_IDLE_TICKS		5L	*1000L*125L
# define MENU_MAXITEMSPERMENU	16
# define MENU_MAXPAGES			8

// NITEMS , PARENT , (ITEM VALUE,ITEM STRING,ITEM ACTION, ITEM ACTION PARAM) ...

typedef struct
{
	uint8_t	value;
	uint8_t	label;
	uint8_t action;
	uint8_t actionparam;
} menuitem_t ;

# define MENUPAGESIZE(n)		(2+(sizeof(menuitem_t)*(n)))	// calculate total size from nitems
# define MENUPAGENITEMS(n)		((n-2)/sizeof(menuitem_t) )		// from size , get the nitems
# define MENUPAGEITEM(n)		(2+(sizeof(menuitem_t)*(n)))	// offset to the item value

static uint8_t tmpmenupage [MENUPAGESIZE(MENU_MAXITEMSPERMENU)];
static uint8_t tmpmenupage_nitems[MENU_MAXPAGES];

static uint8_t menuaction;
static uint8_t menupage_cursor ;
static uint8_t menupage_base ;
static uint8_t menupage_current;
static uint32_t menuidleticks ;
static uint8_t menuidle ;


//                                "                "
const char mstrglob0 [] PROGMEM = ".." ;
# define MSTRGLOB 		mstrglob0

const char mstrmain0 [] PROGMEM = "START" ;
const char mstrmain1 [] PROGMEM = "STOP" ;
const char mstrmain2 [] PROGMEM = "CONF PREHEAT" ;
const char mstrmain3 [] PROGMEM = "CONF REFLOW" ;
const char mstrmain4 [] PROGMEM = "CONF COOLDOWN" ;
const char mstrmain5 [] PROGMEM = "LOAD PRESET 1" ;
const char mstrmain6 [] PROGMEM = "LOAD PRESET 2" ;
const char mstrmain7 [] PROGMEM = "LOAD PRESET 3" ;
const char mstrmain8 [] PROGMEM = "SAVE PRESET 1" ;
const char mstrmain9 [] PROGMEM = "SAVE PRESET 2" ;
const char mstrmain10[] PROGMEM = "SAVE PRESET 3" ;
const char mstrmain11[] PROGMEM = "SETTINGS" ;
const char mstrmain12[] PROGMEM = "ABOUT" ;
# define MSTRMAIN	mstrmain0,mstrmain1, mstrmain2, mstrmain3,mstrmain4,mstrmain5,mstrmain6,\
					mstrmain7,mstrmain8,mstrmain9,mstrmain10,mstrmain11,mstrmain12

const char mstrcfg0 [] PROGMEM = "set min temp" ;
const char mstrcfg1 [] PROGMEM = "set max temp" ;
const char mstrcfg2 [] PROGMEM = "set time ramp" ;
const char mstrcfg3 [] PROGMEM = "set time maxtemp" ;
# define MSTRCFG0	mstrcfg0,mstrcfg1, mstrcfg2, mstrcfg3

const char * const menustrings[] PROGMEM = { MSTRGLOB , MSTRMAIN , MSTRCFG0 };

# define MENU_ACTION_IDLE 				0x00
# define MENU_ACTION_NOTIMPLEMENTED		0x01
# define MENU_ACTION_DISPLAYMENU		0x02
# define MENU_ACTION_DISPLAYMENU		0x02
# define MENU_ACTION_DISPLAYMENU		0x02

# define MENU_ACTION_START				0x11
# define MENU_ACTION_STOP				0x12
# define MENU_ACTION_ABOUT				0x13

# define MENUITEM_GLOB_PARENT			0
# define MENUITEM_MAIN_START			1
# define MENUITEM_MAIN_STOP				2
# define MENUITEM_MAIN_CONFPREHEAT		3
# define MENUITEM_MAIN_CONFREFLOW		4
# define MENUITEM_MAIN_CONFCOOLDOWN		5
# define MENUITEM_MAIN_LOADPRESET1		6
# define MENUITEM_MAIN_LOADPRESET2		7
# define MENUITEM_MAIN_LOADPRESET3		8
# define MENUITEM_MAIN_SAVEPRESET1		9
# define MENUITEM_MAIN_SAVEPRESET2		10
# define MENUITEM_MAIN_SAVEPRESET3		11
# define MENUITEM_MAIN_SETTINGS			12
# define MENUITEM_MAIN_ABOUT			13

const uint8_t menu_main [] PROGMEM = { MENUPAGESIZE(13) , -1
										, MENUITEM_MAIN_START			, 1     , MENU_ACTION_START          , -1
										, MENUITEM_MAIN_STOP			, 2     , MENU_ACTION_STOP           , -1
										, MENUITEM_MAIN_CONFPREHEAT		, 3     , MENU_ACTION_NOTIMPLEMENTED , -1
										, MENUITEM_MAIN_CONFREFLOW		, 4     , MENU_ACTION_NOTIMPLEMENTED , -1
										, MENUITEM_MAIN_CONFCOOLDOWN	, 5     , MENU_ACTION_NOTIMPLEMENTED , -1
										, MENUITEM_MAIN_LOADPRESET1		, 6     , MENU_ACTION_NOTIMPLEMENTED , -1
										, MENUITEM_MAIN_LOADPRESET2		, 7     , MENU_ACTION_NOTIMPLEMENTED , -1
										, MENUITEM_MAIN_LOADPRESET3		, 8     , MENU_ACTION_NOTIMPLEMENTED , -1
										, MENUITEM_MAIN_SAVEPRESET1		, 9     , MENU_ACTION_NOTIMPLEMENTED , -1
										, MENUITEM_MAIN_SAVEPRESET2		, 10    , MENU_ACTION_NOTIMPLEMENTED , -1
										, MENUITEM_MAIN_SAVEPRESET3		, 11    , MENU_ACTION_NOTIMPLEMENTED , -1
										, MENUITEM_MAIN_SETTINGS		, 12    , MENU_ACTION_NOTIMPLEMENTED , -1
										, MENUITEM_MAIN_ABOUT			, 13    , MENU_ACTION_ABOUT          , -1
									} ;

const uint8_t * const menupages[] PROGMEM = { menu_main } ;
# define MENUPAGES_NPAGES	1

void menuaction_set (const uint8_t action)
{
	menuaction = action ;
	encoder_click_reset ();
}

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

	menuaction_set (MENU_ACTION_DISPLAYMENU);
}

# define MENUITEM(idx)			(( const menuitem_t *) (tmpmenupage + MENUPAGEITEM(idx)))

void menu_displaymenu ()
{
	uint8_t i , current;

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

void menu_action_displaymenu (void )
{
	// load menu
	menu_loadpage (tmpmenupage , menupage_current);

	menu_displaymenu ();

	// read input
	const uint8_t click = encoder_click_read();
	if (click != ENCODER_AFTERCLICK) return ;
	
	// if we have pending clicks, switch action
	const menuitem_t * item = MENUITEM(menupage_cursor) ;
	menuaction_set (item->action);
}

void menu_action_about (void )
{
	lcd_print ("derethor"   , 0 , 0 ) ;
	lcd_print ("@gmail.com" , 0 , 1 ) ;

	const uint8_t click = encoder_click_read();
	if (click != ENCODER_AFTERCLICK) return ;

	menuaction_set (MENU_ACTION_DISPLAYMENU);
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
	}

	// dont write code here, actions will leave the function before
}

void menu_init (void )
{
	lcd_init ();

	menuaction_set(MENU_ACTION_DISPLAYMENU);

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

void menuproc (void )
{
	// if we dont do anything, go to idle
	if (menuidle) menuaction_set(MENU_ACTION_IDLE);

	// run menu
	switch (menuaction)
	{
		case MENU_ACTION_IDLE 			: menu_action_heater (); break;
		case MENU_ACTION_DISPLAYMENU 	: menu_action_displaymenu (); break;		
		case MENU_ACTION_START 			: { heater_start (); menuaction_set(MENU_ACTION_IDLE); } break;
		case MENU_ACTION_STOP 			: { heater_stop  (); menuaction_set(MENU_ACTION_IDLE); } break;
		case MENU_ACTION_ABOUT 			: menu_action_about (); break;
		default: menu_action_heater();
	}
	lcd_update () ;
}
