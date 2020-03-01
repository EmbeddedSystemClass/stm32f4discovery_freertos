

#include "i2c.hpp"
#include <stdlib.h>
extern "C" {
  #include "libopencm3/cm3/nvic.h"
}

namespace i2c {

I2C::I2C(uint32_t i2c_base) : i2c_base(i2c_base) {

}

void I2C::Init(config_t* config) {
  i2c_reset(i2c_base);
  i2c_peripheral_disable(i2c_base);
  i2c_set_speed(i2c_base, static_cast<i2c_speeds>(config->speed), static_cast<uint32_t>(config->clock_freq));
}

void I2C::Enable() {
  i2c_peripheral_enable(i2c_base);
}

void I2C::Disable() {
  i2c_peripheral_disable(i2c_base);
}

void I2C::BlockingRead(uint8_t addr, xfer_data_t* xfer) {
  i2c_transfer7(i2c_base, addr, xfer->tx_buffer, 1, xfer->rx_buffer, xfer->num_bytes);
}

void I2C::BlockingWrite(uint8_t addr, xfer_data_t* xfer) {
  i2c_transfer7(i2c_base, addr, xfer->tx_buffer, xfer->num_bytes, xfer->rx_buffer, 1);
}

};  // namespace i2c
