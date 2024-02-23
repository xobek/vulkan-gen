#pragma once 

#include "defines.h"

struct game;

typedef struct app_config { 
    i16 x;
    i16 y;
    i16 width;
    i16 height;
    const char* name;
} app_config;

API b8 application_create(struct game* game_inst);

API b8 application_run();