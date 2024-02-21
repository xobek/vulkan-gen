#include "asserts.h"
#include "logger.h"

void report_assertion_failure(const char* expression, const char* msg, const char* file, int line) {
    FATAL("Assertion failed: %s, file: %s, line: %d", expression, file, line);
}