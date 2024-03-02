#include "logger.h"
#include "asserts.h"
#include "core/vstring.h"
#include "core/vmemory.h"
#include "platform/platform.h"
#include "platform/filesystem.h"
#include <stdarg.h>

typedef struct logger_system_state {
    file_handle log_file_handle;
} logger_system_state;
static logger_system_state* state_ptr;

void append_to_log_file(const char* msg) {
    if (state_ptr && state_ptr->log_file_handle.is_valid) {
        u64 length = string_length(msg);
        u64 written = 0;
        if (!filesystem_write(&state_ptr->log_file_handle, length, msg, &written)) {
            platform_console_write_error("ERROR: Unable to write to log file.", LOG_LEVEL_ERROR);
        }
    }
}

b8 initialize_logger(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(logger_system_state);
    if (state == 0) {
        return true;
    }

    state_ptr = state;

    if (!filesystem_open("console.log", FILE_MODE_WRITE, false, &state_ptr->log_file_handle)) {
        platform_console_write_error("ERROR: Unaable to open console log file for writing.", LOG_LEVEL_ERROR);
        return false;
    }


    return true;
}

void shutdown_logger(void* state) {
    // finish queue
    state_ptr = 0;
}

void log_output(log_level level, const char* msg, ...) {
    const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    b8 is_error = level < LOG_LEVEL_WARN;

    char out_message[32000];
    vzero_memory(out_message, sizeof(out_message));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, msg);
    string_format_v(out_message, msg, arg_ptr);
    va_end(arg_ptr);

    string_format(out_message, "%s%s\n", level_strings[level], out_message);
    if (is_error) {
        platform_console_write_error(out_message, level);
    } else {
        platform_console_write(out_message, level);
    }
    append_to_log_file(out_message);
}