#include "application.h"
#include "gametypes.h"
#include "logger.h"

#include "platform/platform.h"
#include "core/vmemory.h"
#include "core/event.h"
#include "core/input.h"
#include "core/clock.h"

#include "memory/linear_allocator.h"
#include "renderer/renderer_frontend.h"

typedef struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i16 width;
    i16 height;
    clock clock;
    f64 last_time;
    linear_allocator systems_allocator;
    u64 memory_system_memory_requirement;
    void* memory_system_state;
    u64 logging_system_memory_requirement;
    void* logging_system_state;
} application_state;

static application_state* app_state;

b8 application_on_event(u16 code, void* sender, void* listener, event_context context);
b8 application_on_key(u16 code, void* sender, void* listener, event_context context);
b8 application_on_resized(u16 code, void* sender, void* listener_inst, event_context context);

b8 application_create(game* game_inst) {
    if (game_inst->application_state) {
        ERROR("Application trying to initialize more than once.");
        return false;
    }

    game_inst->application_state = vallocate(sizeof(application_state), MEMORY_TAG_APPLICATION);
    app_state = game_inst->application_state;
    app_state->game_inst = game_inst;
    app_state->is_running = false;
    app_state->is_suspended = false;

    u64 systems_allocator_total_size = 64 * 1024 * 1024; // 64MB
    linear_allocator_create(systems_allocator_total_size, 0, &app_state->systems_allocator);


    // SUBSYSTEMS // 

    initialize_memory(&app_state->memory_system_memory_requirement, 0);
    app_state->memory_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->memory_system_memory_requirement);
    initialize_memory(&app_state->memory_system_memory_requirement, app_state->memory_system_state);


    initialize_logger(&app_state->logging_system_memory_requirement, 0);
    app_state->logging_system_state = linear_allocator_allocate(&app_state->systems_allocator, app_state->logging_system_memory_requirement);

    if (!initialize_logger(&app_state->logging_system_memory_requirement, app_state->logging_system_state)) {
        ERROR("Failed to initialize logging system; shutting down..");
        return false;
    }

    input_initialize();
    

    if(!event_initialize()) {
        FATAL("Event system failed initialization!");
        return false;
    }

    event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key); 
    event_register(EVENT_CODE_RESIZED, 0, application_on_resized);

    if (!platform_startup(
        &app_state->platform, 
        game_inst->config.name, 
        game_inst->config.x, 
        game_inst->config.y, 
        game_inst->config.width, 
        game_inst->config.height)) {
        FATAL("Platform subsystem failed to start!");
        return false;
    }

    // Initialize Renderer
    if (!renderer_initialize(game_inst->config.name, &app_state->platform)) {
        FATAL("Renderer failed to initialize!");
        return false;
    }

    // Initialize Game Instance
    if (!app_state->game_inst->initialize(app_state->game_inst)) {
        FATAL("Game failed to initialize!");
        return false;
    }

    app_state->game_inst->on_resize(app_state->game_inst, app_state->width, app_state->height);

    return true;
}

b8 application_run() {
    app_state->is_running = true;
    clock_start(&app_state->clock);
    clock_update(&app_state->clock);
    app_state->last_time = app_state->clock.elapsed;
    f64 running_time = 0;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60;

    INFO(get_memory_usage_str());
    while(app_state->is_running) {
        if(!platform_pump_messages(&app_state->platform)) {
            app_state->is_running = false;
        }
        if(!app_state->is_suspended) {

            clock_update(&app_state->clock);
            f64 current_time = app_state->clock.elapsed;
            f64 delta_time = current_time - app_state->last_time;
            f64 frame_start_time = platform_get_absolute_time();

            if (!app_state->game_inst->update(app_state->game_inst, (f32)delta_time)) {
                FATAL("Game update failed, exiting..");
                app_state->is_running = false;
                break;
            }

            if (!app_state->game_inst->render(app_state->game_inst, (f32)delta_time)) {
                FATAL("Game render failed, exiting..");
                app_state->is_running = false;
                break;
            }

            render_packet packet;
            packet.delta_time = (f32)delta_time;
            renderer_draw_frame(&packet);

            f64 frame_end_time = platform_get_absolute_time();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            running_time += frame_elapsed_time;
            f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

            if (remaining_seconds > 0) {
                u64 remaining_milliseconds = (u64)(remaining_seconds * 1000);
                b8 limit_frames = false;
                if (remaining_milliseconds > 0 && limit_frames) {
                    platform_sleep(remaining_milliseconds - 1);
                }
                frame_count++;
            }

            input_update(delta_time);

            app_state->last_time = current_time;
        }
    }

    app_state->is_running = false;
    
    // Technically obsolete, but serves as a demonstration on how it works
    event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);

    event_shutdown();
    input_shutdown();

    renderer_shutdown();

    platform_shutdown(&app_state->platform);
    shutdown_memory(&app_state->memory_system_state);
    return true;
}

void application_get_framebuffer_size(u32* width, u32* height) {
    *width = app_state->width;
    *height = app_state->height;
}

b8 application_on_event(u16 code, void* sender, void* listener, event_context context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            INFO("Event: Application Quit");
            app_state->is_running = false;
            return true;
        }
    }
    return false;
}

b8 application_on_key(u16 code, void* sender, void* listener, event_context context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 key_code = context.data.u16[0];
        
        if (key_code == KEY_ESCAPE) {
            event_context data = {};
            event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
            return true;
        }
        // Handle key press
    } else if (code == EVENT_CODE_KEY_RELEASED) {
        // u16 key_code = context.data.u16[0];
        // Handle key release
    }
    return false;
}


b8 application_on_resized(u16 code, void* sender, void* listener_inst, event_context context) {
    if (code == EVENT_CODE_RESIZED) {
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        // Check if different. If so, trigger a resize event.
        if (width != app_state->width || height != app_state->height) {
            app_state->width = width;
            app_state->height = height;

            DEBUG("Window resize: %i, %i", width, height);

            // Handle minimization
            if (width == 0 || height == 0) {
                INFO("Window minimized, suspending application.");
                app_state->is_suspended = true;
                return true;
            } else {
                if (app_state->is_suspended) {
                    INFO("Window restored, resuming application.");
                    app_state->is_suspended = false;
                }
                app_state->game_inst->on_resize(app_state->game_inst, width, height);
                renderer_on_resized(width, height);
            }
        }
    }

    // Event purposely not handled to allow other listeners to get this.
    return false;
}