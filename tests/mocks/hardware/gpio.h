#ifndef _HARDWARE_GPIO_H
#define _HARDWARE_GPIO_H

#include "pico.h"
#include "hardware/irq.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO: This needs to end up in the right spot later
typedef void (*irq_handler_t)(void);

enum gpio_dir {
    GPIO_OUT = 1u,
    GPIO_IN = 0u,
};

enum gpio_irq_level {
    GPIO_IRQ_LEVEL_LOW = 0x1u,
    GPIO_IRQ_LEVEL_HIGH = 0x2u,
    GPIO_IRQ_EDGE_FALL = 0x4u,
    GPIO_IRQ_EDGE_RISE = 0x8u,
};

typedef void (*gpio_irq_callback_t)(uint gpio, uint32_t event_mask);

enum gpio_override {
    GPIO_OVERRIDE_NORMAL = 0,
    GPIO_OVERRIDE_INVERT = 1,
    GPIO_OVERRIDE_LOW = 2,
    GPIO_OVERRIDE_HIGH = 3,
};

enum gpio_slew_rate {
    GPIO_SLEW_RATE_SLOW = 0,
    GPIO_SLEW_RATE_FAST = 1
};

enum gpio_drive_strength {
    GPIO_DRIVE_STRENGTH_2MA = 0,
    GPIO_DRIVE_STRENGTH_4MA = 1,
    GPIO_DRIVE_STRENGTH_8MA = 2,
    GPIO_DRIVE_STRENGTH_12MA = 3
};

typedef enum gpio_function_rp2040 {
    GPIO_FUNC_XIP = 0, ///< Select XIP as GPIO pin function
    GPIO_FUNC_SPI = 1, ///< Select SPI as GPIO pin function
    GPIO_FUNC_UART = 2, ///< Select UART as GPIO pin function
    GPIO_FUNC_I2C = 3, ///< Select I2C as GPIO pin function
    GPIO_FUNC_PWM = 4, ///< Select PWM as GPIO pin function
    GPIO_FUNC_SIO = 5, ///< Select SIO as GPIO pin function
    GPIO_FUNC_PIO0 = 6, ///< Select PIO0 as GPIO pin function
    GPIO_FUNC_PIO1 = 7, ///< Select PIO1 as GPIO pin function
    GPIO_FUNC_GPCK = 8, ///< Select GPCK as GPIO pin function
    GPIO_FUNC_USB = 9, ///< Select USB as GPIO pin function
    GPIO_FUNC_NULL = 0x1f, ///< Select NULL as GPIO pin function
} gpio_function_t;

void check_gpio_param(uint gpio);

void gpio_set_function(uint gpio, gpio_function_t fn);
void gpio_set_function_masked(uint32_t gpio_mask, gpio_function_t fn);
void gpio_set_function_masked64(uint64_t gpio_mask, gpio_function_t fn);
gpio_function_t gpio_get_function(uint gpio);

void gpio_set_pulls(uint gpio, bool up, bool down);
void gpio_pull_up(uint gpio);
bool gpio_is_pulled_up(uint gpio);
void gpio_pull_down(uint gpio);
bool gpio_is_pulled_down(uint gpio);
void gpio_disable_pulls(uint gpio);

void gpio_set_irqover(uint gpio, uint value);
void gpio_set_outover(uint gpio, uint value);
void gpio_set_inover(uint gpio, uint value);
void gpio_set_oeover(uint gpio, uint value);

void gpio_set_input_enabled(uint gpio, bool enabled);

void gpio_set_input_hysteresis_enabled(uint gpio, bool enabled);
bool gpio_is_input_hysteresis_enabled(uint gpio);

void gpio_set_slew_rate(uint gpio, enum gpio_slew_rate slew);
enum gpio_slew_rate gpio_get_slew_rate(uint gpio);

void gpio_set_drive_strength(uint gpio, enum gpio_drive_strength drive);
enum gpio_drive_strength gpio_get_drive_strength(uint gpio);

void gpio_set_irq_enabled(uint gpio, uint32_t event_mask, bool enabled);

void gpio_set_irq_callback(gpio_irq_callback_t callback);

void gpio_set_irq_enabled_with_callback(uint gpio, uint32_t event_mask, bool enabled, gpio_irq_callback_t callback);

void gpio_set_dormant_irq_enabled(uint gpio, uint32_t event_mask, bool enabled);

void gpio_add_raw_irq_handler_with_order_priority_masked(uint32_t gpio_mask, irq_handler_t handler, uint8_t order_priority);
void gpio_add_raw_irq_handler_with_order_priority_masked64(uint64_t gpio_mask, irq_handler_t handler, uint8_t order_priority);

void gpio_add_raw_irq_handler_masked(uint32_t gpio_mask, irq_handler_t handler);
void gpio_add_raw_irq_handler_masked64(uint64_t gpio_mask, irq_handler_t handler);
void gpio_add_raw_irq_handler(uint gpio, irq_handler_t handler);

void gpio_remove_raw_irq_handler_masked(uint32_t gpio_mask, irq_handler_t handler);
void gpio_remove_raw_irq_handler_masked64(uint64_t gpio_mask, irq_handler_t handler);
void gpio_remove_raw_irq_handler(uint gpio, irq_handler_t handler);

void gpio_init(uint gpio);
void gpio_deinit(uint gpio);
void gpio_init_mask(uint gpio_mask);

bool gpio_get(uint gpio);
uint32_t gpio_get_all(void);
uint64_t gpio_get_all64(void);

void gpio_set_mask(uint32_t mask);
void gpio_set_mask64(uint64_t mask);
void gpio_set_mask_n(uint n, uint32_t mask);

void gpio_clr_mask(uint32_t mask);
void gpio_clr_mask64(uint64_t mask);
void gpio_clr_mask_n(uint n, uint32_t mask);

void gpio_xor_mask(uint32_t mask);
void gpio_xor_mask64(uint64_t mask);
void gpio_xor_mask_n(uint n, uint32_t mask);

void gpio_put_masked(uint32_t mask, uint32_t value);
void gpio_put_masked64(uint64_t mask, uint64_t value);
void gpio_put_masked_n(uint n, uint32_t mask, uint32_t value);

void gpio_put_all(uint32_t value);
void gpio_put_all64(uint64_t value);

void gpio_put(uint gpio, bool value);

bool gpio_get_out_level(uint gpio);

void gpio_set_dir_out_masked(uint32_t mask);
void gpio_set_dir_out_masked64(uint64_t mask);

void gpio_set_dir_in_masked(uint32_t mask);
void gpio_set_dir_in_masked64(uint64_t mask);

void gpio_set_dir_masked(uint32_t mask, uint32_t value);
void gpio_set_dir_masked64(uint64_t mask, uint64_t value);

void gpio_set_dir_all_bits(uint32_t values);
void gpio_set_dir_all_bits64(uint64_t values);

void gpio_set_dir(uint gpio, bool out);

bool gpio_is_dir_out(uint gpio);
uint gpio_get_dir(uint gpio);

#if PICO_SECURE
void gpio_assign_to_ns(uint gpio, bool ns);
#endif

extern void gpio_debug_pins_init(void);

#ifdef __cplusplus
}
#endif

#endif // _GPIO_H_