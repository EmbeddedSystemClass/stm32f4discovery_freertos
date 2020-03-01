

#include "spi.hpp"
#include <stdlib.h>
extern "C" {
  #include "libopencm3/cm3/nvic.h"
}

namespace spi {

Spi::Spi(uint32_t spi_base, uint32_t irqn) : spi_base(spi_base), irqn(irqn) {

}

void Spi::Init(config_t* config) {
  spi_init_master(spi_base, static_cast<uint32_t>(config->baud_rate),
                            static_cast<uint32_t>(config->polarity),
                            static_cast<uint32_t>(config->phase),
                            static_cast<uint32_t>(config->frame_format),
                            static_cast<uint32_t>(config->byte_order));
}

void Spi::Enable() {
  nvic_clear_pending_irq(irqn);
  spi_enable(spi_base);
}

void Spi::Disable() {
  spi_clean_disable(spi_base);
}

bool Spi::IsBusy() {
  return (SPI_SR(spi_base) & SPI_SR_BSY);
}

void Spi::BlockingRead(xfer_data_t* xfer) {
  // send address byte
  (void)spi_xfer(spi_base, xfer->tx_buffer[0]);

  for (uint8_t i = 0; i < xfer->num_bytes; i++) {
    // send dummy byte to keep clock going for each byte read
    xfer->rx_buffer[i] = spi_xfer(spi_base, 0x00);
  }
}

void Spi::BlockingWrite(xfer_data_t* xfer) {
  // send address byte
  (void)spi_xfer(spi_base, xfer->tx_buffer[0]);

  // start at one because address byte was already sent
  for (uint8_t i = 1; i < xfer->num_bytes; i++) {
    (void)spi_xfer(spi_base, xfer->tx_buffer[i]);
  }
}

uint8_t Spi::Read() {
  return SPI_DR(spi_base);
}

void Spi::Write(uint8_t data) {
  spi_write(spi_base, data);
}

void Spi::EnableInterrupts(uint8_t priority) {
  // spi_enable_tx_buffer_empty_interrupt(spi_base);
  spi_enable_rx_buffer_not_empty_interrupt(spi_base);
  spi_enable_error_interrupt(spi_base);
  // arm priorities go from 0 to 255 in increments of 16 (hence the shift)
  nvic_set_priority(irqn, (priority << 4));
  nvic_enable_irq(irqn);
}

void Spi::DisableInterrupts() {
  spi_disable_rx_buffer_not_empty_interrupt(spi_base);
  spi_disable_error_interrupt(spi_base);
  nvic_disable_irq(irqn);
}

};  // namespace spi
