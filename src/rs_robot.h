
#include "mruby.h"
#include "mruby/variable.h"


extern void PUT32 ( unsigned int, unsigned int );
extern unsigned int GET32 ( unsigned int );
extern void dummy ( unsigned int );

#define GPFSEL0 	0x20200000
#define GPFSEL1 	0x20200004
#define GPFSEL2 	0x20200008
#define GPSET0  	0x2020001C
#define GPCLR0  	0x20200028
#define GPPUD       0x20200094
#define GPPUDCLK0   0x20200098

#define AUX_ENABLES     0x20215004	//Auxiliary enables

#define IRQ_BASIC 0x2000B200
#define IRQ_PEND1 0x2000B204
#define IRQ_PEND2 0x2000B208
#define IRQ_FIQ_CONTROL 0x2000B210
#define IRQ_ENABLE_BASIC 0x2000B218
#define IRQ_DISABLE_BASIC 0x2000B224
