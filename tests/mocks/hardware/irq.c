#include "CppUTestExt/MockSupport_c.h"

void check_irq_param(uint num) {
    mock_c()->actualCall("check_irq_param")
        ->withUnsignedIntParameters("num", num);
}

void irq_set_priority(uint num, uint8_t hardware_priority) {
    mock_c()->actualCall("irq_set_priority")
        ->withUnsignedIntParameters("num", num)
        ->withUnsignedIntParameters("hardware_priority", hardware_priority);
}

uint irq_get_priority(uint num) {
    mock_c()->actualCall("irq_get_priority")
        ->withUnsignedIntParameters("num", num);
    return (uint)(mock_c()->returnUnsignedIntValueOrDefault(0));
}

void irq_set_enabled(uint num, bool enabled) {
    mock_c()->actualCall("irq_set_enabled")
        ->withUnsignedIntParameters("num", num)
        ->withBoolParameters("enabled", enabled);
}

bool irq_is_enabled(uint num) {
    mock_c()->actualCall("irq_is_enabled")
        ->withUnsignedIntParameters("num", num);
    return (bool)(mock_c()->returnBoolValueOrDefault(false));
}

void irq_set_mask_enabled(uint32_t mask, bool enabled) {
    mock_c()->actualCall("irq_set_mask_enabled")
        ->withUnsignedIntParameters("mask", mask)
        ->withBoolParameters("enabled", enabled);
}

void irq_set_mask_n_enabled(uint n, uint32_t mask, bool enabled) {
    mock_c()->actualCall("irq_set_mask_n_enabled")
        ->withUnsignedIntParameters("n", n)
        ->withUnsignedIntParameters("mask", mask)
        ->withBoolParameters("enabled", enabled);
}

void irq_set_exclusive_handler(uint num, irq_handler_t handler) {
    mock_c()->actualCall("irq_set_exclusive_handler")
        ->withUnsignedIntParameters("num", num)
        ->withPointerParameters("handler", (void*)handler);
}

irq_handler_t irq_get_exclusive_handler(uint num) {
    mock_c()->actualCall("irq_get_exclusive_handler")
        ->withUnsignedIntParameters("num", num);
    return (irq_handler_t)(mock_c()->returnPointerValueOrDefault(NULL));
}

void irq_add_shared_handler(uint num, irq_handler_t handler, uint8_t order_priority) {
    mock_c()->actualCall("irq_add_shared_handler")
        ->withUnsignedIntParameters("num", num)
        ->withPointerParameters("handler", (void*)handler)
        ->withUnsignedIntParameters("order_priority", order_priority);
}

void irq_remove_handler(uint num, irq_handler_t handler) {
    mock_c()->actualCall("irq_remove_handler")
        ->withUnsignedIntParameters("num", num)
        ->withPointerParameters("handler", (void*)handler);
}

bool irq_has_handler(uint num) {
    mock_c()->actualCall("irq_has_handler")
        ->withUnsignedIntParameters("num", num);
    return (bool)(mock_c()->returnBoolValueOrDefault(false));
}

bool irq_has_shared_handler(uint num) {
    mock_c()->actualCall("irq_has_shared_handler")
        ->withUnsignedIntParameters("num", num);
    return (bool)(mock_c()->returnBoolValueOrDefault(false));
}

irq_handler_t irq_get_vtable_handler(uint num) {
    mock_c()->actualCall("irq_get_vtable_handler")
        ->withUnsignedIntParameters("num", num);
    return (irq_handler_t)(mock_c()->returnPointerValueOrDefault(NULL));
}

void irq_clear(uint int_num) {
    mock_c()->actualCall("irq_clear")
        ->withUnsignedIntParameters("int_num", int_num);
}

void irq_set_pending(uint num) {
    mock_c()->actualCall("irq_set_pending")
        ->withUnsignedIntParameters("num", num);
}

void runtime_init_per_core_irq_priorities(void) {
    mock_c()->actualCall("runtime_init_per_core_irq_priorities");
}

void irq_init_priorities(void) {
    mock_c()->actualCall("irq_init_priorities");
}

void user_irq_claim(uint irq_num) {
    mock_c()->actualCall("user_irq_claim")
        ->withUnsignedIntParameters("irq_num", irq_num);
}

void user_irq_unclaim(uint irq_num) {
    mock_c()->actualCall("user_irq_unclaim")
        ->withUnsignedIntParameters("irq_num", irq_num);
}

int user_irq_claim_unused(bool required) {
    mock_c()->actualCall("user_irq_claim_unused")
        ->withBoolParameters("required", required);
    return (int)(mock_c()->returnIntValueOrDefault(0));
}

bool user_irq_is_claimed(uint irq_num) {
    mock_c()->actualCall("user_irq_is_claimed")
        ->withUnsignedIntParameters("irq_num", irq_num);
    return (bool)(mock_c()->returnBoolValueOrDefault(false));
}

void __unhandled_user_irq(void) {
    mock_c()->actualCall("__unhandled_user_irq");
}

#ifdef __riscv
irq_handler_t irq_set_riscv_vector_handler(enum riscv_vector_num index, irq_handler_t handler) {
    mock_c()->actualCall("irq_set_riscv_vector_handler")
        ->withUnsignedIntParameters("index", index)
        ->withPointerParameters("handler", (void*)handler);
    return (irq_handler_t)(mock_c()->returnPointerValueOrDefault(NULL));
}
#endif

#if PICO_SECURE
void irq_assign_to_ns(uint irq_num, bool ns) {
    mock_c()->actualCall("irq_assign_to_ns")
        ->withUnsignedIntParameters("irq_num", irq_num)
        ->withBoolParameters("ns", ns);
}
#endif
