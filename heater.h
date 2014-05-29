# ifndef REFLOW_HEATER_INCLUDE
# define REFLOW_HEATER_INCLUDE

# define HEATER_STATUS_IDLE     0
# define HEATER_STATUS_RUNNING  1

# define HEATER_STAGE_PREHEATER_NONE	0
# define HEATER_STAGE_PREHEATER_START	1
# define HEATER_STAGE_PREHEATER_KEEP	2
# define HEATER_STAGE_REFLOW_START		3
# define HEATER_STAGE_REFLOW_KEEP		4
# define HEATER_STAGE_COOLDOWN			5
# define HEATER_STAGE_READY				6

# define HEATER_NSTAGES					HEATER_STAGE_READY+1

extern void heater_update (void ) ;
extern void heater_menu_update (const int8_t diff) ;
extern void heaterproc (void ) ;
extern void heater_init (void ) ;

extern void heater_settemp (const uint8_t temp) ;
extern void heater_run (void ) ;
extern void heater_stop (void ) ;

extern void heater_display (void ) ;

# endif // REFLOW_HEATER_INCLUDE