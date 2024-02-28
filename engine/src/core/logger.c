#include "logger.h" 
#include "platform/platform.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef struct logger_system_state {
    b8 initialized;
} logger_system_state;
static logger_system_state* state_ptr;

b8 initialize_logger(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(logger_system_state);
    if (state == 0) {
        return true;
    }

    state_ptr = state;
    state_ptr->initialized = true;
    return true;
}

void shutdown_logger(void* state) {
    // finish queue
    state_ptr = 0;
}

void log_output(log_level level, const char* msg, ...) {
    const char* levels[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
    b8 isError = level <= LOG_LEVEL_ERROR;

    const i32 msg_length = 32000;
    char output[msg_length];
    memset(output, 0, sizeof(output));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, msg); // start after msg arg
    vsnprintf(output, sizeof(output), msg, arg_ptr);
    va_end(arg_ptr);

    char new_output[msg_length];

    sprintf(new_output, "%s%s\n", levels[level], output); // prepend level label before output
    if (isError)
    {
        platform_console_write(new_output, level);
    }
    else
    {
        platform_console_write(new_output, level);
    }
}