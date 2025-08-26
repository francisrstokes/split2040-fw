/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include "usb_common.h"
#include "hid.h"

#define EP0_OUT         (0)
#define EP0_IN          (1)
#define EP_KEYBOARD_IN  (2)

typedef void (*usb_ep_handler)(uint8_t *buf, uint16_t len);

const uint8_t hid_boot_keyboard_report_descriptor[] = {
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
    HID_USAGE(HID_USAGE_DESKTOP_KEYBOARD),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

    /* 8 bits Modifier Keys (Shift, Control, Alt) */
    HID_USAGE_PAGE(HID_USAGE_PAGE_KEYBOARD),
        HID_USAGE_MIN(224),
        HID_USAGE_MAX(231),
        HID_LOGICAL_MIN(0),
        HID_LOGICAL_MAX(1),
        HID_REPORT_COUNT(8),
        HID_REPORT_SIZE(1),
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

        /* 8 bit reserved */
        HID_REPORT_COUNT(1),
        HID_REPORT_SIZE(8),
        HID_INPUT(HID_CONSTANT),

        /* 6-byte Keycodes */
        HID_REPORT_COUNT(6),
        HID_REPORT_SIZE(8),
        HID_USAGE_MIN(0),
        HID_USAGE_MAX_N(255, 2),
        HID_LOGICAL_MIN(0),
        HID_LOGICAL_MAX_N( 255, 2),
        HID_INPUT(HID_DATA | HID_ARRAY | HID_ABSOLUTE),

    HID_COLLECTION_END
};

// Struct in which we keep the endpoint configuration
struct usb_endpoint_configuration {
    const struct usb_endpoint_descriptor *descriptor;
    usb_ep_handler handler;

    // Pointers to endpoint + buffer control registers
    // in the USB controller DPSRAM
    volatile uint32_t *endpoint_control;
    volatile uint32_t *buffer_control;
    volatile uint8_t *data_buffer;

    // Toggle after each packet (unless replying to a SETUP)
    uint8_t next_pid;
};

// Struct in which we keep the device configuration
struct usb_device_configuration {
    const struct usb_device_descriptor *device_descriptor;
    const struct usb_interface_descriptor *interface_descriptor;
    const struct usb_configuration_descriptor *config_descriptor;
    const struct usb_hid_descriptor *hid_descriptor;
    const uint8_t *hid_keyboard_report_descriptor;
    const unsigned char *lang_descriptor;
    const unsigned char **descriptor_strings;
    // USB num endpoints is 16
    struct usb_endpoint_configuration endpoints[USB_NUM_ENDPOINTS];
};

#define EP0_IN_ADDR  (USB_DIR_IN  | 0)
#define EP0_OUT_ADDR (USB_DIR_OUT | 0)
#define EP1_IN_ADDR  (USB_DIR_IN  | 1)

// EP0 IN and OUT
static const struct usb_endpoint_descriptor ep0_out = {
    .bLength          = sizeof(struct usb_endpoint_descriptor),
    .bDescriptorType  = USB_DT_ENDPOINT,
    .bEndpointAddress = EP0_OUT_ADDR, // EP number 0, OUT from host (rx to device)
    .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
    .wMaxPacketSize   = 64,
    .bInterval        = 0
};

static const struct usb_endpoint_descriptor ep0_in = {
    .bLength          = sizeof(struct usb_endpoint_descriptor),
    .bDescriptorType  = USB_DT_ENDPOINT,
    .bEndpointAddress = EP0_IN_ADDR, // EP number 0, OUT from host (rx to device)
    .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
    .wMaxPacketSize   = 64,
    .bInterval        = 0
};

// Descriptors
static const struct usb_device_descriptor device_descriptor = {
    .bLength         = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB          = 0x0110, // USB 1.1 device
    .bDeviceClass    = 0,      // Specified in interface descriptor
    .bDeviceSubClass = 0,      // No subclass
    .bDeviceProtocol = 0,      // No protocol
    .bMaxPacketSize0 = 64,     // Max packet size for ep0
    .idVendor        = 0x7083, // Your vendor id
    .idProduct       = 0x0002, // Your product ID
    .bcdDevice       = 0,      // No device revision number
    .iManufacturer   = 1,      // Manufacturer string index
    .iProduct        = 2,      // Product string index
    .iSerialNumber = 0,        // No serial number
    .bNumConfigurations = 1    // One configuration
};

static const struct usb_interface_descriptor interface_descriptor = {
    .bLength            = sizeof(struct usb_interface_descriptor),
    .bDescriptorType    = USB_DT_INTERFACE,
    .bInterfaceNumber   = 0,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 1,    // Just the keyboard report (no leds)
    .bInterfaceClass    = 0x03, // HID
    .bInterfaceSubClass = 0x01, // Boot subclass
    .bInterfaceProtocol = 0x01, // Keyboard
    .iInterface         = 0
};

static const struct usb_endpoint_descriptor ep1_in = {
    .bLength          = sizeof(struct usb_endpoint_descriptor),
    .bDescriptorType  = USB_DT_ENDPOINT,
    .bEndpointAddress = EP1_IN_ADDR, // EP number 1, IN from host (tx from device)
    .bmAttributes     = USB_TRANSFER_TYPE_INTERRUPT,
    .wMaxPacketSize   = 8,
    .bInterval        = 10 // 10ms / 100Hz
};

static const struct usb_hid_descriptor hid_descriptor = {
    .bLength = sizeof(struct usb_hid_descriptor),
    .bDescriptorType = HID_DESC_TYPE_HID,
    .bcdHID = 0x0111,         // HID 1.11
    .bCountryCode = 0,        // Not supported
    .bNumDescriptors = 1,     // We only have one descriptor (report)
    .bReportType = HID_DESC_TYPE_REPORT,
    .wReportLength = sizeof(hid_boot_keyboard_report_descriptor)
};

static const struct usb_configuration_descriptor config_descriptor = {
    .bLength         = sizeof(struct usb_configuration_descriptor),
    .bDescriptorType = USB_DT_CONFIG,
    .wTotalLength    = (sizeof(config_descriptor) +
                        sizeof(interface_descriptor) +
                        sizeof(hid_descriptor) +
                        sizeof(ep1_in)),
    .bNumInterfaces  = 1,
    .bConfigurationValue = 1, // Configuration 1
    .iConfiguration = 0,      // No string
    .bmAttributes = 0xa0,     // attributes: bus powered, remote wakeup
    .bMaxPower = 0xfa         // 500mA
};

static const unsigned char lang_descriptor[] = {
    4,         // bLength
    0x03,      // bDescriptorType == String Descriptor
    0x09, 0x04 // language id = us english
};

static const unsigned char *descriptor_strings[] = {
    (unsigned char *) "Francis Stokes", // Vendor
    (unsigned char *) "split2040-fw"    // Product
};

#endif
