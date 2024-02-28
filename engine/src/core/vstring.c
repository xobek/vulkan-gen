#include "core/vstring.h"
#include "core/vmemory.h"

#include "string.h"
#include <stdio.h>
#include <stdarg.h>

u64 string_length(const char* str) {
    return strlen(str);
}

char* string_duplicate(const char* str) {
    u64 length = string_length(str);
    char* copy = vallocate(length + 1, MEMORY_TAG_STRING);
    vcopy_memory(copy, str, length + 1);
    return copy;
}

char* string_concat(const char* string1, const char* string2) {
    u64 length1 = string_length(string1);
    u64 length2 = string_length(string2);
    char* result = vallocate(length1 + length2 + 1, MEMORY_TAG_STRING);
    vcopy_memory(result, string1, length1);
    vcopy_memory(result + length1, string2, length2 + 1);
    return result;
}

b8 strings_equal(const char* string1, const char* string2) {
    return strcmp(string1, string2) == 0;
}


i32 string_format(char* dest, const char* format, ...) {
    if (dest) {
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, format);
        i32 written = string_format_v(dest, format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }
    return -1;
}

i32 string_format_v(char* dest, const char* format, void* va_listp) {
    if (dest) {
        char buffer[32000];
        i32 written = vsnprintf(buffer, 32000, format, va_listp);
        buffer[written] = 0;
        vcopy_memory(dest, buffer, written + 1);

        return written;
    }
    return -1;
}