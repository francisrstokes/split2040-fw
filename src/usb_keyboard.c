/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2025 Francis Stokes
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "usb_common.h"
#include "hardware/regs/usb.h"
#include "hardware/structs/usb.h"
#include "hardware/irq.h"
#include "hardware/resets.h"

// Device descriptors
#include "usb_descriptors.h"
#include "matrix.h"
#include "keyboard.h"
#include "leds.h"

#define usb_hw_set ((usb_hw_t *)hw_set_alias_untyped(usb_hw))
#define usb_hw_clear ((usb_hw_t *)hw_clear_alias_untyped(usb_hw))

// Typedefs
typedef enum ep_transfer_state_t {
    ep_transfer_state_idle,
    ep_transfer_state_data,
    ep_transfer_state_status,
} ep_transfer_state_t;

typedef struct ep_data_state_t {
    uint16_t bytes_transferred;
    uint16_t bytes_total;
    uint8_t* current_buffer;
} ep_data_state_t;

typedef struct endpoint_t {
    uint8_t* data_buffer;
    volatile uint32_t* buffer_control;
    volatile uint32_t* endpoint_control;
    const struct usb_endpoint_descriptor* descriptor;
    uint8_t next_pid;

    // State for tracking what's going on with the endpoint at any time
    ep_transfer_state_t transfer;
    ep_data_state_t data;
} endpoint_t;

typedef struct endpoint_zero_state_t {
    struct usb_setup_packet setup_packet;
    endpoint_t in;
    endpoint_t out;
} endpoint_zero_state_t;

// Function prototypes for our device specific endpoint handlers defined later on
static void ep0_in_handler(void);
static void ep0_out_handler(void);

// Global device address
static volatile bool configured_by_host = false;

static endpoint_zero_state_t ep0 = {
    .setup_packet = {0},
    .out = {
        .buffer_control = &usb_dpram->ep_buf_ctrl[0].out,
        .endpoint_control = NULL,
        .data_buffer = &usb_dpram->ep0_buf_a[0],
        .descriptor = NULL,
        .next_pid = 0,
        .data = {
            .bytes_total = 0,
            .bytes_transferred = 0,
            .current_buffer = NULL
        },
        .transfer = ep_transfer_state_idle
    },
    .in = {
        .buffer_control = &usb_dpram->ep_buf_ctrl[0].in,
        .endpoint_control = NULL,
        .data_buffer = &usb_dpram->ep0_buf_a[0],
        .descriptor = NULL,
        .next_pid = 0,
        .data = {
            .bytes_total = 0,
            .bytes_transferred = 0,
            .current_buffer = NULL
        },
        .transfer = ep_transfer_state_idle
    },
};

static endpoint_t ep_kb_in = {
    .buffer_control = &usb_dpram->ep_buf_ctrl[1].in,
    .endpoint_control = &usb_dpram->ep_ctrl[0].in,
    .data_buffer = &usb_dpram->epx_data[0],
    .descriptor = NULL,
    .next_pid = 0,
    .data = {
        .bytes_total = 0,
        .bytes_transferred = 0,
        .current_buffer = NULL
    },
    .transfer = ep_transfer_state_idle
};

static endpoint_t ep_cc_in = {
    .buffer_control = &usb_dpram->ep_buf_ctrl[2].in,
    .endpoint_control = &usb_dpram->ep_ctrl[1].in,
    // The Keyboard interrupt only needs 8 bytes, but each data buffer slot has to be aligned to 64 bytes
    .data_buffer = &(usb_dpram->epx_data[64]),
    .descriptor = NULL,
    .next_pid = 0
};

static uint8_t multi_packet_buffer[1024] = {0};

// HID keyboard report
static uint8_t keyboard_hid_report[8] = {0};
static uint8_t next_keyboard_hid_report[8] = {0};

static uint16_t consumer_control_report = 0;
static uint16_t next_consumer_control_report = 0;

