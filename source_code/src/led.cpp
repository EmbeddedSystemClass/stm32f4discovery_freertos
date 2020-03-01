

#include "led.hpp"
#include "assert_err.hpp"
extern "C" {
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>
}

namespace led {

void vBlinkLed(void *params);

Led::Led(gpio::Gpio* gpio_port, uint32_t gpio_pin) : port(gpio_port),
                                                     pin(gpio_pin) {

}

void Led::Init() {
  port->ModeSetup(GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pin);
  port->SetOutputOptions(GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pin);
  ASSERT_ERR(xTaskCreate(vBlinkLed, "BlinkLed", configMINIMAL_STACK_SIZE, this, 2, NULL));
}

void Led::BlinkLedTask() {
  while (1) {
    port->TogglePins(pin);
    vTaskDelay(1000);
  }
}

void vBlinkLed(void *params) {
  static_cast<led::Led *>(params)->BlinkLedTask();
}

};  // namespace led
