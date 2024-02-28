# pragma once 

#include "defines.h"

#define LOG_WARN_ENABLED 1 
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

#if RELEASE == 1 
    #define LOG_DEBUG_ENABLED 0
    #define LOG_TRACE_ENABLED 0
#endif


typedef enum log_level {
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE
} log_level;

/**
 * @brief Initializes logging system. Call twice; once with state = 0 to get required memory size,
 * then a second time passing allocated memory to state.
 * 
 * @param memory_requirement A pointer to hold the required memory size of internal state.
 * @param state 0 if just requesting memory requirement, otherwise allocated block of memory.
 * @return b8 True on success; otherwise false.
 */
b8 initialize_logger(u64* memory_requirement, void* state);
void shutdown_logger(void* state);

API void log_output(log_level level, const char* msg, ...);


#if LOG_WARN_ENABLED == 1
    #ifndef WARN 
        #define WARN(msg, ...) log_output(LOG_LEVEL_WARN, msg, ##__VA_ARGS__)
    #endif
#else 
    #define WARN(msg, ...) // do nothing if not enabled
#endif

#if LOG_INFO_ENABLED == 1
    #ifndef INFO 
        #define INFO(msg, ...) log_output(LOG_LEVEL_INFO, msg, ##__VA_ARGS__)
    #endif
#else
    #define INFO(msg, ...) // do nothing 
#endif

#if LOG_DEBUG_ENABLED == 1
    #ifndef DEBUG 
        #define DEBUG(msg, ...) log_output(LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)
    #endif
#else
    #define DEBUG(msg, ...) // do nothing
#endif

#if LOG_TRACE_ENABLED == 1
    #ifndef TRACE 
        #define TRACE(msg, ...) log_output(LOG_LEVEL_TRACE, msg, ##__VA_ARGS__)
    #endif
#else
    #define TRACE(msg, ...) // do nothing   
#endif

#ifndef FATAL 
    #define FATAL(msg, ...) log_output(LOG_LEVEL_FATAL, msg, ##__VA_ARGS__)
#endif

#ifndef ERROR 
    #define ERROR(msg, ...) log_output(LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)
#endif

