#include "renderer/renderer_frontend.h"
#include "renderer/renderer_backend.h"

#include "core/logger.h"
#include "core/vmemory.h"
#include "math/vmath.h"

static renderer_backend* backend = 0;

typedef struct renderer_system_state {
    renderer_backend backend;
} renderer_system_state;

static renderer_system_state* state_ptr;

b8 renderer_system_initialize(u64* memory_requirement, void* state, const char* application_name) {
    *memory_requirement = sizeof(renderer_system_state);
    if (state == 0) {
        return true;
    }
    state_ptr = state;

    backend = vallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);

    renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, &state_ptr->backend);
    state_ptr->backend.frame_number = 0;

    if (!state_ptr->backend.initialize(&state_ptr->backend, application_name)) {
        FATAL("Renderer failed to initialize!");
        return false;
    }
    return true;
}

void renderer_system_shutdown(void* state) {
    if (state_ptr) {
        state_ptr->backend.shutdown(&state_ptr->backend);
    }
    state_ptr = 0;
}

void renderer_resized(u16 width, u16 height);

b8 renderer_begin_frame(f32 delta_time) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->backend.begin_frame(&state_ptr->backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
    if (!state_ptr) {
        return false;
    }
    b8 result = state_ptr->backend.end_frame(&state_ptr->backend, delta_time);
    state_ptr->backend.frame_number++;
    return result;
}

b8 renderer_draw_frame(render_packet* packet) {
    if (renderer_begin_frame(packet->delta_time)) {
        
        mat4 projection = mat4_perspective(deg_to_rad(45.0f), 1280 / 720.0f, 0.1f, 1000.0f);
        static f32 z = -1.0f;
        z -= 0.005f;
        mat4 view = mat4_translation((vec3){0, 0, z});

        state_ptr->backend.update_global_state(projection, view, vec3_zero(), vec4_one(), 0);
        
        b8 result = renderer_end_frame(packet->delta_time);

        if (!result) {
            ERROR("Failed to draw frame!");
            return false;
        }
    }
    return true;
}

void renderer_on_resized(u16 width, u16 height) {
    if (state_ptr) {
        state_ptr->backend.resized(&state_ptr->backend, width, height);
    }
}