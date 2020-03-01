

#include <stdlib.h>
#include "gpio.hpp"
#include "usb.hpp"
extern "C" {
  #include <freertos/portmacro.h>
  #include <libopencm3/usb/cdc.h>
  #include <libopencm3/stm32/rcc.h>
}

namespace usb {

constexpr uint8_t IN_EP = 0x01;
constexpr uint8_t OUT_EP = 0x82;
constexpr uint8_t CTRL_EP = 0x83;

TaskHandle_t Usb::buff_handler;
usbd_device* Usb::m_usbd_dev = nullptr;

Usb::Usb(gpio::Gpio* port_a) : port_a(port_a) {

}

void Usb::Init() {
  m_usbd_dev = init_usb();
  usbd_register_set_config_callback(m_usbd_dev, SetConfig);
  rcc_periph_clock_enable(RCC_OTGFS);
  port_a->ModeSetup(GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
  port_a->SetAltFunction(GPIO_AF10, GPIO11 | GPIO12);
  access_mutex = xSemaphoreCreateMutex();
}

void Usb::Write(const char* buf, uint16_t len) {
  int16_t bytes_left = len;
  uint8_t index_offset = 0;
  char temp_buf[PACKET_SIZE] = {0};
  do {
    if (bytes_left > PACKET_SIZE) {
      strncpy(temp_buf, &buf[index_offset], PACKET_SIZE);
      index_offset += PACKET_SIZE;
    } else {
      memset(temp_buf, 0, PACKET_SIZE);
      strncpy(temp_buf, &buf[index_offset], bytes_left);
    }
    while (usbd_ep_write_packet(m_usbd_dev, OUT_EP, temp_buf, PACKET_SIZE) == 0) {}
    bytes_left -= PACKET_SIZE;
  } while (bytes_left > 0);

  // I don't understand why but if this isn't here, packets are sometimes not sent and they
  // will be sent in the next call to Write. Some kind of flushing mechanism.
  memset(temp_buf, 0, PACKET_SIZE);
  while (usbd_ep_write_packet(m_usbd_dev, OUT_EP, temp_buf, PACKET_SIZE) == 0) {}
}

void Usb::Poll() {
  usbd_poll(m_usbd_dev);
}

void Usb::ProcessPacket() {
  packet_buffer_bytes = usbd_ep_read_packet(m_usbd_dev, IN_EP, packet_buffer, PACKET_SIZE);
  for (uint8_t i = 0; i < packet_buffer_bytes; i++) {
    data_buffer.Put(packet_buffer[i]);
  }
  byte_ctr = 0;
}

char Usb::ReadChar() {
  char ret_val = data_buffer.Get();
  byte_ctr++;
  return ret_val;
}

bool Usb::IsCharAvailable() {
  if (byte_ctr == packet_buffer_bytes) {
    return false;
  } else {
    return true;
  }
}

void Usb::SetRxCallbackTask(TaskHandle_t *handle) {
  buff_handler = *handle;
}

void Usb::SetConfig(usbd_device *usbd_dev, uint16_t wValue) {
  (void)wValue;

  usbd_ep_setup(usbd_dev, IN_EP, USB_ENDPOINT_ATTR_BULK, PACKET_SIZE,
      RxCallback);
  usbd_ep_setup(usbd_dev, OUT_EP, USB_ENDPOINT_ATTR_BULK, PACKET_SIZE, NULL);
  usbd_ep_setup(usbd_dev, CTRL_EP, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

  usbd_register_control_callback(
        m_usbd_dev,
        USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        ControlRequestCallback);
}

enum usbd_request_return_codes Usb::ControlRequestCallback(usbd_device *usbd_dev,
  struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
  void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req)) {

  (void)complete;
  (void)buf;
  (void)usbd_dev;

  switch (req->bRequest) {
  case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
    /*
     * This Linux cdc_acm driver requires this to be implemented
     * even though it's optional in the CDC spec, and we don't
     * advertise it in the ACM functional descriptor.
     */
    return USBD_REQ_HANDLED;
    }
  case USB_CDC_REQ_SET_LINE_CODING:
    if (*len < sizeof(struct usb_cdc_line_coding)) {
      return USBD_REQ_NOTSUPP;
    }

    return USBD_REQ_HANDLED;
  }
  return USBD_REQ_NOTSUPP;
}

void Usb::RxCallback(usbd_device *usbd_dev, uint8_t ep) {
  (void)ep;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  xTaskNotifyFromISR(buff_handler, CLI_NOTIFICATION, eSetBits, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

};  // namespace usb
