# ifndef REFLOW_COMMON_INCLUDE
# define REFLOW_COMMON_INCLUDE

# define SetBit(adr, bit)			( adr  |=  _BV(bit) )
# define ClearBit(adr, bit)			( adr  &= ~_BV(bit) )
# define ToggleBit(adr, bit)		( adr  ^= _BV(bit) )
# define BitIsSet(adr, bit)			((adr  &   _BV(bit) ) ? 1 : 0 )
# define BitIsClear(adr, bit)			((adr  &   _BV(bit) ) ? 0 : 1 )
# define BitAck(adr, bit)			( adr  =~  _BV(bit) )

# define MIN(a,b)	( (a)<(b) ? (a) : (b) )
# define MAX(a,b)	( (a)>(b) ? (a) : (b) )

# define ROUNDNEAR(num,factor)		((num) + (factor) - 1 - ((num) - 1) % (factor))

# endif // REFLOW_COMMON_INCLUDE
