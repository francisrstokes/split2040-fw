#include "CppUTestExt/MockSupport_c.h"
#include "hardware/gpio.h"

void check_gpio_param(uint gpio) {
    mock_c()->actualCall("check_gpio_param")
        ->withUnsignedIntParameters("gpio", gpio);
}

void gpio_set_function(uint gpio, gpio_function_t fn) {
    mock_c()->actualCall("gpio_set_function")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withUnsignedIntParameters("fn", fn);
}

void gpio_set_function_masked(uint32_t gpio_mask, gpio_function_t fn) {
    mock_c()->actualCall("gpio_set_function_masked")
        ->withUnsignedIntParameters("gpio_mask", gpio_mask)
        ->withUnsignedIntParameters("fn", fn);
}

void gpio_set_function_masked64(uint64_t gpio_mask, gpio_function_t fn) {
    mock_c()->actualCall("gpio_set_function_masked64")
        ->withUnsignedLongLongIntParameters("gpio_mask", gpio_mask)
        ->withUnsignedIntParameters("fn", fn);
}

gpio_function_t gpio_get_function(uint gpio) {
    mock_c()->actualCall("gpio_get_function")
        ->withUnsignedIntParameters("gpio", gpio);
    return (gpio_function_t)(mock_c()->returnUnsignedIntValueOrDefault(0));
}

void gpio_set_pulls(uint gpio, bool up, bool down) {
    mock_c()->actualCall("gpio_set_pulls")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withBoolParameters("up", up)
        ->withBoolParameters("down", down);
}

void gpio_pull_up(uint gpio) {
    mock_c()->actualCall("gpio_pull_up")->withUnsignedIntParameters("gpio", gpio);
}

bool gpio_is_pulled_up(uint gpio) {
    mock_c()->actualCall("gpio_is_pulled_up")->withUnsignedIntParameters("gpio", gpio);
    return mock_c()->returnBoolValueOrDefault(false);
}

void gpio_pull_down(uint gpio) {
    mock_c()->actualCall("gpio_pull_down")->withUnsignedIntParameters("gpio", gpio);
}

bool gpio_is_pulled_down(uint gpio) {
    mock_c()->actualCall("gpio_is_pulled_down")->withUnsignedIntParameters("gpio", gpio);
    return mock_c()->returnBoolValueOrDefault(false);
}

void gpio_disable_pulls(uint gpio) {
    mock_c()->actualCall("gpio_disable_pulls")->withUnsignedIntParameters("gpio", gpio);
}

void gpio_set_irqover(uint gpio, uint value) {
    mock_c()->actualCall("gpio_set_irqover")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withUnsignedIntParameters("value", value);
}

void gpio_set_outover(uint gpio, uint value) {
    mock_c()->actualCall("gpio_set_outover")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withUnsignedIntParameters("value", value);
}

void gpio_set_inover(uint gpio, uint value) {
    mock_c()->actualCall("gpio_set_inover")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withUnsignedIntParameters("value", value);
}

void gpio_set_oeover(uint gpio, uint value) {
    mock_c()->actualCall("gpio_set_oeover")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withUnsignedIntParameters("value", value);
}

void gpio_set_input_enabled(uint gpio, bool enabled) {
    mock_c()->actualCall("gpio_set_input_enabled")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withBoolParameters("enabled", enabled);
}

void gpio_set_input_hysteresis_enabled(uint gpio, bool enabled) {
    mock_c()->actualCall("gpio_set_input_hysteresis_enabled")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withBoolParameters("enabled", enabled);
}

bool gpio_is_input_hysteresis_enabled(uint gpio) {
    mock_c()->actualCall("gpio_is_input_hysteresis_enabled")
        ->withUnsignedIntParameters("gpio", gpio);
    return (bool)(mock_c()->returnBoolValueOrDefault(false));
}

void gpio_set_slew_rate(uint gpio, enum gpio_slew_rate slew) {
    mock_c()->actualCall("gpio_set_slew_rate")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withUnsignedIntParameters("slew", slew);
}

