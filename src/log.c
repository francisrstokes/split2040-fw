#include "log.h"
#include "kb_config.h"
#include "string.h"

// statics
static const char* ascii_digits = "0123456789abcdef";

// public functions
void log_str(char* str) {
    kb_config_log_to_ring_buffer(str, strlen(str));
}

void log_int(uint32_t value) {
    char buf[10] = {0};
    uint8_t index = 10-1;

    do {
        buf[index--] = ascii_digits[value % 10];
    } while ((value /= 10) != 0);
    index++;

    kb_config_log_to_ring_buffer(&buf[index], 10 - index);
}

void log_hex(uint32_t value, bool pad) {
    char buf[10] = {0};
    uint8_t index = 10-1;

    do {
        buf[index--] = ascii_digits[value % 16];
    } while ((value /= 16) != 0);

    if (pad) {
        while (index >= 2) {
            buf[index--] = '0';
        }
    }

    buf[index--] = 'x';
    buf[index] = '0';

    kb_config_log_to_ring_buffer(&buf[index], 10 - index);
}

void log_ptr(void* ptr) {
    log_hex((uint32_t)ptr, true);
}
