#include "vulkan_backend.h"

#include "vulkan_types.inl"
#include "vulkan_platform.h"

#include "core/logger.h"
#include "core/vstring.h"

#include "containers/darray.h"

#include "platform/platform.h"

static vulkan_context context;

b8 vulkan_renderer_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state) {
    context.allocator = 0;

    // Setup Vulkan instance.
    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_2;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "vRo Engine";

    VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;

    const char** extensions = darray_create(const char*);
    darray_push(extensions, &VK_KHR_SURFACE_EXTENSION_NAME);
    platform_get_required_extension_names(&extensions);

    #if defined(_DEBUG)
    darray_push(extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    DEBUG("Required Extensions:");
    u32 length = darray_length(extensions);
    for (u32 i = 0; i < length; i++) {
        DEBUG(" ->  %s", extensions[i]);
    }
    #endif

    create_info.enabledExtensionCount = darray_length(extensions);
    create_info.ppEnabledExtensionNames = extensions;

    const char** validation_layer_names = 0;
    u32 validation_layer_count = 0;

    #if defined(_DEBUG)
    INFO("Validation layers enabled. Enumerating...");

    // The list of validation layers required.
    validation_layer_names = darray_create(const char*);
    darray_push(validation_layer_names, &"VK_LAYER_KHRONOS_validation");
    validation_layer_count = darray_length(validation_layer_names);

    // Obtain a list of available validation layers
    u32 available_layer_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
    VkLayerProperties* available_layers = darray_reserve(VkLayerProperties, available_layer_count);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

    // Verify all required layers are available.
    for (u32 i = 0; i < validation_layer_count; ++i) {
        INFO("Searching for layer: %s...", validation_layer_names[i]);
        b8 found = FALSE;
        for (u32 j = 0; j < available_layer_count; ++j) {
            if (strings_equal(validation_layer_names[i], available_layers[j].layerName)) {
                found = TRUE;
                INFO("Found.");
                break;
            }
        }

        if (!found) {
            FATAL("Required validation layer is missing: %s", validation_layer_names[i]);
            return FALSE;
        }
    }
    INFO("All required validation layers are present.");
    #endif

    create_info.enabledLayerCount = validation_layer_count;
    create_info.ppEnabledLayerNames = validation_layer_names;

    VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));

    INFO("Vulkan renderer initialized successfully.");
    return TRUE;
}

void vulkan_renderer_backend_shutdown(renderer_backend* backend) {
}

void vulkan_renderer_backend_on_resized(renderer_backend* backend, u16 width, u16 height) {
}

b8 vulkan_renderer_backend_begin_frame(renderer_backend* backend, f32 delta_time) {
    return TRUE;
}

b8 vulkan_renderer_backend_end_frame(renderer_backend* backend, f32 delta_time) {
    return TRUE;
}