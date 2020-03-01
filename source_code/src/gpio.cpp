

#include "gpio.hpp"
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/projdefs.h"
  #include "freertos/task.h"
  #include "libopencm3/stm32/rcc.h"
}

namespace gpio {

Gpio::Gpio(uint32_t port) : m_port(port) {
  // base address for the gpio ports
  uint32_t offset = m_port % PERIPH_BASE_AHB1;
  // based on address passed in, check which port it is and enable RCC
  switch (offset) {
    case static_cast<uint32_t>(PortOffsets::PORT_A): {
      rcc_periph_clock_enable(RCC_GPIOA);
    }
    case static_cast<uint32_t>(PortOffsets::PORT_B): {
      rcc_periph_clock_enable(RCC_GPIOB);
    }
    case static_cast<uint32_t>(PortOffsets::PORT_C): {
      rcc_periph_clock_enable(RCC_GPIOC);
    }
    case static_cast<uint32_t>(PortOffsets::PORT_D): {
      rcc_periph_clock_enable(RCC_GPIOD);
    }
    case static_cast<uint32_t>(PortOffsets::PORT_E): {
      rcc_periph_clock_enable(RCC_GPIOE);
    }
    case static_cast<uint32_t>(PortOffsets::PORT_F): {
      rcc_periph_clock_enable(RCC_GPIOF);
    }

    default:
      break;
  }
}

void Gpio::SetPins(uint16_t gpios) {
  gpio_set(m_port, gpios);
}

void Gpio::ClearPins(uint16_t gpios) {
  gpio_clear(m_port, gpios);
}

uint16_t Gpio::ReadPins(uint16_t gpios) {
  return gpio_get(m_port, gpios);
}

void Gpio::TogglePins(uint16_t gpios) {
  gpio_toggle(m_port, gpios);
}

uint16_t Gpio::ReadPort() {
  return gpio_port_read(m_port);
}

void Gpio::WritePort(uint16_t data) {
  gpio_port_write(m_port, data);
}

void Gpio::ModeSetup(uint8_t mode, uint8_t pull_up_down, uint16_t gpios) {
  gpio_mode_setup(m_port, mode, pull_up_down, gpios);
}

void Gpio::SetOutputOptions(uint8_t otype, uint8_t speed, uint16_t gpios) {
  gpio_set_output_options(m_port, otype, speed, gpios);
}

void Gpio::SetAltFunction(uint8_t alt_func_num, uint16_t gpios) {
  gpio_set_af(m_port, alt_func_num, gpios);
}

};  // namespace gpio
