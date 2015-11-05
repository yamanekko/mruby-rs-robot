
#include "rs_robot.h"


volatile unsigned int *clk = (volatile unsigned int*)0x20101000;
volatile unsigned int *pwm = (volatile unsigned int*)0x2020C000;
volatile unsigned int *gpio = (volatile unsigned int*)0x20200000;

#define	PWM_CTL  0
//same as wiriling pi
#define	PWM_RNG1 4
#define	PWM_DAT1 5
#define	PWM_RNG2 8
#define	PWM_DAT2 9

#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
// from wiriling pi
#define	PWMCLK_CNTL 40
#define	PWMCLK_DIV  41	// stop clock and waiting for busy flag doesn't work, so kill clock

// PWM
//	Word offsets into the PWM control region

#define	PWM_CONTROL 0
#define	PWM_STATUS  1
#define	PWM0_RANGE  4
#define	PWM0_DATA   5
#define	PWM1_RANGE  8
#define	PWM1_DATA   9

#define	PWM0_MS_MODE    0x0080  // Run in MS mode
#define	PWM0_USEFIFO    0x0020  // Data from FIFO
#define	PWM0_REVPOLAR   0x0010  // Reverse polarity
#define	PWM0_OFFSTATE   0x0008  // Ouput Off state
#define	PWM0_REPEATFF   0x0004  // Repeat last value if FIFO empty
#define	PWM0_SERIAL     0x0002  // Run in serial mode
#define	PWM0_ENABLE     0x0001  // Channel Enable

#define	PWM1_MS_MODE    0x8000  // Run in MS mode
#define	PWM1_USEFIFO    0x2000  // Data from FIFO
#define	PWM1_REVPOLAR   0x1000  // Reverse polarity
#define	PWM1_OFFSTATE   0x0800  // Ouput Off state
#define	PWM1_REPEATFF   0x0400  // Repeat last value if FIFO empty
#define	PWM1_SERIAL     0x0200  // Run in serial mode
#define	PWM1_ENABLE     0x0100  // Channel Enable

#define	BCM_PASSWORD		0x5A000000

// from wirilng.h
#define	PWM_MODE_MS		0
#define	PWM_MODE_BAL		1

#define L_MOTOR 0
#define R_MOTOR 1

#define FORWARD 0
#define REVERSE 1

static void
rs_motor_set_mode(mrb_state *mrb, mrb_int mode)
{
    mrb_value mode_val = mrb_fixnum_value(mode);
    mrb_value motor = mrb_obj_value(mrb_class_get(mrb, "Motor"));
    mrb_iv_set(mrb, motor, mrb_intern_lit(mrb, "@pwm_mode"), mode_val);

    if (mode == PWM_MODE_MS) {
		*(pwm + PWM_CONTROL) = PWM0_ENABLE | PWM1_ENABLE | PWM0_MS_MODE | PWM1_MS_MODE ;
    } else {
		*(pwm + PWM_CONTROL) = PWM0_ENABLE | PWM1_ENABLE ;
    }
}

static mrb_value
mrb_rs_motor_pwm_SetMode (mrb_state *mrb, mrb_value self)
{	//int mode
	  mrb_int mode;

	  mrb_get_args(mrb, "i", &mode);
      rs_motor_set_mode(mrb, mode);
	  return self;
}


static void
rs_motor_set_clock(mrb_state *mrb, mrb_int divisor)
{
    unsigned int ra2;
	uint32_t pwm_control;

	mrb_value divisor_val = mrb_fixnum_value(divisor);
    mrb_value motor = mrb_obj_value(mrb_class_get(mrb, "Motor"));
    mrb_iv_set(mrb, motor, mrb_intern_lit(mrb, "@divisor"), divisor_val);

	  pwm_control = *(pwm + PWM_CONTROL) ;		// preserve PWM_CONTROL
	  *(pwm + PWM_CONTROL) = 0 ;			// Stop PWM
	  *(clk + PWMCLK_CNTL) = BCM_PASSWORD | 0x01 ;	// Stop PWM Clock
//    delayMicroseconds (110) ;			// prevents clock going sloooow
	  for(ra2=0;ra2<0x110000;ra2++) dummy(ra2);

	  while ((*(clk + PWMCLK_CNTL) & 0x80) != 0)	// Wait for clock to be !BUSY
	//    delayMicroseconds (1) ;
	  for(ra2=0;ra2<0x1000;ra2++) dummy(ra2);

//	  *(clk + PWMCLK_DIV)  = BCM_PASSWORD | (divisor << 12) ;
	  *(clk + PWMCLK_DIV)  = 0x5A000000 | (75<<12);
//	  *(clk + PWMCLK_CNTL) = BCM_PASSWORD | 0x11 ;	// Start PWM clock
	  *(clk + PWMCLK_CNTL) = 0x5A000211;	// source=osc and enable clock
	  *(pwm + PWM_CONTROL) = pwm_control ;		// restore PWM_CONTROL
}

static mrb_value
mrb_rs_motor_pwm_SetClock(mrb_state *mrb, mrb_value self)
{	//int divisor

    mrb_int divisor;

    mrb_get_args(mrb, "i", &divisor);	//TODO 引数の値は使用してない
    divisor &= 4095;
    rs_motor_set_clock(mrb, divisor);

    return self;
}

