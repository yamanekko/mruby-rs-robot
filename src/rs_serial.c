#include "rs_robot.h"
#include "mruby/string.h"
#include <string.h>

#define AUX_MU_IO_REG   0x20215040	//Mini Uart I/O Data
#define AUX_MU_IER_REG  0x20215044	//Mini Uart Interrupt Enable
#define AUX_MU_IIR_REG  0x20215048	//Mini Uart Interrupt Identify
#define AUX_MU_LCR_REG  0x2021504C	//Mini Uart Line Control
#define AUX_MU_MCR_REG  0x20215050	//Mini Uart Modem Control
#define AUX_MU_LSR_REG  0x20215054	//Mini Uart Line Status
#define AUX_MU_MSR_REG  0x20215058	//Mini Uart Modem Status
#define AUX_MU_SCRATCH  0x2021505C	//Mini Uart Scratch
#define AUX_MU_CNTL_REG 0x20215060	//Mini Uart Extra Control
#define AUX_MU_STAT_REG 0x20215064	//Mini Uart Extra Status
#define AUX_MU_BAUD_REG 0x20215068	//Mini Uart Baudrate

/**
 * from https://github.com/dwelch67/raspberrypi.git
 */
static void
uart_putc(unsigned int c)
{
    while(1)
    {
        if(GET32(AUX_MU_LSR_REG)&0x20) break;
    }
    PUT32(AUX_MU_IO_REG,c);
}

static mrb_value
mrb_rs_serial_initialize(mrb_state *mrb, mrb_value self)
{
	// use port 14,15
    unsigned int ra;

    PUT32(AUX_ENABLES,1);
    PUT32(AUX_MU_IER_REG,0);
    PUT32(AUX_MU_CNTL_REG,0);
    PUT32(AUX_MU_LCR_REG,3);
    PUT32(AUX_MU_MCR_REG,0);
    PUT32(AUX_MU_IER_REG,0x5); //enable rx interrupts
    PUT32(AUX_MU_IIR_REG,0xC6);
    PUT32(AUX_MU_BAUD_REG,270);

    ra=GET32(GPFSEL1);
    ra&=~(7<<12); //gpio14
    ra|=2<<12;    //alt5
    ra&=~(7<<15); //gpio15
    ra|=2<<15;    //alt5
    PUT32(GPFSEL1,ra);

    PUT32(GPPUD,0);
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,(1<<14)|(1<<15));
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,0);

    PUT32(AUX_MU_CNTL_REG,3);


	  return self;
}

static mrb_value
mrb_rs_serial_puts(mrb_state *mrb, mrb_value self)
{
	char *data;
	int data_len;
    int idx = 0;

	mrb_get_args(mrb, "s", &data, &data_len);

    for(idx = 0; idx < data_len ; idx++){
    	uart_putc(data[idx]);
    }
    uart_putc(0x0D);
    uart_putc(0x0A);

	return self;
}

static mrb_value
mrb_rs_serial_write(mrb_state *mrb, mrb_value self)
{
	char *data;
	int data_len;
    int idx = 0;

	mrb_get_args(mrb, "s", &data, &data_len);

    for(idx = 0; idx < data_len ; idx++){
    	uart_putc(data[idx]);
    }
	return self;
}


void
mrb_mruby_rs_serial_gem_init(mrb_state* mrb) {
	struct RClass *serial;
	serial = mrb_define_class(mrb, "Serial", mrb->object_class);

	/* methods */
	mrb_define_method(mrb, serial, "initialize", mrb_rs_serial_initialize, MRB_ARGS_NONE());
	mrb_define_method(mrb, serial, "write", mrb_rs_serial_write, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, serial, "puts", mrb_rs_serial_puts, MRB_ARGS_REQ(1));

}

void
mrb_mruby_rs_serial_gem_final(mrb_state* mrb) {
  // finalizer
}