enum gpio_slew_rate gpio_get_slew_rate(uint gpio) {
    mock_c()->actualCall("gpio_get_slew_rate")
        ->withUnsignedIntParameters("gpio", gpio);
    return (enum gpio_slew_rate)(mock_c()->returnUnsignedIntValueOrDefault(0));
}

void gpio_set_drive_strength(uint gpio, enum gpio_drive_strength drive) {
    mock_c()->actualCall("gpio_set_drive_strength")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withUnsignedIntParameters("drive", drive);
}

enum gpio_drive_strength gpio_get_drive_strength(uint gpio) {
    mock_c()->actualCall("gpio_get_drive_strength")
        ->withUnsignedIntParameters("gpio", gpio);
    return (enum gpio_drive_strength)(mock_c()->returnUnsignedIntValueOrDefault(0));
}

void gpio_set_irq_enabled(uint gpio, uint32_t event_mask, bool enabled) {
    mock_c()->actualCall("gpio_set_irq_enabled")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withUnsignedIntParameters("event_mask", event_mask)
        ->withBoolParameters("enabled", enabled);
}

void gpio_set_irq_callback(gpio_irq_callback_t callback) {
    mock_c()->actualCall("gpio_set_irq_callback")
        ->withPointerParameters("callback", (void*)callback);
}

void gpio_set_irq_enabled_with_callback(uint gpio, uint32_t event_mask, bool enabled, gpio_irq_callback_t callback) {
    mock_c()->actualCall("gpio_set_irq_enabled_with_callback")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withUnsignedIntParameters("event_mask", event_mask)
        ->withBoolParameters("enabled", enabled)
        ->withPointerParameters("callback", (void*)callback);
}

void gpio_set_dormant_irq_enabled(uint gpio, uint32_t event_mask, bool enabled) {
    mock_c()->actualCall("gpio_set_dormant_irq_enabled")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withUnsignedIntParameters("event_mask", event_mask)
        ->withBoolParameters("enabled", enabled);
}

void gpio_add_raw_irq_handler_with_order_priority_masked(uint32_t gpio_mask, irq_handler_t handler, uint8_t order_priority) {
    mock_c()->actualCall("gpio_add_raw_irq_handler_with_order_priority_masked")
        ->withUnsignedIntParameters("gpio_mask", gpio_mask)
        ->withPointerParameters("handler", (void*)handler)
        ->withUnsignedIntParameters("order_priority", order_priority);
}

void gpio_add_raw_irq_handler_with_order_priority_masked64(uint64_t gpio_mask, irq_handler_t handler, uint8_t order_priority) {
    mock_c()->actualCall("gpio_add_raw_irq_handler_with_order_priority_masked64")
        ->withUnsignedLongLongIntParameters("gpio_mask", gpio_mask)
        ->withPointerParameters("handler", (void*)handler)
        ->withUnsignedIntParameters("order_priority", order_priority);
}

void gpio_add_raw_irq_handler_masked(uint32_t gpio_mask, irq_handler_t handler) {
    mock_c()->actualCall("gpio_add_raw_irq_handler_masked")
        ->withUnsignedIntParameters("gpio_mask", gpio_mask)
        ->withPointerParameters("handler", (void*)handler);
}

void gpio_add_raw_irq_handler_masked64(uint64_t gpio_mask, irq_handler_t handler) {
    mock_c()->actualCall("gpio_add_raw_irq_handler_masked64")
        ->withUnsignedLongLongIntParameters("gpio_mask", gpio_mask)
        ->withPointerParameters("handler", (void*)handler);
}

void gpio_add_raw_irq_handler(uint gpio, irq_handler_t handler) {
    mock_c()->actualCall("gpio_add_raw_irq_handler")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withPointerParameters("handler", (void*)handler);
}

