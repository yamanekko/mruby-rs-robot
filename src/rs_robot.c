#include "rs_robot.h"

void mrb_mruby_rs_gyro_gem_init(mrb_state* mrb);
void mrb_mruby_rs_motor_gem_init(mrb_state* mrb);
void mrb_mruby_rs_serial_gem_init(mrb_state* mrb);
void mrb_mruby_rs_system_timer_gem_init(mrb_state* mrb);
void mrb_mruby_rs_gyro_gem_final(mrb_state* mrb);
void mrb_mruby_rs_motor_gem_final(mrb_state* mrb);
void mrb_mruby_rs_serial_gem_final(mrb_state* mrb);
void mrb_mruby_rs_system_timer_gem_final(mrb_state* mrb);

static mrb_value
mrb_mruby_rs_robot_delay(mrb_state *mrb, mrb_value self)
{	//int divisor
    unsigned int ra2;
	uint32_t pwm_control;
    mrb_int wait;

    mrb_get_args(mrb, "i", &wait);
    for(ra2=0;ra2<wait;ra2++) dummy(ra2);

    return self;
}

void
mrb_mruby_rs_robot_gem_init(mrb_state* mrb) {
	struct RClass * robot_class = mrb_define_class(mrb, "RSRobot", mrb->object_class);
	mrb_define_class_method(mrb, robot_class, "delay", mrb_mruby_rs_robot_delay, MRB_ARGS_REQ(1));

	// call gem initializer
    mrb_mruby_rs_gyro_gem_init(mrb);
    mrb_mruby_rs_motor_gem_init(mrb);
    mrb_mruby_rs_serial_gem_init(mrb);
    mrb_mruby_rs_system_timer_gem_init(mrb);
}

void
mrb_mruby_rs_robot_gem_final(mrb_state* mrb) {
    // call gem finalizer
    mrb_mruby_rs_gyro_gem_final(mrb);
    mrb_mruby_rs_motor_gem_final(mrb);
    mrb_mruby_rs_serial_gem_final(mrb);
    mrb_mruby_rs_system_timer_gem_final(mrb);
}
