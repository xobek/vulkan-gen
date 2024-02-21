#pragma once 

#include "defines.h"

typedef struct platform_state {
    void* internal_state;
} platform_state;

API b8 platform_startup(platform_state* pstate, const char* app_name, i32 x, i32 y, i32 w, i32 h);
API void platform_shutdown(platform_state* state);
API b8 platform_pump_messages(platform_state* state);

void* platform_allocate(u64 size, b8 aligned);
void platform_free(void* block, b8 aligned);
void* platform_zero_memory(void* block, u64 size);
void* platform_copy_memory(void* dest, const void* source, u64 size);
void* platform_set_memory(void* dest, i32 value, u64 size);

void platform_console_write(const char* msg, u8 color);
void platform_console_write_error(const char* msg, u8 color);
void platform_sleep(u64 ms);
f64 platform_get_absolute_time();