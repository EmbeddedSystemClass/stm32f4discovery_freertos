

#include "i2c1.hpp"
#include "assert_err.hpp"
#include <stdlib.h>
#include "isr.hpp"
extern "C" {
  #include <libopencm3/stm32/rcc.h>
  #include <libopencm3/cm3/nvic.h>
  #include "freertos/FreeRTOS.h"
  #include "freertos/portmacro.h"
  #include "freertos/projdefs.h"
}

namespace i2c1 {

constexpr uint32_t i2c1_base = I2C1_BASE;

I2C1::I2C1(gpio::Gpio *port_b) : I2C(i2c1_base),
                    current_state(State::DISABLED) {
  // i2c1 pin/port configuration
  pins.scl = {GPIO6, port_b};
  pins.sda = {GPIO9, port_b};
}

i2c::RetCode I2C1::Init(i2c::config_t* cfg) {
  i2c::RetCode ret_code = i2c::RetCode::SUCCESS;
  switch (current_state) {
    case State::DISABLED:
      config = *cfg;

      rcc_periph_clock_enable(RCC_I2C1);
      pins.scl.port->ModeSetup(GPIO_MODE_AF, GPIO_PUPD_NONE, pins.scl.pin_num);
      pins.scl.port->SetAltFunction(GPIO_AF4, pins.scl.pin_num);
      pins.scl.port->SetOutputOptions(GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, pins.scl.pin_num);
      pins.sda.port->ModeSetup(GPIO_MODE_AF, GPIO_PUPD_NONE, pins.sda.pin_num);
      pins.sda.port->SetAltFunction(GPIO_AF4, pins.sda.pin_num);
      pins.sda.port->SetOutputOptions(GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, pins.sda.pin_num);

      I2C::Init(&config);
      current_state = State::INITIALIZED;
      break;

    default:
      ret_code = i2c::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

i2c::RetCode I2C1::Enable() {
  i2c::RetCode ret_code = i2c::RetCode::SUCCESS;
  switch (current_state) {
    case State::INITIALIZED:
      I2C::Enable();
      current_state = State::READY;
      break;

    default:
      ret_code = i2c::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

i2c::RetCode I2C1::Disable() {
  i2c::RetCode ret_code = i2c::RetCode::SUCCESS;
  switch (current_state) {
    case State::INITIALIZED:
      I2C::Disable();
      current_state = State::DISABLED;
      break;

    default:
      ret_code = i2c::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

i2c::RetCode I2C1::BlockingRead(i2c::xfer_data_t* xfer) {
  i2c::RetCode ret_code = i2c::RetCode::SUCCESS;
  switch (current_state) {
    case State::READY:
      current_state = State::BUSY;
      I2C::BlockingRead(xfer->address, xfer);
      current_state = State::READY;
      break;

    case State::BUSY:
      ret_code = i2c::RetCode::BUSY;
      break;

    default:
      ret_code = i2c::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

i2c::RetCode I2C1::BlockingWrite(i2c::xfer_data_t* xfer) {
  i2c::RetCode ret_code = i2c::RetCode::SUCCESS;
  switch (current_state) {
    case State::READY:
      current_state = State::BUSY;
      I2C::BlockingWrite(xfer->address, xfer);
      current_state = State::READY;
      break;

    case State::BUSY:
      ret_code = i2c::RetCode::BUSY;
      break;

    default:
      ret_code = i2c::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

};  // namespace i2c1
