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
#include "descriptors.h"
#include "matrix.h"
#include "keyboard.h"
#include "leds.h"

#define usb_hw_set ((usb_hw_t *)hw_set_alias_untyped(usb_hw))
#define usb_hw_clear ((usb_hw_t *)hw_clear_alias_untyped(usb_hw))

// Function prototypes for our device specific endpoint handlers defined later on
void ep0_in_handler(uint8_t *buf, uint16_t len);
void ep0_out_handler(uint8_t *buf, uint16_t len);
void ep1_in_handler(uint8_t *buf, uint16_t len);

// Global device address
static bool should_set_address = false;
static uint8_t dev_addr = 0;
static volatile bool configured = false;

// Global data buffer for EP0
static uint8_t ep0_buf[64];

// HID keyboard report
static uint8_t keyboard_hid_report[8] = {0};
static uint8_t next_keyboard_hid_report[8] = {0};
static repeating_timer_t matrix_scan_timer = {0};

// Struct defining the device configuration
static struct usb_device_configuration dev_config = {
    .device_descriptor = &device_descriptor,
    .interface_descriptor = &interface_descriptor,
    .config_descriptor = &config_descriptor,
    .hid_descriptor = &hid_descriptor,
    .hid_keyboard_report_descriptor = hid_boot_keyboard_report_descriptor,
    .lang_descriptor = lang_descriptor,
    .descriptor_strings = descriptor_strings,
    .endpoints = {
        // EP0_OUT
        {
            .descriptor = &ep0_out,
            .handler = &ep0_out_handler,
            .endpoint_control = NULL, // NA for EP0
            .buffer_control = &usb_dpram->ep_buf_ctrl[0].out,
            // EP0 in and out share a data buffer
            .data_buffer = &usb_dpram->ep0_buf_a[0],
        },
        // EP0_IN
        {
            .descriptor = &ep0_in,
            .handler = &ep0_in_handler,
            .endpoint_control = NULL, // NA for EP0,
            .buffer_control = &usb_dpram->ep_buf_ctrl[0].in,
            // EP0 in and out share a data buffer
            .data_buffer = &usb_dpram->ep0_buf_a[0],
        },

        // EP_KEYBOARD_IN
        {
            .descriptor = &ep1_in,
            .handler = &ep1_in_handler,
            // endpoint control starts with endpoint 1, so this is 0
            .endpoint_control = &usb_dpram->ep_ctrl[0].in,
            // buffer control starts with endpoint 0, so this is 1
            .buffer_control = &usb_dpram->ep_buf_ctrl[1].in,
            // Since ep0 has a dedicated shared buffer, this endpoint essentially gets the whole rest of the dpram
            .data_buffer = &usb_dpram->epx_data[0],
        }
    }
};

