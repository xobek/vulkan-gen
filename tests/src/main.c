#include "test_manager.h"

#include "memory/linear_allocator_tests.h"
#include <core/logger.h>

int main() {
    test_manager_init();

    linear_allocator_register_tests();

    DEBUG("=> Starting tests...");

    test_manager_run_tests();
    return 0;
}