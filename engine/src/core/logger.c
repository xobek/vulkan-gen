#include "logger.h" 

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 initalize_logger() {
    // create log file
    return TRUE;
}

void shutdown_logger() {
    // finish queue
}

void log_output(log_level level, const char* msg, ...) {
    const char* levels[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
    // b8 isError = level <= LOG_LEVEL_ERROR;

    char output[32000];
    memset(output, 0, sizeof(output));

    __builtin_va_list arg_ptr;
    va_start(arg_ptr, msg); // start after msg arg
    vsnprintf(output, sizeof(output), msg, arg_ptr);
    va_end(arg_ptr);

    char new_output[32000];

    sprintf(new_output, "%s%s\n", levels[level], output); // prepend level label before output
    printf("%s", new_output);
}