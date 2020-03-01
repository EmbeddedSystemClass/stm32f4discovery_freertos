

#pragma once

#include "gpio.hpp"
extern "C" {
  #include "libopencm3/stm32/spi.h"
}

namespace spi {

enum class BaudRate {
  DIV_BY_2    = SPI_CR1_BAUDRATE_FPCLK_DIV_2,
  DIV_BY_4    = SPI_CR1_BAUDRATE_FPCLK_DIV_4,
  DIV_BY_8    = SPI_CR1_BAUDRATE_FPCLK_DIV_8,
  DIV_BY_16   = SPI_CR1_BAUDRATE_FPCLK_DIV_16,
  DIV_BY_32   = SPI_CR1_BAUDRATE_FPCLK_DIV_32,
  DIV_BY_64   = SPI_CR1_BAUDRATE_FPCLK_DIV_64,
  DIV_BY_128  = SPI_CR1_BAUDRATE_FPCLK_DIV_128,
  DIV_BY_256  = SPI_CR1_BAUDRATE_FPCLK_DIV_256
};

enum class Polarity {
  ZERO_WHEN_IDLE  = SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
  ONE_WHEN_IDLE   = SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
};

enum class Phase {
  FIRST_CLK_TRANS_FIRST_DATA  = SPI_CR1_CPHA_CLK_TRANSITION_1,
  SECOND_CLK_TRANS_FIRST_DATA = SPI_CR1_CPHA_CLK_TRANSITION_2,
};

enum class FrameFormat {
  EIGHT_BITS    = SPI_CR1_DFF_8BIT,
  SIXTEEN_BITS  = SPI_CR1_DFF_16BIT
};

enum class ByteOrder {
  MSB_FIRST = SPI_CR1_MSBFIRST,
  LSB_FIRST = SPI_CR1_LSBFIRST
};

enum class RetCode {
  SUCCESS,
  BAD_STATE,
  INVALID_PARAMS,
  BUSY
};

enum Event {
  IGNORE      = (1 << 0),
  TX_COMPLETE = (1 << 1),
  RX_COMPLETE = (1 << 2),
  FAULT       = (1 << 3),
  OVERRUN_ERR = (1 << 4),
  CRC_ERR     = (1 << 5),
};

enum class XferType {
  READ,
  WRITE,
  BLOCKING_READ,
  BLOCKING_WRITE,
};

typedef struct {
  uint8_t* rx_buffer;
  uint8_t* tx_buffer;
  uint8_t num_bytes;
} xfer_data_t;

typedef struct {
  xfer_data_t* xfer_data;
  XferType xfer_type;
  uint8_t current_byte;
} drv_xfer_t;

typedef struct {
  gpio::pin_t clk;
  gpio::pin_t miso;
  gpio::pin_t mosi;
  gpio::pin_t cs;
} pins_t;

typedef struct {
  BaudRate baud_rate;
  Polarity polarity;
  Phase phase;
  ByteOrder byte_order;
  FrameFormat frame_format;
  bool interrupt_driven;
} config_t;

};  // namespace spi
