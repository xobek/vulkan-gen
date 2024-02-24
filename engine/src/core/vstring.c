#include "core/vstring.h"
#include "core/vmemory.h"

#include "string.h"

char* vstring_duplicate(const char* str) {
    u64 length = string_length(str);
    char* copy = vallocate(length + 1, MEMORY_TAG_STRING);
    vcopy_memory(copy, str, length + 1);
    return copy;
}

u64 string_length(const char* str) {
    return strlen(str);
}

char* string_concat(const char* string1, const char* string2) {
    u64 length1 = string_length(string1);
    u64 length2 = string_length(string2);
    char* result = vallocate(length1 + length2 + 1, MEMORY_TAG_STRING);
    vcopy_memory(result, string1, length1);
    vcopy_memory(result + length1, string2, length2 + 1);
    return result;
}