

#pragma once

#include "gpio.hpp"

namespace led {

// LED class that acts a heartbeat for program
//
// Currently just toggles the LED on and off @ 1Hz. The port and pin to be used are passed during instantiation.
class Led {
 public:
  Led(gpio::Gpio* port, uint32_t pin);
  void Init();
  void BlinkLedTask();

 private:
  gpio::Gpio* port;
  uint32_t pin;
};

};  // namespace led
