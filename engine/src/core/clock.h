#pragma once 

#include "defines.h"

typedef struct clock {
    f64 start_time;
    f64 elapsed;
} clock;

API void clock_update(clock* c);

API void clock_start(clock* c);

API void clock_stop(clock* c);