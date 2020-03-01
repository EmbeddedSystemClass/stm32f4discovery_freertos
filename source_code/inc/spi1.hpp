

#pragma once

#include "etl/delegate.h"
#include "etl/observer.h"
#include "spi.hpp"
#include "gpio.hpp"
extern "C" {
  #include "libopencm3/stm32/spi.h"
  #include <libopencm3/stm32/rcc.h>
  #include <freertos/FreeRTOS.h>
  #include <freertos/portmacro.h>
  #include <freertos/FreeRTOS_CLI.h>
  #include <freertos/queue.h>
}

namespace spi1 {

typedef struct {
  spi::Event event;
} note_t;

enum class State {
  DISABLED,
  INITIALIZED,
  READY,
  BUSY,
};

// Class for SPI1 peripheral which uses SPI base class
//
// The constructor takes the port(s) that will be used for the peripheral. InterruptHandler is used a callback
// for the SPI1 IRQ. This driver handles the logic for multiple bytes that are read/written in this callback
// and once the transaction is complete, the queue passed during initialization is sent a message with the result.
class Spi1 : public spi::Spi {
 public:
  Spi1(gpio::Gpio* port_a, gpio::Gpio* port_e);
  spi::RetCode Init(spi::config_t *config, QueueHandle_t queue);
  spi::RetCode Enable();
  spi::RetCode Disable();
  spi::RetCode Read(spi::xfer_data_t *xfer);
  spi::RetCode Write(spi::xfer_data_t *xfer);
  spi::RetCode BlockingRead(spi::xfer_data_t* xfer);
  spi::RetCode BlockingWrite(spi::xfer_data_t* xfer);
  void InterruptHandler(const size_t id);

 private:
  spi::pins_t pins;
  spi::config_t config;
  volatile spi::drv_xfer_t current_xfer;
  note_t spi_note;
  etl::delegate<void(size_t)> callback;
  QueueHandle_t observer_queue;
  volatile State current_state;
};

};  // namespace spi1
