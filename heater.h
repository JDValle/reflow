# ifndef REFLOW_HEATER_INCLUDE
# define REFLOW_HEATER_INCLUDE

# define HEATER_STATUS_IDLE     0
# define HEATER_STATUS_RUNNING  1
# define HEATER_STATUS_READY    2

# define HEATER_STAGE_PREHEATER_START 0
# define HEATER_STAGE_PREHEATER_KEEP  1
# define HEATER_STAGE_REFLOW_START    2
# define HEATER_STAGE_REFLOW_KEEP     3
# define HEATER_STAGE_COOLDOWN        4
# define HEATER_STAGE_READY           5

# define HEATER_NSTAGES               5

extern void heater_update (void ) ;
extern void heaterproc (void ) ;
extern void heater_init (void ) ;

extern void heater_start (void ) ;
extern void heater_stop (void ) ;

extern void heater_display (void ) ;

# endif // REFLOW_HEATER_INCLUDE