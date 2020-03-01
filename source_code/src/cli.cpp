

#include <string.h>
#include <stdio.h>
#include "cli.hpp"
#include "assert_err.hpp"
extern "C" {
  #include "freertos/FreeRTOS_CLI.h"
  #include "freertos/portmacro.h"
  #include "freertos/projdefs.h"
}

namespace cli {

constexpr uint8_t MAX_INPUT_LENGTH = 50;
constexpr uint8_t MAX_OUTPUT_LENGTH = 100;

const char NEW_LINE[] = "\n";
const char LINE_SEP[] = "\n[Press ENTER to execute the previous command again]\n> ";

void vUsbRxCallback(void *params);
void vUsbPoll(void *params);
BaseType_t TestCli(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

Cli::Cli(usb::Usb* usb) : m_usb(usb) {

}

void Cli::Init() {
  ASSERT_ERR(xTaskCreate(vUsbRxCallback, "UsbRxCallback", 500, this, 2, &xUsbRxCallback) == pdPASS);
  ASSERT_ERR(xTaskCreate(vUsbPoll, "UsbPoll", configMINIMAL_STACK_SIZE, this, 2, NULL) == pdPASS);
  m_usb->Init();
  m_usb->SetRxCallbackTask(&xUsbRxCallback);
}

void Cli::UsbPollTask() {
  while (1) {
    m_usb->Poll();
    vTaskDelay(10);
  }
}

void Cli::UsbRxCallbackTask() {
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(500);
  BaseType_t xResult;
  portBASE_TYPE xMoreToWrite;
  uint32_t ulNotifiedValue;
  uint8_t received_char;
  uint8_t input_index = 0;
  char output_string[MAX_OUTPUT_LENGTH] = {0};
  char input_string[MAX_INPUT_LENGTH] = {0};
  char last_input_string[MAX_INPUT_LENGTH] = {0};

  while (1) {
    // wait to receive notification from Rx Callback
    xResult = xTaskNotifyWait(pdFALSE, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);
    if (xResult == pdPASS) {
      // if a notification was received
      if (ulNotifiedValue == usb::CLI_NOTIFICATION) {
        // if notification was received from rx callback
        // try to claim access to usb peripheral
        if (xSemaphoreTake(m_usb->access_mutex, xMaxBlockTime) == pdTRUE) {
          // read the packet out (up to 64 bytes vs just one byte when using UART)
          m_usb->ProcessPacket();
          while (m_usb->IsCharAvailable()) {
            // read packet out character at a time for processing
            received_char = m_usb->ReadChar();
            if (received_char == '\r') {
              // command submitted, print a new line for readability
              m_usb->Write(NEW_LINE, strlen(NEW_LINE));
              // last command is repeated if nothing else entered
              if (input_index == 0) {
                strncpy(input_string, last_input_string, strlen(last_input_string));
              }

              // process command with FreeRTOS CLI and print output
              do {
                xMoreToWrite = FreeRTOS_CLIProcessCommand(input_string,
                                                      output_string,
                                                      MAX_OUTPUT_LENGTH);
                // write the output string and clear it out in case next string is shorter
                m_usb->Write(output_string, strlen(output_string));
                memset(output_string, 0x00, MAX_OUTPUT_LENGTH);
              } while (xMoreToWrite != pdFALSE);

              // clear last input just in case it was longer than current input
              memset(last_input_string, 0x00, MAX_INPUT_LENGTH);
              strncpy(last_input_string, input_string, strlen(input_string));
              input_index = 0;
              memset(input_string, 0x00, MAX_INPUT_LENGTH);

              m_usb->Write(LINE_SEP, strlen(LINE_SEP));
            } else if (received_char == '\n') {
              // ignore this character
            } else if (received_char == '\b') {
              // delete last character
              if (input_index > 0) {
                input_index--;
                input_string[input_index] = '\0';
              }
            } else {
              // normal character received
              if (input_index < MAX_INPUT_LENGTH) {
                input_string[input_index] = received_char;
                input_index++;
              }
            }
          }
          // release control of usb peripheral
          xSemaphoreGive(m_usb->access_mutex);
        }
      }
    }
  }
}


void vUsbRxCallback(void *params) {
  static_cast<cli::Cli *>(params)->UsbRxCallbackTask();
}

void vUsbPoll(void *params) {
  static_cast<cli::Cli *>(params)->UsbPollTask();
}

};  // namespace cli
