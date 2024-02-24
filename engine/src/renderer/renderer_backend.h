#pragma once

#include "renderer/renderer_types.inl"

struct platform_state;

b8 renderer_backend_create(renderer_backend_type type, struct platform_state* plat_state, renderer_backend* backend);
void renderer_backend_destroy(renderer_backend* backend);