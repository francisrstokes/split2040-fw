#ifndef _HARDWARE_IRQ_H
#define _HARDWARE_IRQ_H

#ifndef PICO_MAX_SHARED_IRQ_HANDLERS
#define PICO_MAX_SHARED_IRQ_HANDLERS 4
#endif

#ifndef PICO_DISABLE_SHARED_IRQ_HANDLERS
#define PICO_DISABLE_SHARED_IRQ_HANDLERS 0
#endif

#ifndef PICO_VTABLE_PER_CORE
#define PICO_VTABLE_PER_CORE 0
#endif

#ifndef __ASSEMBLER__

#include "pico.h"
// #include "hardware/address_mapped.h"
// #include "hardware/regs/intctrl.h"

// #include "pico/platform/cpu_regs.h"

#ifndef PICO_DEFAULT_IRQ_PRIORITY
#define PICO_DEFAULT_IRQ_PRIORITY 0x80
#endif

#define PICO_LOWEST_IRQ_PRIORITY 0xff
#define PICO_HIGHEST_IRQ_PRIORITY 0x00

#ifndef PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY
#define PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY 0x80
#endif

#define PICO_SHARED_IRQ_HANDLER_HIGHEST_ORDER_PRIORITY 0xff
#define PICO_SHARED_IRQ_HANDLER_LOWEST_ORDER_PRIORITY 0x00

#ifndef PARAM_ASSERTIONS_ENABLED_HARDWARE_IRQ
#ifdef PARAM_ASSERTIONS_ENABLED_IRQ
#define PARAM_ASSERTIONS_ENABLED_HARDWARE_IRQ PARAM_ASSERTIONS_ENABLED_IRQ
#else
#define PARAM_ASSERTIONS_ENABLED_HARDWARE_IRQ 0
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*irq_handler_t)(void);

void check_irq_param(uint num);

void irq_set_priority(uint num, uint8_t hardware_priority);

uint irq_get_priority(uint num);

void irq_set_enabled(uint num, bool enabled);

bool irq_is_enabled(uint num);

void irq_set_mask_enabled(uint32_t mask, bool enabled);

void irq_set_mask_n_enabled(uint n, uint32_t mask, bool enabled);

void irq_set_exclusive_handler(uint num, irq_handler_t handler);

irq_handler_t irq_get_exclusive_handler(uint num);

void irq_add_shared_handler(uint num, irq_handler_t handler, uint8_t order_priority);

void irq_remove_handler(uint num, irq_handler_t handler);

bool irq_has_handler(uint num);

bool irq_has_shared_handler(uint num);

irq_handler_t irq_get_vtable_handler(uint num);

void irq_clear(uint int_num);

void irq_set_pending(uint num);

void runtime_init_per_core_irq_priorities(void);

void irq_init_priorities(void);

void user_irq_claim(uint irq_num);

void user_irq_unclaim(uint irq_num);

int user_irq_claim_unused(bool required);

bool user_irq_is_claimed(uint irq_num);

void __unhandled_user_irq(void);

#ifdef __riscv
enum riscv_vector_num {
    RISCV_VEC_MACHINE_EXCEPTION = 0,
    RISCV_VEC_MACHINE_SOFTWARE_IRQ = 3,
    RISCV_VEC_MACHINE_TIMER_IRQ = 7,
    RISCV_VEC_MACHINE_EXTERNAL_IRQ = 11,
};

irq_handler_t irq_set_riscv_vector_handler(enum riscv_vector_num index, irq_handler_t handler);
#endif

#if PICO_SECURE
void irq_assign_to_ns(uint irq_num, bool ns);
#endif

#ifdef __cplusplus
}
#endif

#endif
#endif
