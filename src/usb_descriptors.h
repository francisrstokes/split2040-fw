/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include "usb_common.h"

// defines
#define EP0_IN_ADDR     (USB_DIR_IN  | 0)
#define EP0_OUT_ADDR    (USB_DIR_OUT | 0)
#define EP1_IN_ADDR     (USB_DIR_IN  | 1)
#define EP2_IN_ADDR     (USB_DIR_IN  | 2)

#define KB_INTERFACE    (0)
#define CC_INTERFACE    (0)

// Public functions
const uint8_t* usb_get_hid_boot_keyboard_report_descriptor(void);
uint32_t usb_get_hid_boot_keyboard_report_descriptor_size(void);
const uint8_t* usb_get_hid_consumer_control_report_descriptor(void);
uint32_t usb_get_hid_consumer_control_report_descriptor_size(void);

const struct usb_endpoint_descriptor* usb_get_ep0_out_descriptor(void);
const struct usb_endpoint_descriptor* usb_get_ep0_in_descriptor(void);
const struct usb_endpoint_descriptor* usb_get_ep1_in_descriptor(void);
const struct usb_endpoint_descriptor* usb_get_ep2_in_descriptor(void);
const struct usb_device_descriptor* usb_get_device_descriptor(void);
const struct usb_interface_descriptor* usb_get_kb_interface_descriptor(void);
const struct usb_interface_descriptor* usb_get_cc_interface_descriptor(void);
const struct usb_hid_descriptor* usb_get_kb_hid_descriptor(void);
const struct usb_hid_descriptor* usb_get_cc_hid_descriptor(void);
const struct usb_configuration_descriptor* usb_get_configuration_descriptor(void);
const unsigned char* usb_get_lang_descriptor(void);
const unsigned char** usb_get_descriptor_strings(void);

#endif
