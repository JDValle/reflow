# include <string.h>
# include <avr/interrupt.h>
# include "common.h"
# include "timer.h"
# include "uart.h"

# define UART_RXBUFSIZEMASK   ( UART_RXBUFSIZE - 1 )

static volatile uint8_t uart_rxbuf  [ UART_RXBUFSIZE ] ;
static volatile uint8_t uart_rxhead = 0 ;
static volatile uint8_t uart_rxtail = 0 ;
static volatile uint8_t uart_rxused = 0 ;

inline void uart_rxbuf_init (void )
{
  uart_rxhead = 0 ;
  uart_rxtail = 0 ;
  uart_rxused = 0 ;
}

# define UART_TXBUFSIZEMASK   ( UART_TXBUFSIZE - 1 )

static volatile uint8_t uart_txbuf  [ UART_TXBUFSIZE ] ;
static volatile uint8_t uart_txhead   = 0 ;
static volatile uint8_t uart_txtail   = 0 ;
static volatile uint8_t uart_txunused = 0 ;
static volatile uint8_t uart_txend    = 1 ; /* TRUE -> all data has been sent */

inline void uart_txbuf_init (void )
{
  uart_txhead = 0 ;
  uart_txtail = 0 ;
  uart_txunused = 0 ;
  uart_txend  = 1 ;
}

# define UART_ENABLE_RXINT    SetBit    ( UCSR0B , RXCIE0 )
# define UART_DISABLE_RXINT   ClearBit ( UCSR0B , RXCIE0 )
# define UART_ENABLE_UDRINT   SetBit   ( UCSR0B , UDRIE0 )
# define UART_DISABLE_UDRINT  ClearBit ( UCSR0B , UDRIE0 )
# define UART_UDR        UDR0
# define UART_RX_INT     USART_RX_vect
# define UART_TX_INT     USART_TX_vect
# define UART_UDRE_INT   USART_UDRE_vect

//----------------------------------------------------
// SETUP
//----------------------------------------------------

// DO NOT CHANGE SETUP ORDER!!!
// 1 BAUD RATE
// 2 ENABLE RX/TX
// 3 SETUP PARITY

void uart_init ( const uint8_t flags )
{
  if ( flags & UART_INIT_INTERRUPT_ENABLED )
    {
      while ( uart_txend != 1 ) ;
    }

  // disable to set the baudrate
  UCSR0B = 0 ;

  /* set the framing to 8N1 async */
  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00) ;  

  /* Set the baud rate , high first */  
  UBRR0H = (uint8_t) (UART_UBRR_CURRENT >> 8) ;
  UBRR0L = (uint8_t)  UART_UBRR_CURRENT ;

  if ( flags & UART_INIT_INTERRUPT_ENABLED )
    {
      uart_rxbuf_init () ;
      uart_txbuf_init () ;
      UCSR0B = _BV(RXCIE0) | _BV(TXCIE0) | _BV(RXEN0) | _BV(TXEN0) ;
      UCSR0A = _BV(TXC0) ;
    }
  else
    {
      UCSR0B = _BV(RXEN0) | _BV(TXEN0) ;
    }
}

//----------------------------------------------------
// BLOCKED I/O
//----------------------------------------------------

const uint8_t uart_is_read_ready (void )
{
return ( UCSR0A & _BV(RXC0) ) ;
}

const uint8_t uart_read_byte (void )
{
  while ( (UCSR0A & _BV(RXC0) ) == 0 ) ;
  return UART_UDR ;
}

void uart_write_byte(const uint8_t c)
{
  while ( (UCSR0A & _BV(UDRE0) ) == 0 ) ;
  UART_UDR = c ;
}

void uart_write_buffer (const void * src , uint8_t n )
{
  const uint8_t * p = src ;
  
  while (n--) uart_write_byte ( *p++ );
}

void uart_puts ( const char * str )
{
  while ( *str ) uart_write_byte ( *str++ ) ;
}

//----------------------------------------------------
// ASYNC I/O
//----------------------------------------------------

uint8_t uart_rxbuf_read (uint8_t * dst , const uint8_t maxsize )
{
  if (!maxsize) return 0 ;
  
  UART_DISABLE_RXINT ;

  const uint8_t readable = (maxsize > uart_rxused) ? uart_rxused : maxsize ;
  if (!readable) goto uart_rxbuf_read_end ;
  
  uint8_t i = readable ;
  
  while (i--)
    {
      *(dst++) = uart_rxbuf[uart_rxhead++] ;
      uart_rxhead &= UART_RXBUFSIZEMASK ;
    }
  uart_rxused -= readable;

uart_rxbuf_read_end :
  
  UART_ENABLE_RXINT ;

  return readable ;
}

uint8_t uart_rxbuf_readbyte (uint8_t * dst )
{  
  UART_DISABLE_RXINT ;

  if ( uart_rxused == 0 )
    {
      UART_ENABLE_RXINT ;
      return 0 ;
    }  
  
  *dst = uart_rxbuf[uart_rxhead++] ;
  uart_rxhead &= UART_RXBUFSIZEMASK ;
  -- uart_rxused ;  
  UART_ENABLE_RXINT ;

  return 1 ;
}

