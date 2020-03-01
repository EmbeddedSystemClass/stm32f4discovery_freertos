

#include "usb.hpp"
#include "gpio.hpp"
#include "cli.hpp"
#include "spi1.hpp"
#include "lis3dsh.hpp"
#include "led.hpp"
#include "i2c1.hpp"
#include "cs43l22.hpp"
extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <libopencm3/stm32/rcc.h>
  #include <libopencm3/stm32/gpio.h>
  #include <libopencm3/cm3/dwt.h>
  #include <libopencm3/cm3/scs.h>
  #include "freertos/FreeRTOS.h"
  #include "freertos/FreeRTOSConfig.h"
  #include "freertos/task.h"
}

/***** Definitions ***********************************************************/
#define HSE_VALUE ((uint32_t)8000000)

// Peripheral instantiations
gpio::Gpio port_a(GPIOA);
gpio::Gpio port_b(GPIOB);
gpio::Gpio port_d(GPIOD);
gpio::Gpio port_e(GPIOE);
spi1::Spi1 spi1_inst(&port_a, &port_e);
i2c1::I2C1 i2c1_inst(&port_b);

// Driver instantiations
led::Led led_inst(&port_d, GPIO12);
usb::Usb usb_inst(&port_a);

// Device layer
lis3dsh::Lis3dsh accel(&spi1_inst);
cs43l22::Cs43l22 dac(&i2c1_inst, &port_d);
cli::Cli cli_inst(&usb_inst);

/***** Main Loop *************************************************************/
int main() {
  // Enable counter so segger systemview has timestamps for events
  // dwt_enable_cycle_counter();

  // Enable system clock
  rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

  // Start SEGGER SystemView
  // SEGGER_SYSVIEW_Conf();
  // SEGGER_SYSVIEW_Start();

  // Device initializations
  led_inst.Init();
  cli_inst.Init();
  accel.Init(&accel);
  dac.Init(&dac);

  // Start scheduler and run program
  vTaskStartScheduler();

  // Program should never reach this
  while (1) {}
}

extern "C" {

void vApplicationTickHook() {

}

void vApplicationMallocFailedHook(xTaskHandle pxTask, signed char *pcTaskName) {
  ASSERT_ERR(false);
}

void vApplicationIdleHook() {

}

void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName) {
  ASSERT_ERR(false);
}

void vApplicationGetIdleTaskMemory() {

}

void vApplicationGetTimerTaskMemory() {

}

}
