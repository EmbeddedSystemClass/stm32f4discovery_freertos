

#include "lis3dsh.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "isr.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string>

namespace lis3dsh {

static Lis3dsh* p_lis_callback;
const TickType_t SpiTimeout = 100 / portTICK_PERIOD_MS;
const CLI_Command_Definition_t Lis3dsh::cli_command = {
  "accel",
  "Usage: accel command\n",
  &Lis3dsh::CliCommandCallback,
  -1
};

void vSpiInterruptHandler(void *params);

Lis3dsh::Lis3dsh(spi1::Spi1 *spi) : m_spi(spi),
  xfer({&rx_buffer[0], &tx_buffer[0], 0}),
  commands {{
    {"get_axes", etl::delegate<void(char* pcWriteBuffer, size_t xWriteBufferLen)>::create<Lis3dsh, &Lis3dsh::CliReadAxes>(*this)},
    {"get_id", etl::delegate<void(char* pcWriteBuffer, size_t xWriteBufferLen)>::create<Lis3dsh, &Lis3dsh::CliReadID>(*this)},
    {"help", etl::delegate<void(char* pcWriteBuffer, size_t xWriteBufferLen)>::create<Lis3dsh, &Lis3dsh::CliHelp>(*this)}
  }} {

}

void Lis3dsh::Init(Lis3dsh *p_main_inst) {
  spi::RetCode ret_code = spi::RetCode::SUCCESS;
  p_lis_callback = p_main_inst;

  queue = xQueueCreate(QUEUE_SIZE, sizeof(spi1::note_t));
  ASSERT_ERR(queue != NULL);

  spi_events = xEventGroupCreate();
  ASSERT_ERR(spi_events != NULL);

  ASSERT_ERR(FreeRTOS_CLIRegisterCommand(&cli_command) == pdPASS);
  ASSERT_ERR(xTaskCreate(vSpiInterruptHandler, "SpiInterruptHandler", configMINIMAL_STACK_SIZE, this, 2, NULL) == pdPASS);

  spi_config = {
    spi::BaudRate::DIV_BY_16,
    spi::Polarity::ZERO_WHEN_IDLE,
    spi::Phase::FIRST_CLK_TRANS_FIRST_DATA,
    spi::ByteOrder::MSB_FIRST,
    spi::FrameFormat::EIGHT_BITS,
    // interrupts used
    true,
  };

  ret_code = m_spi->Init(&spi_config, queue);
  ASSERT_RETURN(ret_code);
  ret_code = m_spi->Enable();
  ASSERT_RETURN(ret_code);

  Configure();
}

RetCode Lis3dsh::ReadID(uint8_t* id) {
  spi::RetCode ret_code = spi::RetCode::SUCCESS;
  RetCode lis_code = RetCode::SUCCESS;
  uint8_t attempts = 0;
  xfer.tx_buffer[0] = (static_cast<uint8_t>(Commands::READ) |
                        static_cast<uint8_t>(Regs::WHO_AM_I));
  xfer.num_bytes = 1;

  do {
    ret_code = m_spi->Read(&xfer);
    attempts++;
  } while (ret_code == spi::RetCode::BUSY && attempts < MAX_ATTEMPTS);
  if (ret_code != spi::RetCode::BUSY) {
    ASSERT_RETURN(ret_code);
    if (xEventGroupWaitBits(spi_events, spi::Event::RX_COMPLETE, pdTRUE, pdTRUE, SpiTimeout) & spi::Event::RX_COMPLETE) {
      // event received before timeout
      *id = xfer.rx_buffer[0];
    } else {
      // event wasn't received before timeout
      lis_code = RetCode::TIMED_OUT;
    }
  } else {
    lis_code = RetCode::BUSY;
  }

  return lis_code;
}

RetCode Lis3dsh::ReadAxes(axes_data_t* data) {
  spi::RetCode ret_code = spi::RetCode::SUCCESS;
  RetCode lis_code = RetCode::SUCCESS;
  xfer.tx_buffer[0] = (static_cast<uint8_t>(Commands::READ) |
                        static_cast<uint8_t>(Regs::OUT_X));
  xfer.num_bytes = 6;
  uint8_t attempts = 0;

  do {
    ret_code = m_spi->Read(&xfer);
    attempts++;
  } while (ret_code == spi::RetCode::BUSY && attempts < MAX_ATTEMPTS);
  if (ret_code != spi::RetCode::BUSY) {
    ASSERT_RETURN(ret_code);
    if (xEventGroupWaitBits(spi_events, spi::Event::RX_COMPLETE, pdTRUE, pdTRUE, SpiTimeout) & spi::Event::RX_COMPLETE) {
      // event received before timeout
      data->x_axis = static_cast<int16_t>((static_cast<uint16_t>(xfer.rx_buffer[1]) << 8) | xfer.rx_buffer[0]);
      data->y_axis = static_cast<int16_t>((static_cast<uint16_t>(xfer.rx_buffer[3]) << 8) | xfer.rx_buffer[2]);
      data->z_axis = static_cast<int16_t>((static_cast<uint16_t>(xfer.rx_buffer[5]) << 8) | xfer.rx_buffer[4]);
    } else {
      // event wasn't received before timeout
      lis_code = RetCode::TIMED_OUT;
    }
  } else {
    lis_code = RetCode::BUSY;
  }

  return lis_code;
}

BaseType_t Lis3dsh::ProcessCliCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
  const char *pcParameter;
  BaseType_t lParameterStringLength;
  uint8_t lParameterNumber = 1;
  bool cmd_found = false;