// Private functions
static uint8_t usb_prepare_string_descriptor(const unsigned char *str) {
    // 2 for bLength + bDescriptorType + strlen * 2 because string is unicode. i.e. other byte will be 0
    uint8_t bLength = 2 + (strlen((const char *)str) * 2);
    static const uint8_t bDescriptorType = 0x03;

    volatile uint8_t *buf = ep0.in.data_buffer;
    *buf++ = bLength;
    *buf++ = bDescriptorType;

    uint8_t c;

    do {
        c = *str++;
        *buf++ = c;
        *buf++ = 0;
    } while (c != '\0');

    return bLength;
}

static void usb_setup_endpoints(void) {
    ep0.out.descriptor = usb_get_ep0_out_descriptor();
    ep0.in.descriptor = usb_get_ep0_in_descriptor();
    ep_kb_in.descriptor = usb_get_ep1_in_descriptor();
    ep_cc_in.descriptor = usb_get_ep2_in_descriptor();

    // Set up the keyboard report endpoint
    uint32_t dpram_offset = (uint32_t)ep_kb_in.data_buffer ^ (uint32_t)usb_dpram;
    uint32_t reg = EP_CTRL_ENABLE_BITS
                   | EP_CTRL_INTERRUPT_PER_BUFFER
                   | (ep_kb_in.descriptor->bmAttributes << EP_CTRL_BUFFER_TYPE_LSB)
                   | dpram_offset;

    *ep_kb_in.endpoint_control = reg;

    // Set up the consumer control report endpoint
    dpram_offset = (uint32_t)ep_cc_in.data_buffer ^ (uint32_t)usb_dpram;
    reg = EP_CTRL_ENABLE_BITS
                   | EP_CTRL_INTERRUPT_PER_BUFFER
                   | (ep_cc_in.descriptor->bmAttributes << EP_CTRL_BUFFER_TYPE_LSB)
                   | dpram_offset;

    *ep_cc_in.endpoint_control = reg;
}

static inline bool ep_is_tx(endpoint_t* ep) {
    return ep->descriptor->bEndpointAddress & USB_DIR_IN;
}

static uint32_t usb_ep_get_next_pid(endpoint_t* ep) {
    uint32_t pid_bit = ep->next_pid ? USB_BUF_CTRL_DATA1_PID : USB_BUF_CTRL_DATA0_PID;
    ep->next_pid ^= 1u;
    return pid_bit;
}

static void usb_write_data(endpoint_t* ep) {
    // Actual data during data stage
    // Assume data descriptor has been set up for the endpoint
    // Handle multi-packet logic here
    assert(ep_is_tx(ep));

    ep->transfer = ep_transfer_state_data;

    uint32_t buf_ctrl_val = usb_ep_get_next_pid(ep) | USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL;

    // Simple case: This is a single packet
    if (ep->data.bytes_total <= ep->descriptor->wMaxPacketSize) {
        buf_ctrl_val |= ep->data.bytes_total;

        // Copy into the USB DPSRAM
        memcpy((void*)ep->data_buffer, (void*)ep->data.current_buffer, ep->data.bytes_total);
    } else {
        // More complex case: A multi-packet data transfer
        uint32_t bytes_in_this_packet = MIN((ep->data.bytes_total - ep->data.bytes_transferred), ep->descriptor->wMaxPacketSize);
        buf_ctrl_val |= bytes_in_this_packet;

        // If this is the last packet, and the total length is not a multiple of the packet size, mark the packet explicitly as the last
        if ((ep->data.bytes_transferred + bytes_in_this_packet) == ep->data.bytes_total) {
            if ((ep->data.bytes_total & (ep->descriptor->wMaxPacketSize - 1)) != 0) {
                buf_ctrl_val |= USB_BUF_CTRL_LAST;
            }
        }

        // Copy into the USB DPSRAM
        memcpy((void*)ep->data_buffer, (void*)(ep->data.current_buffer + ep->data.bytes_transferred), bytes_in_this_packet);
    }

    // Start the transfer
    *ep->buffer_control = buf_ctrl_val;
}

static void usb_read_data(endpoint_t* ep) {
    // Actual data during data stage
    // Assume data descriptor has been set up for the endpoint
    assert(!ep_is_tx(ep));

    ep->transfer = ep_transfer_state_data;

    uint32_t buf_ctrl_val = usb_ep_get_next_pid(ep) | USB_BUF_CTRL_AVAIL;

    // TODO: Handle multi-packet RX
    buf_ctrl_val |= ep->data.bytes_total;

    // Queue the transfer
    *ep->buffer_control = buf_ctrl_val;
}

