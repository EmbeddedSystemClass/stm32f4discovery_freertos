

#pragma once

extern "C" {
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>
  #include <freertos/semphr.h>
  #include "cdc_acm.h"
}
#include "ring_buffer.hpp"
#include "gpio.hpp"

namespace usb {

constexpr uint8_t PACKET_SIZE = 64;
constexpr uint8_t CLI_NOTIFICATION = 0x01;

// Class that acts as an interface for the USB CDC ACM driver
//
// When a packet is received by the driver, the callback task is called and the CLI is notified that data
// is available. The CLI then calls the process packet function to read out the contents and the length of data.
// Data is then read out one byte at a time for processing. Writes also need to be performed 64 bytes at a time
// based on how the driver is configured so looping/flushing is done until the write is complete.
class Usb {
 public:
  SemaphoreHandle_t access_mutex = NULL;

  explicit Usb(gpio::Gpio* port_a);
  void Init();
  void Write(const char* buf, uint16_t len);
  void Poll();
  void ProcessPacket();
  char ReadChar();
  bool IsCharAvailable();
  void SetRxCallbackTask(TaskHandle_t* handle);

 private:
  gpio::Gpio* port_a;
  char packet_buffer[PACKET_SIZE];
  ring_buf::RingBuffer<char, PACKET_SIZE> data_buffer;
  uint8_t packet_buffer_bytes = 0;
  uint8_t byte_ctr = 0;
  static TaskHandle_t buff_handler;
  static usbd_device* m_usbd_dev;

  static void SetConfig(usbd_device *usbd_dev, uint16_t wValue);
  static void RxCallback(usbd_device *usbd_dev, uint8_t ep);
  static enum usbd_request_return_codes ControlRequestCallback(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
                                                                uint16_t *len, void (**complete)(usbd_device *usbd_dev,
                                                                struct usb_setup_data *req));
};

};  // namespace usb
