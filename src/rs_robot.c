#include "mruby.h"
#include "mruby/variable.h"

void mrb_mruby_rs_gyro_gem_init(mrb_state* mrb);
void mrb_mruby_rs_motor_gem_init(mrb_state* mrb);
void mrb_mruby_rs_serial_gem_init(mrb_state* mrb);
void mrb_mruby_rs_system_timer_gem_init(mrb_state* mrb);
void mrb_mruby_rs_gyro_gem_final(mrb_state* mrb);
void mrb_mruby_rs_motor_gem_final(mrb_state* mrb);
void mrb_mruby_rs_serial_gem_final(mrb_state* mrb);
void mrb_mruby_rs_system_timer_gem_final(mrb_state* mrb);

void
mrb_mruby_rs_robot_gem_init(mrb_state* mrb) {
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
