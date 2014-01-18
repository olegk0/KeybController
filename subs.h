
#define XTAL F_CPU
#include <stdint.h>


/*************************************************************************
 delay loop for small accurate delays: 16-bit counter, 4 cycles/loop
*************************************************************************/
static inline void _delayFourCycles(unsigned int __count)
{
    if ( __count == 0 )    
        __asm__ __volatile__( "rjmp 1f\n 1:" );    // 2 cycles
    else
        __asm__ __volatile__ (
    	    "1: sbiw %0,1" "\n\t"                  
    	    "brne 1b"                              // 4 cycles/loop
    	    : "=w" (__count)
    	    : "0" (__count)
    	   );
   
}


/************************************************************************* 
delay for a minimum of <us> microseconds
the number of loops is calculated at compile-time from MCU clock frequency
*************************************************************************/
#define delay(us)  _delayFourCycles( ( ( 1*(XTAL/4000) )*us)/1000 )

//on 4MHz
#define t2_on_2ms() 	{ TCNT2 = 0; TCCR2=0x04; }
#define t2_on_32ms() 	{ TCNT2 = 0; TCCR2=0x07; }

#define t2_off() 	 	{ TCCR2 = 0; }
#define t2_overflow()   ( TCCR2==0 )

#define t1_on_Xms(x) 	{ TCNT1 = 0xFFFF-(x<<2); TCCR1B=0x05; }
#define t1_off() 	 	{ TCCR1B = 0; }
#define t1_overflow()   ( TCCR1B==0 )




