# ifndef REFLOW_UART_INCLUDE
# define REFLOW_UART_INCLUDE

# include <stdint.h>

// # define UART_ENABLE

//----------------------------------------------------

# define UART_BAUD2UBRR(b)	((uint16_t)((F_CPU) / (16UL * (b)) - 1))

# define UART_UBRR_1200		UART_BAUD2UBRR(1200UL)
# define UART_UBRR_2400		UART_BAUD2UBRR(2400UL)
# define UART_UBRR_4800		UART_BAUD2UBRR(4800UL)
# define UART_UBRR_9600		UART_BAUD2UBRR(9600UL)
# define UART_UBRR_14400	UART_BAUD2UBRR(14400UL)
# define UART_UBRR_19200	UART_BAUD2UBRR(19200UL)
# define UART_UBRR_28800	UART_BAUD2UBRR(28800UL)
# define UART_UBRR_38400	UART_BAUD2UBRR(38400UL)
# define UART_UBRR_57600	UART_BAUD2UBRR(57600UL)
# define UART_UBRR_76800	UART_BAUD2UBRR(76800UL)
# define UART_UBRR_115200	UART_BAUD2UBRR(115200UL)
# define UART_UBRR_230400	UART_BAUD2UBRR(230400UL)

# define UART_UBRR_CURRENT	UART_UBRR_38400

//----------------------------------------------------------------

# define UART_RXBUFSIZE		64
# define UART_TXBUFSIZE		64

//----------------------------------------------------

# define UART_INIT_INTERRUPT_DISABLED		0<<1
# define UART_INIT_INTERRUPT_ENABLED		1<<1

//----------------------------------------------------------------

# define UART_TIMEOUTMS		100
# define UART_ERROR_OK		0
# define UART_ERROR_TIMEOUT	1

//----------------------------------------------------
// SETUP
//----------------------------------------------------

extern void uart_init ( const uint8_t flags ) ;
  
//----------------------------------------------------
// BLOCKED I/O
//----------------------------------------------------

extern const uint8_t uart_is_read_ready (void ) ;
extern const uint8_t uart_read_byte (void ) ;

extern void uart_write_byte(const uint8_t c) ;
extern void uart_write_buffer (const void * src , uint8_t n ) ;
extern void uart_puts ( const char * str ) ;

//----------------------------------------------------
// ASYNC I/O
//----------------------------------------------------

extern uint8_t uart_rxbuf_read (uint8_t * dst , const uint8_t maxsize ) ;
extern uint8_t uart_rxbuf_readbyte (uint8_t * dst ) ;
extern uint8_t uart_send_async (const void * src , const uint8_t size ) ;
extern uint8_t uart_sendbyte_async (const uint8_t byte ) ;

extern uint8_t uart_rxbuf_read_wait ( void * p , uint8_t size ) ;
extern uint8_t uart_rxbuf_readbyte_wait (uint8_t * dst) ;
extern uint8_t uart_send_async_wait ( const void * p , uint8_t size ) ;
extern uint8_t uart_sendbyte_async_wait (const uint8_t byte ) ;

# endif // REFLOW_UART_INCLUDE