static mrb_value
mrb_rs_motor_initialize(mrb_state *mrb, mrb_value self)
{
	// input1,input2 enableのペア(A:in1,in2,enableA B:in3,in4,enableB)
	//gpioピン番号とPWM1と2のどちらを使うかをもらう
//	mrb_int input1, input2, enable,pwmNo;
    unsigned int ra;
	unsigned int ra2;

//	mrb_get_args(mrb, "iiii", &input1, &input2, &enable, &pwmNo);
//	mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@input1"), mrb_fixnum_value(input1));
//	mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@input2"), mrb_fixnum_value(input2));
//	mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@enable"), mrb_fixnum_value(enable));
//	mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@pwm_no"), mrb_fixnum_value(pwmNo));
	volatile mrb_int n1,n2,n,npwm;
	mrb_value input1, input2, enable,pwmNo;
	mrb_get_args(mrb, "iiii", &n1, &n2, &n, &npwm);
	input1 = mrb_fixnum_value(n1);
	mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@input1"), input1);
	input2 = mrb_fixnum_value(n2);
	mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@input2"), input2);
	enable = mrb_fixnum_value(n);
	mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@enable"), enable);
	pwmNo = mrb_fixnum_value(npwm);
	mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@pwm_no"), pwmNo);

	//とりえあずPWM0=GPIO12, PWM1=GPIO19のみサポート
//	switch (enable) {
	switch (mrb_fixnum(enable)) {
		case 12:
			SET_GPIO_ALT(12, 0);	//PWM0
			//5
			ra=GET32(GPFSEL0);
			ra&=~(7<<15);
			ra|=1<<15;
			PUT32(GPFSEL0,ra);

			//6
			ra=GET32(GPFSEL0);
			ra&=~(7<<18);
			ra|=1<<18;
			PUT32(GPFSEL0,ra);

			*(pwm + PWM_RNG1) = 256;
			for(ra2=0;ra2<0x10000;ra2++) dummy(ra2);
			break;
		case 19:
			SET_GPIO_ALT(19, 5);	//PWM1
			//16
			ra=GET32(GPFSEL1);
			ra&=~(7<<18);
			ra|=1<<18;
			PUT32(GPFSEL1,ra);

			//20
			ra=GET32(GPFSEL2);
			ra&=~(7<<0);
			ra|=1<<0;
			PUT32(GPFSEL2,ra);

			*(pwm + PWM_RNG2) = 256;
			for(ra2=0;ra2<0x10000;ra2++) dummy(ra2);
			break;

		default:
		  return mrb_false_value();
		  break;
	}
	return self;
}

static mrb_value
mrb_rs_motor_stop(mrb_state *mrb, mrb_value self)
{

	int motor;
	motor = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@pwm_no")));

	if(motor == L_MOTOR){
		PUT32(GPCLR0,1<<16);
		PUT32(GPCLR0,1<<20);
	}else{
		PUT32(GPCLR0,1<<5);
		PUT32(GPCLR0,1<<6);
	}
	return self;
}

static mrb_value
mrb_rs_motor_drive(mrb_state *mrb, mrb_value self)
{
	int motor;

	//mrb_int n1, n2;
	mrb_int speed, rotation;
	//int hval,lval,val;
	//unsigned int high,low;

	int param = 50;

	mrb_get_args(mrb, "i", &speed);
	//rotation = mrb_fixnum_value(n1);
	//speed = mrb_fixnum_value(n2);

    if (speed > 0) {
        rotation = FORWARD;
    } else {
        rotation = REVERSE;
        speed = -speed;
    }

    mrb_value v = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@pwm_no"));
	motor = mrb_fixnum(v);

	if(motor == L_MOTOR){
		if(rotation == FORWARD){
			PUT32(GPSET0,1<<16);
			PUT32(GPCLR0,1<<20);
		}else{
			PUT32(GPCLR0,1<<16);
			PUT32(GPSET0,1<<20);
		}
		*(pwm + PWM_DAT1) = speed + param;	//DAT1
	}else{
		if(rotation == FORWARD){
			PUT32(GPSET0,1<<5);
			PUT32(GPCLR0,1<<6);
		}else{
			PUT32(GPCLR0,1<<5);
			PUT32(GPSET0,1<<6);
		}
		*(pwm + PWM_DAT2) = speed + param;	//DAT2
	}
	return self;
}

void
mrb_mruby_rs_motor_gem_init(mrb_state* mrb) {
	struct RClass *motor;
	motor = mrb_define_class(mrb, "Motor", mrb->object_class);

	mrb_define_const(mrb, motor, "MS", mrb_fixnum_value(0));
	mrb_define_const(mrb, motor, "BAL", mrb_fixnum_value(1));
	mrb_define_const(mrb, motor, "FORWARD", mrb_fixnum_value(0));
	mrb_define_const(mrb, motor, "REVERSE", mrb_fixnum_value(1));


	/* methods */
	mrb_define_method(mrb, motor, "initialize", mrb_rs_motor_initialize, MRB_ARGS_REQ(4));
	mrb_define_method(mrb, motor, "stop", mrb_rs_motor_stop, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, motor, "drive", mrb_rs_motor_drive, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, motor, "power=", mrb_rs_motor_drive, MRB_ARGS_REQ(1)); /* alias of Motor#drive */
	mrb_define_class_method(mrb, motor, "clock=", mrb_rs_motor_pwm_SetClock, MRB_ARGS_REQ(1));
	mrb_define_class_method(mrb, motor, "pwm_mode=", mrb_rs_motor_pwm_SetMode, MRB_ARGS_REQ(1));

    /* initialize motor */
    rs_motor_set_mode(mrb, PWM_MODE_MS);
    rs_motor_set_clock(mrb, 75);
}

void
mrb_mruby_rs_motor_gem_final(mrb_state* mrb) {
  // finalizer
}
