

#pragma once

#include "etl/delegate.h"
#include "etl/observer.h"
#include "spi_common.hpp"
#include "assert_err.hpp"
extern "C" {
  #include "libopencm3/stm32/spi.h"
}

namespace spi {

// Base class for SPI peripherals
//
// Currently implements interrupt-driver and blocking reads and writes. An object receives a base peripheral
// address (i.e. SPI1). and becomes a set of low-level functions for this peripheral
class Spi {
 public:
  Spi(uint32_t spi_base, uint32_t irqn);
  void Init(config_t *config);
  void Enable();
  void Disable();
  bool IsBusy();
  void BlockingRead(xfer_data_t *xfer);
  void BlockingWrite(xfer_data_t *xfer);
  uint8_t Read();
  void Write(uint8_t data);
  void EnableInterrupts(uint8_t priority);
  void DisableInterrupts();

 private:
  uint32_t spi_base;
  uint32_t irqn;
};

};  // namespace spi
