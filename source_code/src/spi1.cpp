

#include "spi1.hpp"
#include "assert_err.hpp"
#include <stdlib.h>
#include "isr.hpp"
extern "C" {
  #include <libopencm3/stm32/rcc.h>
  #include <libopencm3/cm3/nvic.h>
  #include "freertos/FreeRTOS.h"
  #include "freertos/portmacro.h"
  #include "freertos/projdefs.h"
  #include "freertos/message_buffer.h"
}

namespace spi1 {

constexpr uint32_t spi1_base = SPI1;
constexpr uint32_t spi1_irqn = NVIC_SPI1_IRQ;
constexpr uint32_t spi1_irq_priority = 6;
volatile bool first_value = true;

Spi1::Spi1(gpio::Gpio *port_a, gpio::Gpio *port_e) : Spi(spi1_base, spi1_irqn),
                    callback(etl::delegate<void(size_t)>::create<Spi1, &Spi1::InterruptHandler>(*this)),
                    current_state(State::DISABLED) {
  // spi1 pin/port configuration
  pins.clk = {GPIO5, port_a};
  pins.miso = {GPIO6, port_a};
  pins.mosi = {GPIO7, port_a};
  pins.cs = {GPIO3, port_e};
  GetInterruptVectorsInstance().register_delegate(static_cast<uint8_t>(Callbacks::Spi1), callback);
}

spi::RetCode Spi1::Init(spi::config_t* cfg, QueueHandle_t queue) {
  spi::RetCode ret_code = spi::RetCode::SUCCESS;
  switch (current_state) {
    case State::DISABLED:
      config = *cfg;

      if (config.interrupt_driven) {
        if (queue == nullptr) {
          ret_code = spi::RetCode::INVALID_PARAMS;
          return ret_code;
        }
        observer_queue = queue;
      }

      rcc_periph_clock_enable(RCC_SPI1);
      pins.cs.port->SetPins(pins.cs.pin_num);
      pins.cs.port->ModeSetup(GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pins.cs.pin_num);
      pins.cs.port->SetOutputOptions(GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pins.cs.pin_num);
      pins.clk.port->ModeSetup(GPIO_MODE_AF, GPIO_PUPD_NONE, pins.clk.pin_num);
      pins.clk.port->SetAltFunction(GPIO_AF5, pins.clk.pin_num);
      pins.clk.port->SetOutputOptions(GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pins.clk.pin_num);
      pins.miso.port->ModeSetup(GPIO_MODE_AF, GPIO_PUPD_NONE, pins.miso.pin_num);
      pins.miso.port->SetAltFunction(GPIO_AF5, pins.miso.pin_num);
      pins.miso.port->SetOutputOptions(GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pins.miso.pin_num);
      pins.mosi.port->ModeSetup(GPIO_MODE_AF, GPIO_PUPD_NONE, pins.mosi.pin_num);
      pins.mosi.port->SetAltFunction(GPIO_AF5, pins.mosi.pin_num);
      pins.mosi.port->SetOutputOptions(GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pins.mosi.pin_num);

      Spi::Init(&config);
      current_state = State::INITIALIZED;
      break;

    default:
      ret_code = spi::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

spi::RetCode Spi1::Enable() {
  spi::RetCode ret_code = spi::RetCode::SUCCESS;
  switch (current_state) {
    case State::INITIALIZED:
      Spi::Enable();
      current_state = State::READY;
      break;

    default:
      ret_code = spi::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

spi::RetCode Spi1::Disable() {
  spi::RetCode ret_code = spi::RetCode::SUCCESS;
  switch (current_state) {
    case State::INITIALIZED:
      Spi::Disable();
      current_state = State::DISABLED;
      break;

    default:
      ret_code = spi::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

spi::RetCode Spi1::BlockingRead(spi::xfer_data_t* xfer) {
  spi::RetCode ret_code = spi::RetCode::SUCCESS;
  switch (current_state) {
    case State::READY:
      current_state = State::BUSY;
      Spi::DisableInterrupts();
      pins.cs.port->ClearPins(pins.cs.pin_num);
      Spi::BlockingRead(xfer);
      pins.cs.port->SetPins(pins.cs.pin_num);
      current_state = State::READY;
      break;

    case State::BUSY:
      ret_code = spi::RetCode::BUSY;
      break;

    default:
      ret_code = spi::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

spi::RetCode Spi1::Read(spi::xfer_data_t *xfer) {
  spi::RetCode ret_code = spi::RetCode::SUCCESS;
  switch (current_state) {
    case State::READY:
      current_state = State::BUSY;
      Spi::EnableInterrupts(spi1_irq_priority);
      pins.cs.port->ClearPins(pins.cs.pin_num);
      current_xfer.xfer_data = xfer;
      current_xfer.xfer_type = spi::XferType::READ;
      current_xfer.current_byte = 0;
      Spi::Write(current_xfer.xfer_data->tx_buffer[0]);
      break;

    case State::BUSY:
      ret_code = spi::RetCode::BUSY;
      break;

    default:
      ret_code = spi::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

spi::RetCode Spi1::BlockingWrite(spi::xfer_data_t* xfer) {
  spi::RetCode ret_code = spi::RetCode::SUCCESS;
  switch (current_state) {
    case State::READY:
      current_state = State::BUSY;
      pins.cs.port->ClearPins(pins.cs.pin_num);
      Spi::DisableInterrupts();
      Spi::BlockingWrite(xfer);
      pins.cs.port->SetPins(pins.cs.pin_num);
      current_state = State::READY;
      break;

    case State::BUSY:
      ret_code = spi::RetCode::BUSY;
      break;

    default:
      ret_code = spi::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

spi::RetCode Spi1::Write(spi::xfer_data_t *xfer) {
  spi::RetCode ret_code = spi::RetCode::SUCCESS;
  switch (current_state) {
    case State::READY:
      current_state = State::BUSY;
      Spi::EnableInterrupts(spi1_irq_priority);
      pins.cs.port->ClearPins(pins.cs.pin_num);
      current_xfer.xfer_data = xfer;
      current_xfer.xfer_type = spi::XferType::WRITE;
      current_xfer.current_byte = 1;
      Spi::Write(current_xfer.xfer_data->tx_buffer[0]);
      break;

    case State::BUSY:
      ret_code = spi::RetCode::BUSY;
      break;

    default:
      ret_code = spi::RetCode::BAD_STATE;
      break;
  }

  return ret_code;
}

void Spi1::InterruptHandler(const size_t id) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  // check the interrupt flags to see what event occured
  spi_note.event = spi::Event::IGNORE;
  if (SPI_SR(spi1_base) & SPI_SR_RXNE) {
    if (current_xfer.xfer_type == spi::XferType::READ) {
      if (first_value) {
        (void)Spi::Read();
        Spi::Write(0x00);
        first_value = false;
      } else {
        current_xfer.xfer_data->rx_buffer[current_xfer.current_byte] = Spi::Read();
        current_xfer.current_byte++;
        if (current_xfer.current_byte < current_xfer.xfer_data->num_bytes) {
          Spi::Write(0x00);
        } else {
          first_value = true;
          pins.cs.port->SetPins(pins.cs.pin_num);
          spi_note.event = spi::Event::RX_COMPLETE;
          current_state = State::READY;
        }
      }
    } else if (current_xfer.xfer_type == spi::XferType::WRITE) {
      (void)Spi::Read();
      if (current_xfer.current_byte < current_xfer.xfer_data->num_bytes) {
        Spi::Write(current_xfer.xfer_data->tx_buffer[current_xfer.current_byte]);
        current_xfer.current_byte++;
      } else {
        pins.cs.port->SetPins(pins.cs.pin_num);
        spi_note.event = spi::Event::TX_COMPLETE;
        current_state = State::READY;
      }
    }
  } else if (SPI_SR(spi1_base) & SPI_SR_MODF) {
    spi_note.event = spi::Event::FAULT;
    current_state = State::READY;
  } else if (SPI_SR(spi1_base) & SPI_SR_OVR) {
    spi_note.event = spi::Event::OVERRUN_ERR;
    current_state = State::READY;
  } else if (SPI_SR(spi1_base) & SPI_SR_CRCERR) {
    spi_note.event = spi::Event::CRC_ERR;
    current_state = State::READY;
  }

  if (spi_note.event != spi::Event::IGNORE) {
    xQueueSendFromISR(observer_queue, &spi_note, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

};  // namespace spi1
