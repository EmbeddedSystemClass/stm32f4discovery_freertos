

#pragma once

#include "i2c.hpp"
#include "gpio.hpp"
extern "C" {
  #include "libopencm3/stm32/i2c.h"
  #include <freertos/FreeRTOS.h>
  #include <freertos/portmacro.h>
  #include <freertos/FreeRTOS_CLI.h>
  #include <freertos/queue.h>
}

namespace i2c1 {

// Naming conflict exists with libopencm3 library so it's undefined just for this file
#undef I2C1

enum class State {
  DISABLED,
  INITIALIZED,
  READY,
  BUSY,
};

// Class for I2C1 peripheral which uses I2C base class
//
// The constructor takes the port(s) that will be used for the peripheral. Pins are configured to be open-drain
// with no pull-up as pull-ups exist already on Discovery board.
class I2C1 : public i2c::I2C {
 public:
  explicit I2C1(gpio::Gpio* port_b);
  i2c::RetCode Init(i2c::config_t *config);
  i2c::RetCode Enable();
  i2c::RetCode Disable();
  i2c::RetCode BlockingRead(i2c::xfer_data_t* xfer);
  i2c::RetCode BlockingWrite(i2c::xfer_data_t* xfer);

 private:
  i2c::pins_t pins;
  i2c::config_t config;
  State current_state;
};

};  // namespace i2c1
