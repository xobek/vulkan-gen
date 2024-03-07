#include "renderer/renderer_backend.h"
#include "vulkan/vulkan_backend.h"

b8 renderer_backend_create(renderer_backend_type type, renderer_backend* backend) {

    if (type == RENDERER_BACKEND_TYPE_VULKAN) {
        backend->initialize = vulkan_renderer_backend_initialize;
        backend->shutdown = vulkan_renderer_backend_shutdown;
        backend->resized = vulkan_renderer_backend_on_resized;
        backend->begin_frame = vulkan_renderer_backend_begin_frame;
        backend->update_global_state = vulkan_renderer_update_global_state;
        backend->end_frame = vulkan_renderer_backend_end_frame;
        backend->update_object = vulkan_backend_update_object;
        backend->create_texture = vulkan_renderer_create_texture;
        backend->destroy_texture = vulkan_renderer_destroy_texture;
        return true;
    }

    return false;
}

void renderer_backend_destroy(renderer_backend* backend) {
    backend->initialize = 0;
    backend->shutdown = 0;
    backend->resized = 0;
    backend->begin_frame = 0;
    backend->update_global_state = 0;
    backend->end_frame = 0;
    backend->update_object = 0;
    backend->create_texture = 0;
    backend->destroy_texture = 0;
}