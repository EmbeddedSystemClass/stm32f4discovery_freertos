

#pragma once

#include <libopencm3/usb/usbd.h>

// Creates the usb device structure to be used with the libopencm3 cdc_acm driver.
//
// Output: usbd device structure
usbd_device* init_usb();