static void usb_send_zlp(void) {
    ep0.out.transfer = ep_transfer_state_status;
    *ep0.in.buffer_control = usb_ep_get_next_pid(&ep0.in) | USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL;
}

static void usb_receive_zlp(void) {
    ep0.in.transfer = ep_transfer_state_status;
    *ep0.out.buffer_control = usb_ep_get_next_pid(&ep0.out) | USB_BUF_CTRL_AVAIL;
}

static void usb_handle_device_descriptor(volatile struct usb_setup_packet *pkt) {
    ep0.in.data = (ep_data_state_t) {
        .bytes_total = sizeof(struct usb_device_descriptor),
        .bytes_transferred = 0,
        .current_buffer = (uint8_t*)usb_get_device_descriptor()
    };

    usb_write_data(&ep0.in);
}

static void usb_handle_config_descriptor(volatile struct usb_setup_packet *pkt) {
    uint32_t len = 0;
    uint8_t *buf = multi_packet_buffer;
    uint8_t *buf_start = buf;

    // First request will want just the config descriptor
    const struct usb_configuration_descriptor *d = usb_get_configuration_descriptor();
    memcpy((void *) buf, d, sizeof(struct usb_configuration_descriptor));
    buf += sizeof(struct usb_configuration_descriptor);
    len += sizeof(struct usb_configuration_descriptor);

    // If we're asked more than just the config descriptor copy it all
    if (pkt->wLength >= d->wTotalLength) {
        // The interface, HID, and report descriptors for the keyboard
        memcpy((void *) buf, usb_get_kb_interface_descriptor(), sizeof(struct usb_interface_descriptor));
        buf += sizeof(struct usb_interface_descriptor);
        len += sizeof(struct usb_interface_descriptor);

        memcpy((void *) buf, usb_get_kb_hid_descriptor(), sizeof(struct usb_hid_descriptor));
        buf += sizeof(struct usb_hid_descriptor);
        len += sizeof(struct usb_hid_descriptor);

        memcpy((void *)buf, ep_kb_in.descriptor, sizeof(struct usb_endpoint_descriptor));
        buf += sizeof(struct usb_endpoint_descriptor);
        len += sizeof(struct usb_endpoint_descriptor);

        // The interface, HID, and report descriptors for the consumer control
        memcpy((void *) buf, usb_get_cc_interface_descriptor(), sizeof(struct usb_interface_descriptor));
        buf += sizeof(struct usb_interface_descriptor);
        len += sizeof(struct usb_interface_descriptor);

        memcpy((void *) buf, usb_get_cc_hid_descriptor(), sizeof(struct usb_hid_descriptor));
        buf += sizeof(struct usb_hid_descriptor);
        len += sizeof(struct usb_hid_descriptor);

        memcpy((void *)buf, ep_cc_in.descriptor, sizeof(struct usb_endpoint_descriptor));
        buf += sizeof(struct usb_endpoint_descriptor);
        len += sizeof(struct usb_endpoint_descriptor);
    }

    // Send data
    ep0.in.data = (ep_data_state_t) {
        .bytes_total = len,
        .bytes_transferred = 0,
        .current_buffer = buf_start
    };
    usb_write_data(&ep0.in);
}

static void usb_handle_hid_descriptor(volatile struct usb_setup_packet *pkt) {
    uint8_t* buffer = (uint8_t*)(pkt->wIndex == KB_INTERFACE
        ? usb_get_kb_hid_descriptor()
        : usb_get_cc_hid_descriptor());


    ep0.in.data = (ep_data_state_t) {
        .bytes_total = sizeof(struct usb_hid_descriptor),
        .bytes_transferred = 0,
        .current_buffer = buffer
    };
    usb_write_data(&ep0.in);
}