uint8_t usb_prepare_string_descriptor(const unsigned char *str) {
    // 2 for bLength + bDescriptorType + strlen * 2 because string is unicode. i.e. other byte will be 0
    uint8_t bLength = 2 + (strlen((const char *)str) * 2);
    static const uint8_t bDescriptorType = 0x03;

    volatile uint8_t *buf = &ep0_buf[0];
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

static inline uint32_t usb_buffer_offset(volatile uint8_t *buf) {
    return (uint32_t) buf ^ (uint32_t) usb_dpram;
}

void usb_setup_endpoints(void) {
    // Just set up the keyboard in endpoint
    uint32_t dpram_offset = usb_buffer_offset(dev_config.endpoints[EP_KEYBOARD_IN].data_buffer);
    uint32_t reg = EP_CTRL_ENABLE_BITS
                   | EP_CTRL_INTERRUPT_PER_BUFFER
                   | (dev_config.endpoints[EP_KEYBOARD_IN].descriptor->bmAttributes << EP_CTRL_BUFFER_TYPE_LSB)
                   | dpram_offset;

    *dev_config.endpoints[EP_KEYBOARD_IN].endpoint_control = reg;
}

void usb_device_init() {
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

static inline bool ep_is_tx(struct usb_endpoint_configuration *ep) {
    return ep->descriptor->bEndpointAddress & USB_DIR_IN;
}

void usb_send_nack(struct usb_endpoint_configuration *ep) {
    // Set the abort bit on EP1_IN (keyboard)
    usb_hw->abort |= (1 << 2);
}

void usb_start_transfer(struct usb_endpoint_configuration *ep, uint8_t *buf, uint16_t len) {
    // We are asserting that the length is <= 64 bytes for simplicity of the example.
    // For multi packet transfers see the tinyusb port.
    assert(len <= 64);

    // Prepare buffer control register value
    uint32_t val = len | USB_BUF_CTRL_AVAIL;

    if (ep_is_tx(ep)) {
        // Need to copy the data from the user buffer to the usb memory
        memcpy((void *) ep->data_buffer, (void *) buf, len);
        // Mark as full
        val |= USB_BUF_CTRL_FULL;
    }

    // Set pid and flip for next transfer
    val |= ep->next_pid ? USB_BUF_CTRL_DATA1_PID : USB_BUF_CTRL_DATA0_PID;
    ep->next_pid ^= 1u;

    *ep->buffer_control = val;
}

void usb_handle_device_descriptor(volatile struct usb_setup_packet *pkt) {
    usb_start_transfer(
        &dev_config.endpoints[EP0_IN],
        (uint8_t*)dev_config.device_descriptor,
        MIN(sizeof(struct usb_device_descriptor), pkt->wLength)
    );
}

void usb_handle_config_descriptor(volatile struct usb_setup_packet *pkt) {
    uint32_t len = 0;
    uint8_t *buf = &ep0_buf[0];

    // First request will want just the config descriptor
    const struct usb_configuration_descriptor *d = dev_config.config_descriptor;
    memcpy((void *) buf, d, sizeof(struct usb_configuration_descriptor));
    buf += sizeof(struct usb_configuration_descriptor);
    len += sizeof(struct usb_configuration_descriptor);

    // If we're asked more than just the config descriptor copy it all
    if (pkt->wLength >= d->wTotalLength) {
        memcpy((void *) buf, dev_config.interface_descriptor, sizeof(struct usb_interface_descriptor));
        buf += sizeof(struct usb_interface_descriptor);
        len += sizeof(struct usb_interface_descriptor);

        // Add the HID descriptor
        memcpy((void *) buf, dev_config.hid_descriptor, sizeof(struct usb_hid_descriptor));
        buf += sizeof(struct usb_hid_descriptor);
        len += sizeof(struct usb_hid_descriptor);

        // Add the keyboard endpoint
        memcpy((void *)buf, dev_config.endpoints[EP_KEYBOARD_IN].descriptor, sizeof(struct usb_endpoint_descriptor));
        buf += sizeof(struct usb_endpoint_descriptor);
        len += sizeof(struct usb_endpoint_descriptor);
    }

    // Send data
    usb_start_transfer(&dev_config.endpoints[EP0_IN], &ep0_buf[0], MIN(len, pkt->wLength));
}

void usb_handle_hid_descriptor(volatile struct usb_setup_packet *pkt) {
    usb_start_transfer(
        &dev_config.endpoints[EP0_IN],
        (uint8_t*)dev_config.hid_descriptor,
        MIN(pkt->wLength, sizeof(struct usb_hid_descriptor))
    );
}

void usb_handle_hid_report_descriptor(volatile struct usb_setup_packet *pkt) {
    usb_start_transfer(
        &dev_config.endpoints[EP0_IN],
        (uint8_t*)hid_boot_keyboard_report_descriptor,
        MIN(pkt->wLength, sizeof(hid_boot_keyboard_report_descriptor))
    );
}

void usb_handle_get_protocol(volatile struct usb_setup_packet *pkt) {
    const uint8_t boot_protocol = 0u;
    usb_start_transfer(&dev_config.endpoints[EP0_IN], (uint8_t*)&boot_protocol, 1u);
}

void usb_bus_reset(void) {
    // Set address back to 0
    dev_addr = 0;
    should_set_address = false;
    usb_hw->dev_addr_ctrl = 0;
    configured = false;
}

void usb_handle_string_descriptor(volatile struct usb_setup_packet *pkt) {
    uint8_t i = pkt->wValue & 0xff;
    uint8_t len = 0;

    if (i == 0) {
        len = 4;
        memcpy(&ep0_buf[0], dev_config.lang_descriptor, len);
    } else {
        // Prepare fills in ep0_buf
        len = usb_prepare_string_descriptor(dev_config.descriptor_strings[i - 1]);
    }

    usb_start_transfer(&dev_config.endpoints[EP0_IN], &ep0_buf[0], MIN(len, pkt->wLength));
}

void usb_acknowledge_out_request(void) {
    usb_start_transfer(&dev_config.endpoints[EP0_IN], NULL, 0);
}

/**
 * @brief Handles a SET_ADDR request from the host. The actual setting of the device address in
 * hardware is done in ep0_in_handler. This is because we have to acknowledge the request first
 * as a device with address zero.
 *
 * @param pkt, the setup packet from the host.
 */
void usb_set_device_address(volatile struct usb_setup_packet *pkt) {
    // Set address is a bit of a strange case because we have to send a 0 length status packet first with
    // address 0
    dev_addr = (pkt->wValue & 0xff);

    // Will set address in the callback phase
    should_set_address = true;
    usb_acknowledge_out_request();
}

void usb_set_device_configuration(__unused volatile struct usb_setup_packet *pkt) {
    // Only one configuration so just acknowledge the request
    usb_acknowledge_out_request();
    configured = true;
}

void usb_handle_setup_packet(void) {
    volatile struct usb_setup_packet *pkt = (volatile struct usb_setup_packet *) &usb_dpram->setup_packet;

    // IN=device to host, OUT=host to device
    bool in_req = (pkt->bmRequestType & USB_DIR_MASK) == USB_DIR_IN;
    bool class_req = (pkt->bmRequestType & USB_REQ_TYPE_TYPE_MASK) == USB_REQ_TYPE_TYPE_CLASS;

    // For control transfers, always reset PID to 1 for EP0 IN (which causes the transfer to use DATA0 as the pid)
    dev_config.endpoints[EP0_IN].next_pid = 1u;

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
                case HID_REQ_CONTROL_SET_PROTOCOL:  usb_acknowledge_out_request();          break;
                case HID_REQ_CONTROL_SET_IDLE:      usb_acknowledge_out_request();          break;
                default:                            usb_acknowledge_out_request();          break;
            }
        } else {
            switch (pkt->bRequest) {
                case USB_REQUEST_SET_ADDRESS:       usb_set_device_address(pkt);            break;
                case USB_REQUEST_SET_CONFIGURATION: usb_set_device_configuration(pkt);      break;
            }
        }
    }
}

