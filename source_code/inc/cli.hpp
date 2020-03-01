

#pragma once

#include "usb.hpp"
#include "assert_err.hpp"
#include "etl/delegate.h"
extern "C" {
  #include "freertos/FreeRTOS.h"
}

namespace cli {

constexpr uint8_t MAX_CMD_SIZE = 20;

// callback type containing a string and callback function for modules using CLI
typedef struct {
  char cmd_str[MAX_CMD_SIZE];
  etl::delegate<void(char* pcWriteBuffer, size_t xWriteBufferLen)> callback;
} cb_t;

// CLI class that acts as an interface between the USB CDC ACM driver and the FreeRTOS CLI
//
// This class creates two tasks to handle data throughput: one for polling and another for receiving data.
// The USB poll task is just periodically called for data transmission (this is how the driver is setup by libopencm3).
// When the USB driver receives data, it calls a registered callback function in the USB class. This function then notifies
// the USBRxCallback Task here and data is read out of the USB data buffer one character at a time. Once a carriage return
// is received, the command is processed by the FreeRTOS CLI and if that command exists, the command callback is executed.
// The results of this function (and an error message if the command was invalid) are printed afterwards.
class Cli {
 public:
  explicit Cli(usb::Usb* usb);
  void Init();
  void UsbPollTask();
  void UsbRxCallbackTask();

 private:
  usb::Usb* m_usb;
  TaskHandle_t xUsbRxCallback  = NULL;
};

};  // namespace cli
