#pragma once 

#include "defines.h"

#define ASSERTIONS_ENABLED 

#ifdef ASSERTIONS_ENABLED
    #if _MSC_VER
        #include <intrin.h>
        #define debugBreak() __debugbreak()
    #else
        #define debugBreak() __builtin_trap()
    #endif

    API void report_assertion_failure(const char* expression, const char* msg, const char* file, i32 line);

    #define ASSERT(expr) \
        { \
            if (expr) {} else \
            { \
                report_assertion_failure(#expr, "", __FILE__, __LINE__); \
                debugBreak(); \
            } \
        }
        
    #define ASSERT_MSG(expr, msg) \
        { \
            if (expr) {} else \
            { \
                report_assertion_failure(#expr, msg, __FILE__, __LINE__); \
                debugBreak(); \
            } \
        }

    #ifdef _DEBUG
        #define ASSERT_DEBUG(expr) ASSERT(expr)
        #define ASSERT_DEBUG_MSG(expr, msg) ASSERT_MSG(expr, msg)
    #else
        #define ASSERT_DEBUG(expr)
        #define ASSERT_DEBUG_MSG(expr, msg)
    #endif

#else 
    #define ASSERT(expr)
    #define ASSERT_MSG(expr, msg)
    #define ASSERT_DEBUG(expr)
    #define ASSERT_DEBUG_MSG(expr, msg)
#endif