static void usb_handle_ep_buff_done(struct usb_endpoint_configuration *ep) {
    uint32_t buffer_control = *ep->buffer_control;
    // Get the transfer length for this endpoint
    uint16_t len = buffer_control & USB_BUF_CTRL_LEN_MASK;

    // Call that endpoints buffer done handler
    ep->handler((uint8_t *) ep->data_buffer, len);
}

static void usb_handle_buff_done(uint ep_num, bool in) {
    uint8_t ep_addr = ep_num | (in ? USB_DIR_IN : 0);

    for (uint i = 0; i < USB_NUM_ENDPOINTS; i++) {
        struct usb_endpoint_configuration *ep = &dev_config.endpoints[i];
        if (ep->descriptor && ep->handler) {
            if (ep->descriptor->bEndpointAddress == ep_addr) {
                usb_handle_ep_buff_done(ep);
                return;
            }
        }
    }
}

/**
 * @brief Handle a "buffer status" irq. This means that one or more
 * buffers have been sent / received. Notify each endpoint where this
 * is the case.
 */
static void usb_handle_buff_status() {
    uint32_t buffers = usb_hw->buf_status;
    uint32_t remaining_buffers = buffers;

    uint bit = 1u;
    for (uint i = 0; remaining_buffers && i < USB_NUM_ENDPOINTS * 2; i++) {
        if (remaining_buffers & bit) {
            // clear this in advance
            usb_hw_clear->buf_status = bit;

            // IN transfer for even i, OUT transfer for odd i
            // Note: this comes from the chip register layout, not the indexing of how the endpoints are organised in the dev_config
            usb_handle_buff_done(i >> 1u, !(i & 1u));
            remaining_buffers &= ~bit;
        }
        bit <<= 1u;
    }
}

/**
 * @brief USB interrupt handler
 *
 */
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
        printf("BUS RESET\n");
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

/**
 * @brief EP0 in transfer complete. Either finish the SET_ADDRESS process, or receive a zero
 * length status packet from the host.
 *
 * @param buf the data that was sent
 * @param len the length that was sent
 */
void ep0_in_handler(__unused uint8_t *buf, __unused uint16_t len) {
    if (should_set_address) {
        // Set actual device address in hardware
        usb_hw->dev_addr_ctrl = dev_addr;
        should_set_address = false;
    } else {
        // Receive a zero length status packet from the host on EP0 OUT
        usb_start_transfer(&dev_config.endpoints[EP0_OUT], NULL, 0);
    }
}

void ep0_out_handler(__unused uint8_t *buf, __unused uint16_t len) {
    // If we need to handle specific OUT data, do it here

    // For SET_REPORT requests, we should examine the data
    // but for now, just acknowledge completion
    volatile struct usb_setup_packet *pkt = (volatile struct usb_setup_packet *) &usb_dpram->setup_packet;

    // For most requests, we need to acknowledge with a zero-length IN packet
    usb_start_transfer(&dev_config.endpoints[EP0_IN], NULL, 0);
}

// Device specific functions
void ep1_in_handler(uint8_t *buf, uint16_t len) {
    // Data was RXd by host, clear the bit to mark as handled
    usb_hw->buf_status &= ~(1 << 2);
}

static bool matrix_scan_timer_cb(repeating_timer_t *rt) {
    matrix_scan();

    // Only prepare a new interrupt response when something has changed (the hardware will nack the interrupt IN if no data is already in the buffer)
    if (memcmp(next_keyboard_hid_report, keyboard_hid_report, 8) != 0) {
        memcpy(keyboard_hid_report, next_keyboard_hid_report, 8);
        usb_start_transfer(&dev_config.endpoints[EP_KEYBOARD_IN], (uint8_t*)keyboard_hid_report, 8);
    }

    leds_write();

    return true;
}

int main(void) {
    leds_init();
    matrix_init();
    keyboard_init(next_keyboard_hid_report);
    usb_device_init();

    // Wait until configured
    while (!configured) {
        tight_loop_contents();
    }

    // After we're configured, setup a repeating timer for scanning the key matrix
    add_repeating_timer_ms(-MATRIX_SCAN_INTERVAL_MS, matrix_scan_timer_cb, NULL, &matrix_scan_timer);

    while (1) {
        tight_loop_contents();
    }
}