  pcParameter = FreeRTOS_CLIGetParameter(pcCommandString,
                                        lParameterNumber,
                                        &lParameterStringLength);
  for (auto i : commands) {
    if (strncmp(pcParameter, i.cmd_str, lParameterStringLength) == 0) {
      i.callback(pcWriteBuffer, xWriteBufferLen);
      cmd_found = true;
    }
  }

  if (!cmd_found) {
    // command wasn't found in listed options
    strncpy(pcWriteBuffer, "Invalid command. Try 'lis help' for a list of commands\n", xWriteBufferLen - 1);
    pcWriteBuffer[xWriteBufferLen - 1] = '\0';
  }

  return pdFALSE;
}

void Lis3dsh::SpiInterruptHandlerTask() {
  spi1::note_t spi1_msg;

  while (1) {
    if (xQueueReceive(queue, &spi1_msg, portMAX_DELAY) == pdTRUE) {
      // event received from queue
      xEventGroupSetBits(spi_events, spi1_msg.event);
    }
  }
}

void vSpiInterruptHandler(void *params) {
  static_cast<lis3dsh::Lis3dsh *>(params)->SpiInterruptHandlerTask();
}

void Lis3dsh::CliReadAxes(char* pcWriteBuffer, size_t xWriteBufferLen) {
  axes_data_t data;
  float xaxis, yaxis, zaxis;
  RetCode ret_code;

  ret_code = ReadAxes(&data);
  if (ret_code == RetCode::SUCCESS) {
    Calculate1g(data, &xaxis, &yaxis, &zaxis);
    snprintf(pcWriteBuffer, xWriteBufferLen, "x-axis: %.4f\ny-axis: %.4f\nz-axis: %.4f\n", xaxis, yaxis, zaxis);
  } else {
    snprintf(pcWriteBuffer, xWriteBufferLen, "Error code: %d\n", static_cast<uint8_t>(ret_code));
  }
}

void Lis3dsh::CliReadID(char* pcWriteBuffer, size_t xWriteBufferLen) {
  uint8_t id;
  RetCode ret_code;

  ret_code = ReadID(&id);
  if (ret_code == RetCode::SUCCESS) {
    snprintf(pcWriteBuffer, xWriteBufferLen, "ID: %.2x\n", id);
  } else {
    snprintf(pcWriteBuffer, xWriteBufferLen, "Error code: %d\n", static_cast<uint8_t>(ret_code));
  }
}

void Lis3dsh::CliHelp(char* pcWriteBuffer, size_t xWriteBufferLen) {
  snprintf(pcWriteBuffer, xWriteBufferLen, "Possible commands:\n");
  for (auto i : commands) {
    if (strncmp(i.cmd_str, "help", strlen(i.cmd_str)) != 0) {
      strncat(pcWriteBuffer, "- ", strlen("- "));
      strncat(pcWriteBuffer, i.cmd_str, strlen(i.cmd_str));
      strncat(pcWriteBuffer, "\n", strlen("\n"));
    }
  }
}

void Lis3dsh::Configure() {
  spi::RetCode ret_code = spi::RetCode::SUCCESS;
  xfer.tx_buffer[0] = (static_cast<uint8_t>(Commands::WRITE) |
                        static_cast<uint8_t>(Regs::CTRL_REG4));
  // ctrl_reg4: ODR = 50Hz, x,y,z enabled, block data update
  xfer.tx_buffer[1] = 0x5F;
  xfer.num_bytes = 2;
  ret_code = m_spi->BlockingWrite(&xfer);
  ASSERT_RETURN(ret_code);

  xfer.tx_buffer[0] = (static_cast<uint8_t>(Commands::WRITE) |
                        static_cast<uint8_t>(Regs::CTRL_REG5));
  // ctrl_reg5: Scale = 4g
  xfer.tx_buffer[1] = 0x08;
  xfer.num_bytes = 2;
  ret_code = m_spi->BlockingWrite(&xfer);
  ASSERT_RETURN(ret_code);
}

void Lis3dsh::Calculate1g(axes_data_t data, float* x, float* y, float* z) {
  *x = (static_cast<float>(data.x_axis) * G_SCALE) / (1 << 15);
  *y = (static_cast<float>(data.y_axis) * G_SCALE) / (1 << 15);
  *z = (static_cast<float>(data.z_axis) * G_SCALE) / (1 << 15);
}

BaseType_t Lis3dsh::CliCommandCallback(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
  return p_lis_callback->ProcessCliCommand(pcWriteBuffer, xWriteBufferLen, pcCommandString);
}

};  // namespace lis3dsh
