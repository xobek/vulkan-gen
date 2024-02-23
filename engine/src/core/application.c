#include "application.h"
#include "gametypes.h"
#include "logger.h"

#include <platform/platform.h>
#include <core/vmemory.h>

typedef struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i16 width;
    i16 height;
    f64 last_time;
} application_state;

static b8 initialized = FALSE;
static application_state app_state;

b8 application_create(game* game_inst) {
    if (initialized) {
        ERROR("Application trying to initialize more than once.");
        return FALSE;
    }

    app_state.game_inst = game_inst;

    // SUBSYSTEMS // 
    if (!initialize_logger()) {
        ERROR("Logger subsystem failed to initialize!");
    }

    app_state.is_running = TRUE;
    app_state.is_suspended = FALSE;

    if (!platform_startup(
        &app_state.platform, 
        game_inst->config.name, 
        game_inst->config.x, 
        game_inst->config.y, 
        game_inst->config.width, 
        game_inst->config.height)) {
        ERROR("Platform subsystem failed to start!");
        return FALSE;
    }

    if (!app_state.game_inst->initialize(app_state.game_inst)) {
        ERROR("Game failed to initialize!");
        return FALSE;
    }

    app_state.game_inst->on_resize(app_state.game_inst, app_state.width, app_state.height);

    initialized = TRUE;
    return TRUE;
}

b8 application_run() {
    INFO(get_memory_usage_str());
    while(app_state.is_running) {
        if(!platform_pump_messages(&app_state.platform)) {
            app_state.is_running = FALSE;
        }
        if(!app_state.is_suspended) {
            if (!app_state.game_inst->update(app_state.game_inst, (f32)0)) {
                FATAL("Game update failed, exiting..");
                app_state.is_running = FALSE;
                break;
            }

            if (!app_state.game_inst->render(app_state.game_inst, (f32)0)) {
                FATAL("Game render failed, exiting..");
                app_state.is_running = FALSE;
                break;
            }
        }
    }

    app_state.is_running = FALSE;
    platform_shutdown(&app_state.platform);
    return TRUE;
}