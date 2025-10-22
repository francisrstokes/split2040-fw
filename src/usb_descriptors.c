#include "usb_descriptors.h"
#include "hid.h"

#include "keyboard.h"

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
        HID_USAGE_MAX(255),
        HID_LOGICAL_MIN(0),
        HID_LOGICAL_MAX(255),
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

static const uint8_t hid_consumer_control_report_descriptor[] = {
    HID_USAGE_PAGE(HID_USAGE_PAGE_CONSUMER),
    HID_USAGE(HID_USAGE_CONSUMER_CONTROL),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

    // 16 bits for consumer control
    HID_USAGE_MIN(1),
    HID_USAGE_MAX_N(672, 2),
    HID_LOGICAL_MIN(1),
    HID_LOGICAL_MAX_N(672, 2),
    HID_REPORT_COUNT(1),
    HID_REPORT_SIZE(16),
    HID_INPUT(HID_DATA | HID_ARRAY | HID_ABSOLUTE),

    HID_COLLECTION_END
};

static const uint8_t hid_mouse_report_descriptor[] = {
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
    HID_USAGE(HID_USAGE_DESKTOP_MOUSE),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

        HID_USAGE(HID_USAGE_DESKTOP_POINTER),
        HID_COLLECTION(HID_COLLECTION_PHYSICAL),

        HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),
            HID_USAGE_MIN(1),
            HID_USAGE_MAX(8),
            HID_LOGICAL_MIN(0),
            HID_LOGICAL_MAX(1),
            HID_REPORT_COUNT(8),
            HID_REPORT_SIZE(1),
            HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

        HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
            HID_USAGE(HID_USAGE_DESKTOP_X),
            HID_USAGE(HID_USAGE_DESKTOP_Y),
            HID_LOGICAL_MIN(-127),
            HID_LOGICAL_MAX(127),
            HID_REPORT_COUNT(2),
            HID_REPORT_SIZE(8),
            HID_INPUT(HID_DATA | HID_VARIABLE | HID_RELATIVE),

            HID_USAGE(HID_USAGE_DESKTOP_WHEEL),
            HID_LOGICAL_MIN(-127),
            HID_LOGICAL_MAX(127),
            HID_REPORT_COUNT(1),
            HID_REPORT_SIZE(8),
            HID_INPUT(HID_DATA | HID_VARIABLE | HID_RELATIVE),

        HID_COLLECTION_END,
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
    .idVendor        = USB_VID, // Your vendor id
    .idProduct       = USB_PID, // Your product ID
    .bcdDevice       = 0,      // No device revision number
    .iManufacturer   = 1,      // Manufacturer string index
    .iProduct        = 2,      // Product string index
    .iSerialNumber = 0,        // No serial number
    .bNumConfigurations = 1    // One configuration
};