static void usb_handle_hid_report_descriptor(volatile struct usb_setup_packet *pkt) {
    uint8_t* buffer = (uint8_t*)(pkt->wIndex == KB_INTERFACE
        ? usb_get_hid_boot_keyboard_report_descriptor()
        : usb_get_hid_consumer_control_report_descriptor());

    uint16_t buffer_size = pkt->wIndex == KB_INTERFACE
        ? usb_get_hid_boot_keyboard_report_descriptor_size()
        : usb_get_hid_consumer_control_report_descriptor_size();

    ep0.in.data = (ep_data_state_t) {
        .bytes_total = buffer_size,
        .bytes_transferred = 0,
        .current_buffer = buffer
    };
    usb_write_data(&ep0.in);
}

static void usb_handle_get_protocol(volatile struct usb_setup_packet *pkt) {
    static const uint8_t boot_protocol = 0u;
    ep0.in.data = (ep_data_state_t) {
        .bytes_total = 1,
        .bytes_transferred = 0,
        .current_buffer = (uint8_t*)&boot_protocol
    };
    usb_write_data(&ep0.in);
}

static void usb_bus_reset(void) {
    // Set address back to 0
    usb_hw->dev_addr_ctrl = 0;
    ep0.in.next_pid = 0;
    ep0.out.next_pid = 0;
    ep0.in.transfer = ep_transfer_state_idle;
    ep0.out.transfer = ep_transfer_state_idle;
    ep_kb_in.next_pid = 0;
    configured_by_host = false;
}

static void usb_handle_string_descriptor(volatile struct usb_setup_packet *pkt) {
    uint8_t i = pkt->wValue & 0xff;
    uint8_t len = 0;

    if (i == 0) {
        len = 4;
        memcpy(ep0.in.data_buffer, usb_get_lang_descriptor(), len);
    } else {
        // Prepare fills in ep0_buf
        len = usb_prepare_string_descriptor(usb_get_descriptor_strings()[i - 1]);
    }

    ep0.in.data = (ep_data_state_t) {
        .bytes_total = len,
        .bytes_transferred = 0,
        .current_buffer = ep0.in.data_buffer
    };
    usb_write_data(&ep0.in);
}

static void usb_rx_set_report_from_host(void) {
    ep0.in.data = (ep_data_state_t) {
        .bytes_total = 1,
        .bytes_transferred = 0,
        .current_buffer = ep0.out.data_buffer
    };
    usb_read_data(&ep0.out);
}

static void usb_set_device_configuration(void) {
    // Only one configuration so just acknowledge the request
    configured_by_host = true;
    usb_send_zlp();
}

static void usb_handle_setup_packet(void) {
    volatile struct usb_setup_packet *pkt = (volatile struct usb_setup_packet *) &usb_dpram->setup_packet;

    // Copy the setup packet to the ep0 struct
    memcpy(&ep0.setup_packet, (void*)pkt, sizeof(struct usb_setup_packet));

    // IN=device to host, OUT=host to device
    bool in_req = (pkt->bmRequestType & USB_DIR_MASK) == USB_DIR_IN;
    bool class_req = (pkt->bmRequestType & USB_REQ_TYPE_TYPE_MASK) == USB_REQ_TYPE_TYPE_CLASS;

    // A setup packet is always sent with DATA0, so any response should use DATA1
    ep0.in.next_pid = 1u;
    ep0.out.next_pid = 1u;

    // Is data being requested?
    if (in_req) {
        if (class_req) {
            switch (pkt->bRequest) {
                case HID_REQ_CONTROL_GET_PROTOCOL:  usb_handle_get_protocol(pkt);           break;
            }
        } else {
            switch (pkt->bRequest) {
                case USB_REQUEST_GET_DESCRIPTOR: {
                    uint16_t descriptor_type = pkt->wValue >> 8;

                    switch (descriptor_type) {
                        case USB_DT_DEVICE:         usb_handle_device_descriptor(pkt);      break;
                        case USB_DT_CONFIG:         usb_handle_config_descriptor(pkt);      break;
                        case USB_DT_STRING:         usb_handle_string_descriptor(pkt);      break;

                        case HID_DESC_TYPE_HID:     usb_handle_hid_descriptor(pkt);         break;
                        case HID_DESC_TYPE_REPORT:  usb_handle_hid_report_descriptor(pkt);  break;
                    }
                } break;
            }
        }
    // Data or a command is being sent by the host
    } else {
        if (class_req) {
            switch (pkt->bRequest) {
                case HID_REQ_CONTROL_SET_PROTOCOL:  usb_send_zlp();                         break;
                case HID_REQ_CONTROL_SET_IDLE:      usb_send_zlp();                         break;
                case HID_REQ_CONTROL_SET_REPORT:    usb_rx_set_report_from_host();          break;
                default:                            usb_send_zlp();                         break;
            }
        } else {
            switch (pkt->bRequest) {
                case USB_REQUEST_SET_ADDRESS:       usb_send_zlp();                         break;
                case USB_REQUEST_SET_CONFIGURATION: usb_set_device_configuration();         break;
            }
        }
    }
}

