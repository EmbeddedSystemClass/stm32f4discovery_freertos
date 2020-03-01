

#pragma once

#include "etl/array.h"
#include "i2c1.hpp"
#include "cli.hpp"
extern "C" {
  #include "libopencm3/stm32/spi.h"
  #include <freertos/FreeRTOS.h>
  #include <freertos/portmacro.h>
  #include <freertos/FreeRTOS_CLI.h>
  #include <freertos/queue.h>
}

namespace cs43l22 {

constexpr uint8_t MAX_NUM_CMDS = 3;
constexpr uint8_t I2C_ADDR = 0x4A;
constexpr uint8_t QUEUE_SIZE = 3;
constexpr uint8_t RX_BUFFER_SIZE = 20;
constexpr uint8_t TX_BUFFER_SIZE = 20;
constexpr uint8_t MAX_ATTEMPTS = 3;

enum class RetCode {
  SUCCESS,
  BAD_STATE,
  INVALID_PARAMS,
  BUSY,
  TIMED_OUT
};

// Class used to communicate and configure the CS43L22 DAC over I2C1
//
// In its current state, this class only implements blocking read and write calls. It registers one CLI
// command: "get_id" and doesn't perform any configuration of the device. This was just for a sense of completion
class Cs43l22 {
 public:
  Cs43l22(i2c1::I2C1* i2c1, gpio::Gpio* port_d);
  void Init(Cs43l22* main_inst);
  RetCode ReadID(uint8_t* id);
  BaseType_t ProcessCliCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

 private:
  i2c1::I2C1* m_i2c;
  gpio::pin_t reset_pin;
  i2c::xfer_data_t xfer;
  i2c::config_t i2c_config;
  static const CLI_Command_Definition_t cli_command;
  etl::array<cli::cb_t, MAX_NUM_CMDS> commands;
  etl::array<uint8_t, RX_BUFFER_SIZE> rx_buffer;
  etl::array<uint8_t, TX_BUFFER_SIZE> tx_buffer;

  void CliReadID(char* pcWriteBuffer, size_t xWriteBufferLen);
  void CliHelp(char* pcWriteBuffer, size_t xWriteBufferLen);
  static BaseType_t CliCommandCallback(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
};

enum class Regs {
  CHIP_ID = 0x01,
};

};  // namespace cs43l22
