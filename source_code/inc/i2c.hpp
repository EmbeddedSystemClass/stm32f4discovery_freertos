

#pragma once

#include "i2c_common.hpp"
extern "C" {
  #include "libopencm3/stm32/i2c.h"
}

namespace i2c {

// Base class for I2C peripherals
//
// Currently only implements blocking reads and writes. An object receives a base peripheral address (i.e. I2C1)
// and becomes a set of low-level functions for this peripheral
class I2C {
 public:
  explicit I2C(uint32_t i2c_base);
  void Init(config_t *config);
  void Enable();
  void Disable();
  void BlockingRead(uint8_t addr, xfer_data_t* xfer);
  void BlockingWrite(uint8_t addr, xfer_data_t* xfer);

 private:
  uint32_t i2c_base;
};

};  // namespace i2c