uint8_t uart_send_async (const void * src , const uint8_t size )
{
  const uint8_t * p = (const uint8_t *) src ;
  if (!size) return 0 ;
  
  UART_DISABLE_UDRINT ;
  
  const uint8_t written = (size > uart_txunused) ? uart_txunused : size ;
  if (!written) goto uart_send_async_end ;
  
  uint8_t i = written ;
  
  while (i--)
    {
      uart_txbuf[uart_txtail++] = *p++ ;
      uart_txtail &= UART_TXBUFSIZEMASK ;
    }

  uart_txunused -= written;
  uart_txend = 0 ;

uart_send_async_end :
  UART_ENABLE_UDRINT ;

  return written;
}

uint8_t uart_sendbyte_async (const uint8_t byte )
{  
  UART_DISABLE_UDRINT ;

  if ( uart_txunused == 0 )
    {
      UART_ENABLE_UDRINT ;
      return 0 ;
    }
  
  uart_txbuf[uart_txtail++] = byte ;
  uart_txtail &= UART_TXBUFSIZEMASK ;
  -- uart_txunused ;
  
  uart_txend = 0 ;  
  
  UART_ENABLE_UDRINT ;

  return 1 ;
}

uint8_t uart_rxbuf_read_wait ( void * p , uint8_t size )
{  
  uint32_t prev = timer_ms () ;
  uint16_t ms = UART_TIMEOUTMS ;

  uint8_t * q = (uint8_t * ) p ;
  
  for (;;)
    {
      const uint8_t r = uart_rxbuf_read ( q , size ) ;
      if (r) { size -= r ; q += r ; }

      if (size == 0 ) return UART_ERROR_OK ;
      
      const uint8_t curr = timer_ms () ;
      
      if ( prev != curr )
  {
    prev = curr ;
          if ( (-- ms ) == 0 ) return UART_ERROR_TIMEOUT ;
  }      
    }
}

uint8_t uart_rxbuf_readbyte_wait (uint8_t * dst)
{ 
  uint32_t prev = timer_ms () ;
  uint16_t ms = UART_TIMEOUTMS ;

  for (;;)
    {
      if ( uart_rxbuf_readbyte(dst) != 0 ) return UART_ERROR_OK ;
      
      const uint8_t curr = timer_ms () ;

      if ( prev != curr )
  {
    prev = curr ;
          if ( (-- ms ) == 0 ) return UART_ERROR_TIMEOUT ;
  }
    }
}

uint8_t uart_send_async_wait ( const void * p , uint8_t size )
{  
  uint32_t prev = timer_ms () ;
  uint16_t ms = UART_TIMEOUTMS ;

  const uint8_t * q = (const uint8_t * ) p ;

  for (;;)
  {
    const uint8_t r = uart_send_async ( q , size ) ;
    if (r) { size -= r ; q += r ; }

    if (size == 0 ) return UART_ERROR_OK ;

    const uint8_t curr = timer_ms () ;

    if ( prev != curr )
    {
      prev = curr ;
      if ( (-- ms ) == 0 ) return UART_ERROR_TIMEOUT ;
    }      
  }
}

uint8_t uart_sendbyte_async_wait (const uint8_t byte )
{

  uint32_t prev = timer_ms () ;
  uint16_t ms = UART_TIMEOUTMS ;

  for (;;)
  {
    if ( uart_sendbyte_async(byte) != 0 ) return UART_ERROR_OK ;

    const uint8_t curr = timer_ms () ;

    if ( prev != curr )
    {
      prev = curr ;
      if ( (-- ms ) == 0 ) return UART_ERROR_TIMEOUT ;
    }
  }
}

//----------------------------------------------------
// INTERRUPTS
//----------------------------------------------------

ISR(UART_RX_INT)
{
  const uint8_t status = UCSR0A & ( _BV(FE0) | _BV(DOR0) | _BV(UPE0) ) ;
  const uint8_t data   = UART_UDR ;
  
  if ( (!status) && (uart_rxused != UART_RXBUFSIZE) )
    {
      uart_rxbuf[uart_rxtail++] = data;
      uart_rxtail &= UART_RXBUFSIZEMASK ;
      ++ uart_rxused ;
    }
}

ISR(UART_UDRE_INT)
{
  if (uart_txunused == UART_TXBUFSIZE)
    {
      // disable the interrupt if we dont use it
      UART_DISABLE_UDRINT ;
    }
  else
    {
      // send one byte
      UART_UDR = uart_txbuf[uart_txhead++] ;
      uart_txhead &= UART_TXBUFSIZEMASK ;
      ++ uart_txunused ;
    }
}

ISR(UART_TX_INT)
{
  // we are done
  if (uart_txunused == UART_TXBUFSIZE)
    {
      uart_txend = 1 ;
    }
}
