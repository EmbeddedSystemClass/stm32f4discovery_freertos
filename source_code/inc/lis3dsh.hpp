

#pragma once

#include "etl/array.h"
#include "etl/delegate.h"
#include "etl/observer.h"
#include "cli.hpp"
#include "spi1.hpp"
extern "C" {
  #include "libopencm3/stm32/spi.h"
  #include <freertos/FreeRTOS.h>
  #include <freertos/portmacro.h>
  #include <freertos/FreeRTOS_CLI.h>
  #include <freertos/queue.h>
  #include <freertos/event_groups.h>
}

namespace lis3dsh {

constexpr uint8_t MAX_CMD_SIZE = 20;
constexpr uint8_t MAX_NUM_CMDS = 3;
constexpr uint8_t ID = 0x3B;
constexpr uint8_t G_SCALE = 4;
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

typedef struct {
  int16_t x_axis;
  int16_t y_axis;
  int16_t z_axis;
} axes_data_t;

// Class used to communicate with LIS3DSH accelerometer with Spi1
//
// Both blocking and interrupt-driven reads and writes are available. Commands are available through the CLI to read
// the device ID and the x, y, and z axes. After an interrupt-driven transfer is initiated, the device waits for the
// appropriate bits to be set in an event group (i.e. RX_COMPLETE). When the transaction is finished, the Spi1 class sends
// a queue message with the result to the InterruptHandlerTask here (which is just waiting for this event). When received,
// this task set the appropriate bits in the class' event group and the device wakes up from waiting (or a timeout occurs
// if something went wrong)
class Lis3dsh {
 public:
  explicit Lis3dsh(spi1::Spi1* spi1);
  void Init(Lis3dsh* main_inst);
  RetCode ReadAxes(axes_data_t* data);
  RetCode ReadID(uint8_t* id);
  BaseType_t ProcessCliCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
  void SpiInterruptHandlerTask();

 private:
  spi1::Spi1* m_spi;
  spi::xfer_data_t xfer;
  spi::config_t spi_config;
  EventGroupHandle_t spi_events;
  static const CLI_Command_Definition_t cli_command;
  etl::array<cli::cb_t, MAX_NUM_CMDS> commands;
  QueueHandle_t queue;
  etl::array<uint8_t, RX_BUFFER_SIZE> rx_buffer;
  etl::array<uint8_t, TX_BUFFER_SIZE> tx_buffer;

  void Configure();
  void Calculate1g(axes_data_t data, float* x, float* y, float* z);
  void CliReadAxes(char* pcWriteBuffer, size_t xWriteBufferLen);
  void CliReadID(char* pcWriteBuffer, size_t xWriteBufferLen);
  void CliHelp(char* pcWriteBuffer, size_t xWriteBufferLen);
  static BaseType_t CliCommandCallback(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
};

enum class Regs {
  WHO_AM_I = 0x0F,
  THRS3 = 0x1F,
  CTRL_REG4 = 0x20,
  CTRL_REG1 = 0x21,
  CTRL_REG2 = 0x22,
  CTRL_REG3 = 0x23,
  CTRL_REG5 = 0x24,
  CTRL_REG6 = 0x25,
  STATUS = 0x27,
  OUT_X = 0x28,
  OUT_Y = 0x2A,
  OUT_Z = 0x2C,
};

enum class Commands {
  READ = 0x80,
  WRITE = 0x00,
};

};  // namespace lis3dsh