void gpio_remove_raw_irq_handler_masked(uint32_t gpio_mask, irq_handler_t handler) {
    mock_c()->actualCall("gpio_remove_raw_irq_handler_masked")
        ->withUnsignedIntParameters("gpio_mask", gpio_mask)
        ->withPointerParameters("handler", (void*)handler);
}

void gpio_remove_raw_irq_handler_masked64(uint64_t gpio_mask, irq_handler_t handler) {
    mock_c()->actualCall("gpio_remove_raw_irq_handler_masked64")
        ->withUnsignedLongLongIntParameters("gpio_mask", gpio_mask)
        ->withPointerParameters("handler", (void*)handler);
}

void gpio_remove_raw_irq_handler(uint gpio, irq_handler_t handler) {
    mock_c()->actualCall("gpio_remove_raw_irq_handler")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withPointerParameters("handler", (void*)handler);
}

void gpio_init(uint gpio) {
    mock_c()->actualCall("gpio_init")
        ->withUnsignedIntParameters("gpio", gpio);
}

void gpio_deinit(uint gpio) {
    mock_c()->actualCall("gpio_deinit")
        ->withUnsignedIntParameters("gpio", gpio);
}

void gpio_init_mask(uint gpio_mask) {
    mock_c()->actualCall("gpio_init_mask")
        ->withUnsignedIntParameters("gpio_mask", gpio_mask);
}

bool gpio_get(uint gpio) {
    mock_c()->actualCall("gpio_get")
        ->withUnsignedIntParameters("gpio", gpio);
    return (bool)(mock_c()->returnBoolValueOrDefault(false));
}

uint32_t gpio_get_all(void) {
    mock_c()->actualCall("gpio_get_all");
    return (uint32_t)(mock_c()->returnUnsignedIntValueOrDefault(0));
}

uint64_t gpio_get_all64(void) {
    mock_c()->actualCall("gpio_get_all64");
    return (uint64_t)(mock_c()->returnUnsignedLongLongIntValueOrDefault(0));
}

void gpio_set_mask(uint32_t mask) {
    mock_c()->actualCall("gpio_set_mask")
        ->withUnsignedIntParameters("mask", mask);
}

void gpio_set_mask64(uint64_t mask) {
    mock_c()->actualCall("gpio_set_mask64")
        ->withUnsignedLongLongIntParameters("mask", mask);
}

void gpio_set_mask_n(uint n, uint32_t mask) {
    mock_c()->actualCall("gpio_set_mask_n")
        ->withUnsignedIntParameters("n", n)
        ->withUnsignedIntParameters("mask", mask);
}

void gpio_clr_mask(uint32_t mask) {
    mock_c()->actualCall("gpio_clr_mask")
        ->withUnsignedIntParameters("mask", mask);
}

void gpio_clr_mask64(uint64_t mask) {
    mock_c()->actualCall("gpio_clr_mask64")
        ->withUnsignedLongLongIntParameters("mask", mask);
}

void gpio_clr_mask_n(uint n, uint32_t mask) {
    mock_c()->actualCall("gpio_clr_mask_n")
        ->withUnsignedIntParameters("n", n)
        ->withUnsignedIntParameters("mask", mask);
}

void gpio_xor_mask(uint32_t mask) {
    mock_c()->actualCall("gpio_xor_mask")
        ->withUnsignedIntParameters("mask", mask);
}

void gpio_xor_mask64(uint64_t mask) {
    mock_c()->actualCall("gpio_xor_mask64")
        ->withUnsignedLongLongIntParameters("mask", mask);
}

void gpio_xor_mask_n(uint n, uint32_t mask) {
    mock_c()->actualCall("gpio_xor_mask_n")
        ->withUnsignedIntParameters("n", n)
        ->withUnsignedIntParameters("mask", mask);
}

void gpio_put_masked(uint32_t mask, uint32_t value) {
    mock_c()->actualCall("gpio_put_masked")
        ->withUnsignedIntParameters("mask", mask)
        ->withUnsignedIntParameters("value", value);
}

