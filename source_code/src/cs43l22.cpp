

#include "cs43l22.hpp"
#include "assert_err.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "isr.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string>

namespace cs43l22 {

static Cs43l22* p_cs_callback;
const TickType_t SpiTimeout = 100 / portTICK_PERIOD_MS;
const CLI_Command_Definition_t Cs43l22::cli_command = {
  "dac",
  "Usage: dac command\n",
  &Cs43l22::CliCommandCallback,
  -1
};

void vSpiInterruptHandler(void *params);

Cs43l22::Cs43l22(i2c1::I2C1 *i2c, gpio::Gpio* port_d) : m_i2c(i2c),
  xfer({&rx_buffer[0], &tx_buffer[0], 0, I2C_ADDR}),
  commands {{
    {"get_id", etl::delegate<void(char* pcWriteBuffer, size_t xWriteBufferLen)>::create<Cs43l22, &Cs43l22::CliReadID>(*this)},
    {"help", etl::delegate<void(char* pcWriteBuffer, size_t xWriteBufferLen)>::create<Cs43l22, &Cs43l22::CliHelp>(*this)}
  }} {
  // initialize the reset pin connected to the device
  reset_pin = {GPIO4, port_d};
}

void Cs43l22::Init(Cs43l22 *p_main_inst) {
  i2c::RetCode ret_code = i2c::RetCode::SUCCESS;
  p_cs_callback = p_main_inst;

  ASSERT_ERR(FreeRTOS_CLIRegisterCommand(&cli_command) == pdPASS);

  i2c_config = {
    i2c::Speed::SM_100K,
    i2c::ClockFreq::FREQ_8MHZ
  };

  ret_code = m_i2c->Init(&i2c_config);
  ASSERT_RETURN(ret_code);
  ret_code = m_i2c->Enable();
  ASSERT_RETURN(ret_code);

  // take the device out of reset
  reset_pin.port->SetPins(reset_pin.pin_num);
  reset_pin.port->ModeSetup(GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, reset_pin.pin_num);
  reset_pin.port->SetOutputOptions(GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, reset_pin.pin_num);
}

RetCode Cs43l22::ReadID(uint8_t* id) {
  i2c::RetCode ret_code = i2c::RetCode::SUCCESS;
  RetCode cs_code = RetCode::SUCCESS;
  uint8_t attempts = 0;
  xfer.tx_buffer[0] = static_cast<uint8_t>(Regs::CHIP_ID);
  xfer.num_bytes = 1;

  do {
    ret_code = m_i2c->BlockingRead(&xfer);
    attempts++;
  } while (ret_code == i2c::RetCode::BUSY && attempts < MAX_ATTEMPTS);
  if (ret_code != i2c::RetCode::BUSY) {
    ASSERT_RETURN(ret_code);
    *id = xfer.rx_buffer[0];
  } else {
    cs_code = RetCode::BUSY;
  }

  return cs_code;
}

BaseType_t Cs43l22::ProcessCliCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
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

void Cs43l22::CliReadID(char* pcWriteBuffer, size_t xWriteBufferLen) {
  uint8_t id;
  RetCode ret_code;

  ret_code = ReadID(&id);
  if (ret_code == RetCode::SUCCESS) {
    snprintf(pcWriteBuffer, xWriteBufferLen, "ID: %.2x\n", id);
  } else {
    snprintf(pcWriteBuffer, xWriteBufferLen, "Error code: %d\n", static_cast<uint8_t>(ret_code));
  }
}

void Cs43l22::CliHelp(char* pcWriteBuffer, size_t xWriteBufferLen) {
  snprintf(pcWriteBuffer, xWriteBufferLen, "Possible commands:\n");
  for (auto i : commands) {
    if (strncmp(i.cmd_str, "help", strlen(i.cmd_str)) != 0) {
      strncat(pcWriteBuffer, "- ", strlen("- "));
      strncat(pcWriteBuffer, i.cmd_str, strlen(i.cmd_str));
      strncat(pcWriteBuffer, "\n", strlen("\n"));
    }
  }
}

BaseType_t Cs43l22::CliCommandCallback(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
  return p_cs_callback->ProcessCliCommand(pcWriteBuffer, xWriteBufferLen, pcCommandString);
}

};  // namespace cs43l22
