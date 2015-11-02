
#include "rs_robot.h"

#define ARM_TIMER_LOD 0x2000B400
#define ARM_TIMER_VAL 0x2000B404
#define ARM_TIMER_CTL 0x2000B408
#define ARM_TIMER_CLI 0x2000B40C
#define ARM_TIMER_RIS 0x2000B410
#define ARM_TIMER_MIS 0x2000B414
#define ARM_TIMER_RLD 0x2000B418
#define ARM_TIMER_DIV 0x2000B41C
#define ARM_TIMER_CNT 0x2000B420

#define SYST_CLO 0x20003004
#define SYST_CHI 0x20003008


static mrb_value
mrb_rs_system_timer_initialize(mrb_state *mrb, mrb_value self)
{
	unsigned int interval = 1000 -1;

	PUT32(IRQ_DISABLE_BASIC,1);
    PUT32(ARM_TIMER_CTL,0x003E0000);
    //initial setting?
    PUT32(ARM_TIMER_LOD,1000);
    PUT32(ARM_TIMER_RLD,1000);

    PUT32(ARM_TIMER_DIV,0x000000F9);
    PUT32(ARM_TIMER_CLI,0);
    PUT32(ARM_TIMER_CTL,0x003E00A2);

	while(1) if(GET32(ARM_TIMER_MIS)) break;
	PUT32(ARM_TIMER_CLI,0);

    PUT32(IRQ_ENABLE_BASIC,1);	//don't remove

    while(1) if(GET32(IRQ_BASIC)&1) break;
	PUT32(ARM_TIMER_CLI,0);

    PUT32(ARM_TIMER_LOD,interval);
    PUT32(ARM_TIMER_RLD,interval);

    return self;
}


static mrb_value
mrb_rs_system_timer_now(mrb_state *mrb, mrb_value self)
{
    unsigned int chi;
    unsigned int clo;

    // get counter's value
    chi = *(volatile unsigned int *)SYST_CHI;
    clo = *(volatile unsigned int *)SYST_CLO;

    // carry check
    if (chi != *(volatile unsigned int *)SYST_CHI) {
	    // update CHI and CLO
	    chi = *(volatile unsigned int *) SYST_CHI;
	    clo = *(volatile unsigned int *) SYST_CLO;
    }
    // return as 64bit value
    // return (chi << 32) + clo;
    // now we use only 32bit
    return mrb_fixnum_value(clo);
}

static mrb_value
mrb_rs_system_timer_reset(mrb_state *mrb, mrb_value self)
{
    // reset counter
    *(volatile unsigned int *)SYST_CLO = 0;
    *(volatile unsigned int *)SYST_CHI = 0;
    return mrb_fixnum_value(0);
}

void
mrb_mruby_rs_system_timer_gem_init(mrb_state* mrb) {
	struct RClass *timer;
	timer = mrb_define_class(mrb, "SystemTimer", mrb->object_class);

	/* methods */
	mrb_define_method(mrb, timer, "initialize", mrb_rs_system_timer_initialize, MRB_ARGS_NONE());
	mrb_define_method(mrb, timer, "now", mrb_rs_system_timer_now, MRB_ARGS_NONE());
	mrb_define_method(mrb, timer, "reset", mrb_rs_system_timer_reset, MRB_ARGS_NONE());
}

void
mrb_mruby_rs_system_timer_gem_final(mrb_state* mrb) {
  // finalizer
}