void gpio_put_masked64(uint64_t mask, uint64_t value) {
    mock_c()->actualCall("gpio_put_masked64")
        ->withUnsignedLongLongIntParameters("mask", mask)
        ->withUnsignedLongLongIntParameters("value", value);
}

void gpio_put_masked_n(uint n, uint32_t mask, uint32_t value) {
    mock_c()->actualCall("gpio_put_masked_n")
        ->withUnsignedIntParameters("n", n)
        ->withUnsignedIntParameters("mask", mask)
        ->withUnsignedIntParameters("value", value);
}

void gpio_put_all(uint32_t value) {
    mock_c()->actualCall("gpio_put_all")
        ->withUnsignedIntParameters("value", value);
}

void gpio_put_all64(uint64_t value) {
    mock_c()->actualCall("gpio_put_all64")
        ->withUnsignedLongLongIntParameters("value", value);
}

void gpio_put(uint gpio, bool value) {
    mock_c()->actualCall("gpio_put")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withBoolParameters("value", value);
}

bool gpio_get_out_level(uint gpio) {
    mock_c()->actualCall("gpio_get_out_level")
        ->withUnsignedIntParameters("gpio", gpio);
    return (bool)(mock_c()->returnBoolValueOrDefault(false));
}

void gpio_set_dir_out_masked(uint32_t mask) {
    mock_c()->actualCall("gpio_set_dir_out_masked")
        ->withUnsignedIntParameters("mask", mask);
}

void gpio_set_dir_out_masked64(uint64_t mask) {
    mock_c()->actualCall("gpio_set_dir_out_masked64")
        ->withUnsignedLongLongIntParameters("mask", mask);
}

void gpio_set_dir_in_masked(uint32_t mask) {
    mock_c()->actualCall("gpio_set_dir_in_masked")
        ->withUnsignedIntParameters("mask", mask);
}

void gpio_set_dir_in_masked64(uint64_t mask) {
    mock_c()->actualCall("gpio_set_dir_in_masked64")
        ->withUnsignedLongLongIntParameters("mask", mask);
}

void gpio_set_dir_masked(uint32_t mask, uint32_t value) {
    mock_c()->actualCall("gpio_set_dir_masked")
        ->withUnsignedIntParameters("mask", mask)
        ->withUnsignedIntParameters("value", value);
}

void gpio_set_dir_masked64(uint64_t mask, uint64_t value) {
    mock_c()->actualCall("gpio_set_dir_masked64")
        ->withUnsignedLongLongIntParameters("mask", mask)
        ->withUnsignedLongLongIntParameters("value", value);
}

void gpio_set_dir_all_bits(uint32_t values) {
    mock_c()->actualCall("gpio_set_dir_all_bits")
        ->withUnsignedIntParameters("values", values);
}

void gpio_set_dir_all_bits64(uint64_t values) {
    mock_c()->actualCall("gpio_set_dir_all_bits64")
        ->withUnsignedLongLongIntParameters("values", values);
}

void gpio_set_dir(uint gpio, bool out) {
    mock_c()->actualCall("gpio_set_dir")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withBoolParameters("out", out);
}

bool gpio_is_dir_out(uint gpio) {
    mock_c()->actualCall("gpio_is_dir_out")
        ->withUnsignedIntParameters("gpio", gpio);
    return (bool)(mock_c()->returnBoolValueOrDefault(false));
}

uint gpio_get_dir(uint gpio) {
    mock_c()->actualCall("gpio_get_dir")
        ->withUnsignedIntParameters("gpio", gpio);
    return (uint)(mock_c()->returnUnsignedIntValueOrDefault(0));
}

#if PICO_SECURE
void gpio_assign_to_ns(uint gpio, bool ns) {
    mock_c()->actualCall("gpio_assign_to_ns")
        ->withUnsignedIntParameters("gpio", gpio)
        ->withBoolParameters("ns", ns);
}
#endif

void gpio_debug_pins_init(void) {
    mock_c()->actualCall("gpio_debug_pins_init");
}
