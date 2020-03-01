

#pragma once

#include "etl/delegate.h"
#include "etl/observer.h"
extern "C" {
  #include "libopencm3/stm32/gpio.h"
  #include <freertos/FreeRTOS.h>
  #include <freertos/portmacro.h>
  #include <freertos/FreeRTOS_CLI.h>
}

namespace gpio {

// Generic GPIO class for data encapsulation of the HAL.
//
// When created, the port offset is passed in so that the object ends up being a set of functions to
// access pins on that port. The clock is automatically enabled based on this offset as well because
// I keep forgetting to do it otherwise.
class Gpio {
 public:
  explicit Gpio(uint32_t port);
  void SetPins(uint16_t gpios);
  void ClearPins(uint16_t gpios);
  uint16_t ReadPins(uint16_t gpios);
  void TogglePins(uint16_t gpios);
  uint16_t ReadPort();
  void WritePort(uint16_t data);
  void ModeSetup(uint8_t mode, uint8_t pull_up_down, uint16_t gpios);
  void SetOutputOptions(uint8_t otype, uint8_t speed, uint16_t gpios);
  void SetAltFunction(uint8_t alt_func_num, uint16_t gpios);

 private:
  uint32_t m_port;
};

enum class PortOffsets {
  PORT_A = 0x0000,
  PORT_B = 0x0400,
  PORT_C = 0x0800,
  PORT_D = 0x0C00,
  PORT_E = 0x1000,
  PORT_F = 0x1400,
};

typedef struct {
  uint32_t pin_num;
  gpio::Gpio* port;
} pin_t;

};  // namespace gpio
