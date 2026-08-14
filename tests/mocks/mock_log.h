// mock_log.h
#ifndef MOCK_LOG_H
#define MOCK_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#include "machines/machine.h"
#include "log.h"

typedef struct StLog_t {
    void (*log_str)(char* str);
    void (*log_int)(uint32_t value);
    void (*log_hex)(uint32_t value, bool pad);
    void (*log_ptr)(void* ptr);
} StLog_t;

typedef struct LogInternals_t {
    int _unused;
} LogInternals_t;

// Mock API
void mock_log_use_mocks(bool use_mocks);
StLog_t* mock_log_get_fn_ptr_struct(void);
LogInternals_t* mock_log_get_internals(void);
void mock_log_expect_calls(bool should_expect);

#ifdef __cplusplus
}
#endif

#endif