static void usb_handle_buff_status() {
    uint32_t buffers = usb_hw->buf_status;

    if (buffers & USB_BUFF_CPU_SHOULD_HANDLE_EP0_IN_BITS) {
        usb_hw_clear->buf_status = USB_BUFF_CPU_SHOULD_HANDLE_EP0_IN_BITS;
        ep0_in_handler();
    }

    if (buffers & USB_BUFF_CPU_SHOULD_HANDLE_EP0_OUT_BITS) {
        usb_hw_clear->buf_status = USB_BUFF_CPU_SHOULD_HANDLE_EP0_OUT_BITS;
        ep0_out_handler();
    }

    if (buffers & USB_BUFF_CPU_SHOULD_HANDLE_EP1_IN_BITS) {
        usb_hw_clear->buf_status = USB_BUFF_CPU_SHOULD_HANDLE_EP1_IN_BITS;
    }

    if (buffers & USB_BUFF_CPU_SHOULD_HANDLE_EP2_IN_BITS) {
        usb_hw_clear->buf_status = USB_BUFF_CPU_SHOULD_HANDLE_EP2_IN_BITS;
    }
}


#ifdef __cplusplus
extern "C" {
#endif

void isr_usbctrl(void) {
    // USB interrupt handler
    uint32_t status = usb_hw->ints;
    uint32_t handled = 0;

    // Setup packet received
    if (status & USB_INTS_SETUP_REQ_BITS) {
        handled |= USB_INTS_SETUP_REQ_BITS;
        usb_hw_clear->sie_status = USB_SIE_STATUS_SETUP_REC_BITS;
        usb_handle_setup_packet();
    }

    // Buffer status, one or more buffers have completed
    if (status & USB_INTS_BUFF_STATUS_BITS) {
        handled |= USB_INTS_BUFF_STATUS_BITS;
        usb_handle_buff_status();
    }

    // Bus is reset
    if (status & USB_INTS_BUS_RESET_BITS) {
        handled |= USB_INTS_BUS_RESET_BITS;
        usb_hw_clear->sie_status = USB_SIE_STATUS_BUS_RESET_BITS;
        usb_bus_reset();
    }

    if (status ^ handled) {
        panic("Unhandled IRQ 0x%x\n", (uint) (status ^ handled));
    }
}

#ifdef __cplusplus
}
#endif

// EP0 IN packets are host initiated device-to-host communication.
static void ep0_in_handler(void) {
    bool in_handled = false;
    bool out_handled = false;

    // [IN] We were in the data stage, and a buffer is complete (tx)
    if (ep0.in.transfer == ep_transfer_state_data) {
        uint16_t bytes_remaining = ep0.in.data.bytes_total - ep0.in.data.bytes_transferred;

        // We don't accumulate the transferred count until the buffer completes, so do that now
        ep0.in.data.bytes_transferred += MIN(bytes_remaining, ep0.in.descriptor->wMaxPacketSize);
        bytes_remaining = ep0.in.data.bytes_total - ep0.in.data.bytes_transferred;

        // Is there more data to send?
        if (bytes_remaining > 0) {
            // Queue the next data buffer
            usb_write_data(&ep0.in);
        } else {
            // If we're done, we can rx a ZLP from the host in the status stage
            usb_receive_zlp();
        }

        in_handled = true;
    }

    // [OUT] We were in the status stage, and have successfully written a ZLP
    if (ep0.out.transfer == ep_transfer_state_status) {
        if (ep0.setup_packet.bRequest == USB_REQUEST_SET_ADDRESS) {
            // Now that the SET_ADDRESS request is over, start using the supplied address
            usb_hw->dev_addr_ctrl = ep0.setup_packet.wValue & 0xff;
        }
        ep0.out.transfer = ep_transfer_state_idle;
        out_handled = true;
    }

    if (!in_handled && !out_handled) {
        ep0.in.transfer = ep_transfer_state_idle;
    }
}

