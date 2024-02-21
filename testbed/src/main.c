#include <core/logger.h>
#include <core/asserts.h>

#include <platform/platform.h>

int main(void) {

    ERROR("This is an error message");
    WARN("This is a warning message");
    platform_state state;

    if (platform_startup(&state, "Test Window", 100, 100, 800, 600)) {
        while(1) {
            platform_pump_messages(&state);
        }
    }

    platform_shutdown(&state);

    return 0;
}