const struct usb_interface_descriptor kb_interface_descriptor = {
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

const struct usb_interface_descriptor cc_interface_descriptor = {
    .bLength            = sizeof(struct usb_interface_descriptor),
    .bDescriptorType    = USB_DT_INTERFACE,
    .bInterfaceNumber   = 1,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 1,    // Just the consumer control report
    .bInterfaceClass    = 0x03, // HID
    .bInterfaceSubClass = 0x00, // Report only
    .bInterfaceProtocol = 0x00, // No specific protocol
    .iInterface         = 0
};

const struct usb_interface_descriptor mouse_interface_descriptor = {
    .bLength            = sizeof(struct usb_interface_descriptor),
    .bDescriptorType    = USB_DT_INTERFACE,
    .bInterfaceNumber   = 2,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 1,
    .bInterfaceClass    = 0x03, // HID
    .bInterfaceSubClass = 0x00, // Report only (not a boot mouse)
    .bInterfaceProtocol = 0x02,  // Mouse
    .iInterface         = 0
};

const struct usb_endpoint_descriptor ep1_in = {
    .bLength          = sizeof(struct usb_endpoint_descriptor),
    .bDescriptorType  = USB_DT_ENDPOINT,
    .bEndpointAddress = EP1_IN_ADDR, // EP number 1, IN from host (tx from device)
    .bmAttributes     = USB_TRANSFER_TYPE_INTERRUPT,
    .wMaxPacketSize   = 8,
    .bInterval        = USB_REPORT_INTERVAL
};

const struct usb_endpoint_descriptor ep2_in = {
    .bLength          = sizeof(struct usb_endpoint_descriptor),
    .bDescriptorType  = USB_DT_ENDPOINT,
    .bEndpointAddress = EP2_IN_ADDR, // EP number 2, IN from host (tx from device)
    .bmAttributes     = USB_TRANSFER_TYPE_INTERRUPT,
    .wMaxPacketSize   = 8,
    .bInterval        = USB_REPORT_INTERVAL
};

const struct usb_endpoint_descriptor ep3_in = {
    .bLength          = sizeof(struct usb_endpoint_descriptor),
    .bDescriptorType  = USB_DT_ENDPOINT,
    .bEndpointAddress = EP3_IN_ADDR, // EP number 3, IN from host (tx from device)
    .bmAttributes     = USB_TRANSFER_TYPE_INTERRUPT,
    .wMaxPacketSize   = 8,
    .bInterval        = USB_REPORT_INTERVAL
};

const struct usb_hid_descriptor kb_hid_descriptor = {
    .bLength = sizeof(struct usb_hid_descriptor),
    .bDescriptorType = HID_DESC_TYPE_HID,
    .bcdHID = 0x0111,         // HID 1.11
    .bCountryCode = 0,        // Not supported
    .bNumDescriptors = 1,     // We only have one descriptor (report)
    .bReportType = HID_DESC_TYPE_REPORT,
    .wReportLength = sizeof(hid_boot_keyboard_report_descriptor)
};

const struct usb_hid_descriptor cc_hid_descriptor = {
    .bLength = sizeof(struct usb_hid_descriptor),
    .bDescriptorType = HID_DESC_TYPE_HID,
    .bcdHID = 0x0111,         // HID 1.11
    .bCountryCode = 0,        // Not supported
    .bNumDescriptors = 1,     // We only have one descriptor (report)
    .bReportType = HID_DESC_TYPE_REPORT,
    .wReportLength = sizeof(hid_consumer_control_report_descriptor)
};

const struct usb_hid_descriptor mouse_hid_descriptor = {
    .bLength = sizeof(struct usb_hid_descriptor),
    .bDescriptorType = HID_DESC_TYPE_HID,
    .bcdHID = 0x0111,         // HID 1.11
    .bCountryCode = 0,        // Not supported
    .bNumDescriptors = 1,     // We only have one descriptor (report)
    .bReportType = HID_DESC_TYPE_REPORT,
    .wReportLength = sizeof(hid_mouse_report_descriptor)
};

const struct usb_configuration_descriptor config_descriptor = {
    .bLength         = sizeof(struct usb_configuration_descriptor),
    .bDescriptorType = USB_DT_CONFIG,
    .wTotalLength    = (sizeof(config_descriptor) +
                        sizeof(kb_interface_descriptor) +
                        sizeof(cc_interface_descriptor) +
                        sizeof(mouse_interface_descriptor) +
                        sizeof(kb_hid_descriptor) +
                        sizeof(cc_hid_descriptor) +
                        sizeof(mouse_hid_descriptor) +
                        sizeof(ep1_in) +
                        sizeof(ep2_in) +
                        sizeof(ep3_in) +
                        0),
    .bNumInterfaces  = 3,
    .bConfigurationValue = 1, // Configuration 1
    .iConfiguration = 0,      // No string
    .bmAttributes = 0xa0,     // attributes: bus powered, remote wakeup
    .bMaxPower = 0xfa         // 500mA
    // .bLength         = sizeof(struct usb_configuration_descriptor),
    // .bDescriptorType = USB_DT_CONFIG,
    // .wTotalLength    = (sizeof(config_descriptor) +
    //                     sizeof(kb_interface_descriptor) +
    //                     sizeof(cc_interface_descriptor) +
    //                     sizeof(mouse_interface_descriptor) +
    //                     sizeof(kb_hid_descriptor) +
    //                     sizeof(cc_hid_descriptor) +
    //                     sizeof(mouse_hid_descriptor) +
    //                     sizeof(ep1_in) +
    //                     sizeof(ep2_in) +
    //                     sizeof(ep3_in) +
    //                     0),
    // .bNumInterfaces  = 3,
    // .bConfigurationValue = 1, // Configuration 1
    // .iConfiguration = 0,      // No string
    // .bmAttributes = 0xa0,     // attributes: bus powered, remote wakeup
    // .bMaxPower = 0xfa         // 500mA
};

const uint8_t lang_descriptor[] = {
    4,         // bLength
    0x03,      // bDescriptorType == String Descriptor
    0x09, 0x04 // language id = us english
};

const unsigned char* descriptor_strings[] = {
    (unsigned char*) USB_VENDOR_STRING,  // Vendor
    (unsigned char*) USB_PRODUCT_STRING  // Product
};

// public functions
const uint8_t* usb_get_hid_boot_keyboard_report_descriptor(void) {
    return (uint8_t*)hid_boot_keyboard_report_descriptor;
}

const uint8_t* usb_get_hid_consumer_control_report_descriptor(void) {
    return (uint8_t*)hid_consumer_control_report_descriptor;
}

const uint8_t* usb_get_hid_mouse_report_descriptor(void) {
    return (uint8_t*)hid_mouse_report_descriptor;
}

uint32_t usb_get_hid_boot_keyboard_report_descriptor_size(void) {
    return sizeof(hid_boot_keyboard_report_descriptor);
}

uint32_t usb_get_hid_consumer_control_report_descriptor_size(void) {
    return sizeof(hid_consumer_control_report_descriptor);
}

uint32_t usb_get_hid_mouse_report_descriptor_size(void) {
    return sizeof(hid_mouse_report_descriptor);
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

const struct usb_endpoint_descriptor* usb_get_ep2_in_descriptor(void) {
    return &ep2_in;
}

const struct usb_endpoint_descriptor* usb_get_ep3_in_descriptor(void) {
    return &ep3_in;
}

const struct usb_device_descriptor* usb_get_device_descriptor(void) {
    return &device_descriptor;
}

const struct usb_interface_descriptor* usb_get_kb_interface_descriptor(void) {
    return &kb_interface_descriptor;
}

const struct usb_interface_descriptor* usb_get_cc_interface_descriptor(void) {
    return &cc_interface_descriptor;
}

const struct usb_interface_descriptor* usb_get_mouse_interface_descriptor(void) {
    return &mouse_interface_descriptor;
}

const struct usb_hid_descriptor* usb_get_kb_hid_descriptor(void) {
    return &kb_hid_descriptor;
}

const struct usb_hid_descriptor* usb_get_cc_hid_descriptor(void) {
    return &cc_hid_descriptor;
}

const struct usb_hid_descriptor* usb_get_mouse_hid_descriptor(void) {
    return &mouse_hid_descriptor;
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