// EP0 OUT packets are host initiated host-to-device communication.
static void ep0_out_handler(void) {
    bool in_handled = false;
    bool out_handled = false;

    // [OUT] We were in the data stage, and a buffer is complete (rx)
    if (ep0.out.transfer == ep_transfer_state_data) {
        if (ep0.setup_packet.bRequest == HID_REQ_CONTROL_SET_REPORT) {
            keyboard_on_led_status_report(ep0.out.data_buffer[0]);
        }

        // Send a ZLP to the host in acknowledgement for the status stage
        usb_send_zlp();
        out_handled = true;
    }

    // [IN] We were in the status stage, and have successfully received a ZLP from the host
    if (ep0.in.transfer == ep_transfer_state_status) {
        // The whole transfer is complete
        ep0.in.transfer = ep_transfer_state_idle;
        in_handled = true;
    }

    if (!in_handled && !out_handled) {
        ep0.out.transfer = ep_transfer_state_idle;
    }
}

// public functions
void usb_device_init(void) {
    // Reset usb controller
    reset_unreset_block_num_wait_blocking(RESET_USBCTRL);

    // Clear any previous state in dpram just in case
    memset(usb_dpram, 0, sizeof(*usb_dpram)); // <1>

    // Enable USB interrupt at processor
    irq_set_enabled(USBCTRL_IRQ, true);

    // Mux the controller to the onboard usb phy
    usb_hw->muxing = USB_USB_MUXING_TO_PHY_BITS | USB_USB_MUXING_SOFTCON_BITS;

    // Force VBUS detect so the device thinks it is plugged into a host
    usb_hw->pwr = USB_USB_PWR_VBUS_DETECT_BITS | USB_USB_PWR_VBUS_DETECT_OVERRIDE_EN_BITS;

    // Enable the USB controller in device mode.
    usb_hw->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN_BITS;

    // Enable an interrupt per EP0 transaction
    usb_hw->sie_ctrl = USB_SIE_CTRL_EP0_INT_1BUF_BITS; // <2>

    // Enable interrupts for when a buffer is done, when the bus is reset,
    // and when a setup packet is received
    usb_hw->inte = USB_INTS_BUFF_STATUS_BITS |
                   USB_INTS_BUS_RESET_BITS |
                   USB_INTS_SETUP_REQ_BITS;

    // Set up endpoint control registers described by device configuration
    usb_setup_endpoints();

    // Present full speed device by enabling pull up on DP
    usb_hw_set->sie_ctrl = USB_SIE_CTRL_PULLUP_EN_BITS;
}

uint8_t* usb_get_kb_hid_descriptor_ptr(void) {
    return next_keyboard_hid_report;
}

uint16_t* usb_get_cc_hid_descriptor_ptr(void) {
    return &next_consumer_control_report;
}

void usb_wait_for_device_to_configured(void) {
    // Wait until configured
    while (!configured_by_host) {
        tight_loop_contents();
    }
}

void usb_update(void) {
    // Only prepare a new interrupt response when something has changed (the hardware will nack the interrupt IN if no data is already in the buffer)
    if (memcmp(next_keyboard_hid_report, keyboard_hid_report, 8) != 0) {
        ep_kb_in.data = (ep_data_state_t) {
            .bytes_total = 8,
            .bytes_transferred = 0,
            .current_buffer = (uint8_t*)next_keyboard_hid_report
        };
        usb_write_data(&ep_kb_in);
        memcpy(keyboard_hid_report, next_keyboard_hid_report, 8);
    }

    if (next_consumer_control_report != consumer_control_report) {
        ep_cc_in.data = (ep_data_state_t) {
            .bytes_total = 2,
            .bytes_transferred = 0,
            .current_buffer = (uint8_t*)&next_consumer_control_report
        };
        usb_write_data(&ep_cc_in);
        consumer_control_report = next_consumer_control_report;
    }
}
