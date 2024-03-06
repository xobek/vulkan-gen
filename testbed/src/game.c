#include "game.h"

#include <core/logger.h>
#include <core/vmemory.h>
#include <core/input.h>

// temporary
#include <renderer/renderer_frontend.h>

#include <math/math_types.h>

void recalculate_view_matrix(game_state* state) {
    if (state->camera_view_dirty) {
        mat4 rotation = mat4_euler_xyz(state->camera_euler.x, state->camera_euler.y, state->camera_euler.z);
        mat4 translation = mat4_translation(state->camera_position);
        
        state->view = mat4_mul(rotation, translation);
        state->view = mat4_inverse(state->view);

        state->camera_view_dirty = false;
    }
}

void camera_yaw(game_state* state, f32 amount) {
    state->camera_euler.y += amount;
    state->camera_view_dirty = true;
}

void camera_pitch(game_state* state, f32 amount) {
    state->camera_euler.x += amount;

    f32 lim = deg_to_rad(89.0f);
    state->camera_euler.x = VCLAMP(state->camera_euler.x, -lim, lim);

    state->camera_view_dirty = true;
}

void camera_roll(game_state* state, f32 amount) {
    state->camera_euler.z += amount;
    state->camera_view_dirty = true;
}



b8 game_initialize(game* game_inst) {
    game_state* state = (game_state*)game_inst->state;

    state->camera_position = (vec3){0, 0, 30.0f};
    state->camera_euler = vec3_zero();

    state->view = mat4_translation(state->camera_position);
    state->view = mat4_inverse(state->view);
    state->camera_view_dirty = true;

    DEBUG("Game initialized!");
    return true;
}

b8 game_update(game* game_inst, f32 delta_time) {
    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = get_memory_alloc_count();
    if (input_key_up('M') && input_was_key_down('M')) {
        DEBUG("Allocations: %llu (%llu this frame)", alloc_count, alloc_count - prev_alloc_count);
    }
    game_state* state = (game_state*)game_inst->state;

    if (input_key_down(KEY_LEFT)) {
        camera_yaw(state, 1.0f * delta_time);
    }

    if (input_key_down(KEY_RIGHT)) {
        camera_yaw(state, -1.0f * delta_time);
    }

    if (input_key_down(KEY_UP)) {
        camera_pitch(state, 1.0f * delta_time);
    }

    if (input_key_down(KEY_DOWN)) {
        camera_pitch(state, -1.0f * delta_time);
    }

    f32 move_speed = 50.0f;
    vec3 velocity = vec3_zero();

    if (input_key_down('W')) {
        vec3 forward = mat4_forward(state->view);
        velocity = vec3_add(velocity, forward);
    }

    if (input_key_down('S')) {
        vec3 backward = mat4_backward(state->view);
        velocity = vec3_add(velocity, backward);
    }

    if (input_key_down('A')) {
        vec3 left = mat4_left(state->view);
        velocity = vec3_add(velocity, left);
    }

    if (input_key_down('D')) {
        vec3 right = mat4_right(state->view);
        velocity = vec3_add(velocity, right);
    }

    vec3 z = vec3_zero();
    if (!vec3_compare(z, velocity, 0.0002f)) {
        vec3_normalize(&velocity);
        state->camera_position.x += velocity.x * move_speed * delta_time;
        state->camera_position.y += velocity.y * move_speed * delta_time;
        state->camera_position.z += velocity.z * move_speed * delta_time;
        state->camera_view_dirty = true;
    }

    recalculate_view_matrix(state);

    renderer_set_view(state->view); // temporary

    return true;
}

b8 game_render(game* game_inst,  f32 delta_time) {
    return true;
}


void game_on_resize(game* game_inst, u32 width, u32 height) {
}
