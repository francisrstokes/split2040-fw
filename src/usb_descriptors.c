#include "usb_descriptors.h"
#include "hid.h"

// defines
#define EP0_IN_ADDR     (USB_DIR_IN  | 0)
#define EP0_OUT_ADDR    (USB_DIR_OUT | 0)
#define EP1_IN_ADDR     (USB_DIR_IN  | 1)

// static const structures
static const uint8_t hid_boot_keyboard_report_descriptor[] = {
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
    HID_USAGE(HID_USAGE_DESKTOP_KEYBOARD),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

    // 8 bits Modifier Keys (Shift, Control, Alt)
    HID_USAGE_PAGE(HID_USAGE_PAGE_KEYBOARD),
        HID_USAGE_MIN(224),
        HID_USAGE_MAX(231),
        HID_LOGICAL_MIN(0),
        HID_LOGICAL_MAX(1),
        HID_REPORT_COUNT(8),
        HID_REPORT_SIZE(1),
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

        // 8 bit reserved
        HID_REPORT_COUNT(1),
        HID_REPORT_SIZE(8),
        HID_INPUT(HID_CONSTANT),

        // 6-byte Keycodes
        HID_REPORT_COUNT(6),
        HID_REPORT_SIZE(8),
        HID_USAGE_MIN(0),
        HID_USAGE_MAX_N(255, 2),
        HID_LOGICAL_MIN(0),
        HID_LOGICAL_MAX_N( 255, 2),
        HID_INPUT(HID_DATA | HID_ARRAY | HID_ABSOLUTE),

    // 5 bit led status
    HID_USAGE_PAGE(HID_USAGE_PAGE_LED),
        HID_USAGE_MIN(1),
        HID_USAGE_MAX(5),
        HID_REPORT_COUNT(5),
        HID_REPORT_SIZE(1),
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

        // 3 bit reserved
        HID_REPORT_COUNT(1),
        HID_REPORT_SIZE(3),
        HID_OUTPUT(HID_CONSTANT),

    HID_COLLECTION_END
};

// EP0 IN and OUT
static const struct usb_endpoint_descriptor ep0_out = {
    .bLength          = sizeof(struct usb_endpoint_descriptor),
    .bDescriptorType  = USB_DT_ENDPOINT,
    .bEndpointAddress = EP0_OUT_ADDR, // EP number 0, OUT from host (rx to device)
    .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
    .wMaxPacketSize   = 64,
    .bInterval        = 0
};

const struct usb_endpoint_descriptor ep0_in = {
    .bLength          = sizeof(struct usb_endpoint_descriptor),
    .bDescriptorType  = USB_DT_ENDPOINT,
    .bEndpointAddress = EP0_IN_ADDR, // EP number 0, OUT from host (rx to device)
    .bmAttributes     = USB_TRANSFER_TYPE_CONTROL,
    .wMaxPacketSize   = 64,
    .bInterval        = 0
};

// Descriptors
const struct usb_device_descriptor device_descriptor = {
    .bLength         = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB          = 0x0110, // USB 1.1 device
    .bDeviceClass    = 0,      // Specified in interface descriptor
    .bDeviceSubClass = 0,      // No subclass
    .bDeviceProtocol = 0,      // No protocol
    .bMaxPacketSize0 = 64,     // Max packet size for ep0
    .idVendor        = 0x7083, // Your vendor id
    .idProduct       = 0x0003, // Your product ID
    .bcdDevice       = 0,      // No device revision number
    .iManufacturer   = 1,      // Manufacturer string index
    .iProduct        = 2,      // Product string index
    .iSerialNumber = 0,        // No serial number
    .bNumConfigurations = 1    // One configuration
};

const struct usb_interface_descriptor interface_descriptor = {
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

const struct usb_endpoint_descriptor ep1_in = {
    .bLength          = sizeof(struct usb_endpoint_descriptor),
    .bDescriptorType  = USB_DT_ENDPOINT,
    .bEndpointAddress = EP1_IN_ADDR, // EP number 1, IN from host (tx from device)
    .bmAttributes     = USB_TRANSFER_TYPE_INTERRUPT,
    .wMaxPacketSize   = 8,
    .bInterval        = 10 // 10ms / 100Hz
};

const struct usb_hid_descriptor hid_descriptor = {
    .bLength = sizeof(struct usb_hid_descriptor),
    .bDescriptorType = HID_DESC_TYPE_HID,
    .bcdHID = 0x0111,         // HID 1.11
    .bCountryCode = 0,        // Not supported
    .bNumDescriptors = 1,     // We only have one descriptor (report)
    .bReportType = HID_DESC_TYPE_REPORT,
    .wReportLength = sizeof(hid_boot_keyboard_report_descriptor)
};

const struct usb_configuration_descriptor config_descriptor = {
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

const uint8_t lang_descriptor[] = {
    4,         // bLength
    0x03,      // bDescriptorType == String Descriptor
    0x09, 0x04 // language id = us english
};

const unsigned char* descriptor_strings[] = {
    (unsigned char*) "Francis Stokes",          // Vendor
    (unsigned char*) "Hex-2a Split Keyboard"    // Product
};

// public functions
const uint8_t* usb_get_hid_boot_keyboard_report_descriptor(void) {
    return (uint8_t*)hid_boot_keyboard_report_descriptor;
}

uint32_t usb_get_hid_boot_keyboard_report_descriptor_size(void) {
    return sizeof(hid_boot_keyboard_report_descriptor);
}

const struct usb_endpoint_descriptor* usb_get_ep0_out_descriptor(void) {
    return &ep0_out;
}

const struct usb_endpoint_descriptor* usb_get_ep0_in_descriptor(void) {
    return &ep0_in;
}

const struct usb_endpoint_descriptor* usb_get_ep1_in_descriptor(void) {
    return &ep1_in;
}

const struct usb_device_descriptor* usb_get_device_descriptor(void) {
    return &device_descriptor;
}

const struct usb_interface_descriptor* usb_get_interface_descriptor(void) {
    return &interface_descriptor;
}

const struct usb_hid_descriptor* usb_get_hid_descriptor(void) {
    return &hid_descriptor;
}

const struct usb_configuration_descriptor* usb_get_configuration_descriptor(void) {
    return &config_descriptor;
}

const uint8_t* usb_get_lang_descriptor(void) {
    return lang_descriptor;
}

const unsigned char** usb_get_descriptor_strings(void) {
    return descriptor_strings;
}
