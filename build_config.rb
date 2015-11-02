MRuby::Build.new do |conf|
  toolchain :gcc

  conf.gembox 'default'
end

MRuby::CrossBuild.new('rs') do |conf|

  CC_PATH = ENV["ARM_CC"] || "arm-none-eabi-gcc"
  AR_PATH = ENV["ARM_AR"] || "arm-none-eabi-ar"

  toolchain :gcc

  enable_debug
  #conf.enable_bintest
  #conf.enable_test

  # from default.gembox
  conf.gem :core => "mruby-sprintf"
  conf.gem :core => "mruby-print"
  conf.gem :core => "mruby-math"
  conf.gem :core => "mruby-time"
  conf.gem :core => "mruby-struct"
  conf.gem :core => "mruby-enum-ext"
  conf.gem :core => "mruby-string-ext"
  conf.gem :core => "mruby-numeric-ext"
  conf.gem :core => "mruby-array-ext"
  conf.gem :core => "mruby-hash-ext"
  conf.gem :core => "mruby-range-ext"
  conf.gem :core => "mruby-proc-ext"
  conf.gem :core => "mruby-symbol-ext"
  conf.gem :core => "mruby-random"
  conf.gem :core => "mruby-object-ext"
  conf.gem :core => "mruby-objectspace"
  conf.gem :core => "mruby-fiber"
  conf.gem :core => "mruby-enumerator"
  conf.gem :core => "mruby-enum-lazy"
  conf.gem :core => "mruby-toplevel-ext"
  conf.gem :core => "mruby-kernel-ext"
  conf.gem :core => "mruby-compiler"

  # Use Robot class
  conf.gem '../mruby_rs_robot'

  CFLAGS = "-Wall -nostartfiles -ffreestanding -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfloat-abi=soft -ggdb -g -O2"

  conf.cc do |cc|
    cc.command = CC_PATH

    # cc.include_paths << ["#{MRUBY_ROOT}/include/"]

    cc.flags << "#{CFLAGS}"
  end

  conf.archiver do |archiver|
    archiver.command = AR_PATH
    archiver.archive_options = 'rcs %{outfile} %{objs}'
  end

  # No binaries necessary
  conf.bins = []
end


