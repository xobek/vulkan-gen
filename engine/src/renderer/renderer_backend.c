#include "renderer/renderer_backend.h"
#include "vulkan/vulkan_backend.h"

b8 renderer_backend_create(renderer_backend_type type, struct platform_state* plat_state, renderer_backend* backend) {
    backend->plat_state = plat_state;

    if (type == RENDERER_BACKEND_TYPE_VULKAN) {
        backend->initialize = vulkan_renderer_backend_initialize;
        backend->shutdown = vulkan_renderer_backend_shutdown;
        backend->resized = vulkan_renderer_backend_on_resized;
        backend->begin_frame = vulkan_renderer_backend_begin_frame;
        backend->end_frame = vulkan_renderer_backend_end_frame;

        return TRUE;
    }

    return FALSE;
}

void renderer_backend_destroy(renderer_backend* backend) {
    backend->initialize = 0;
    backend->shutdown = 0;
    backend->resized = 0;
    backend->begin_frame = 0;
    backend->end_frame = 0;
}