#include "vmemory.h"
#include "core/logger.h"
#include "platform/platform.h"
#include <stdio.h>

#include <string.h>

struct memory_stats {
    u64 total_allocated;
    u64 tagged_allocated[MEMORY_TAG_MAX_TAGS];
};

static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN            ",
    "ARRAY              ",
    "DARRAY             ",
    "DICT               ",
    "RING_QUEUE         ",
    "BST                ",
    "STRING             ",
    "APPLICATION        ",
    "JOB                ",
    "TEXTURE            ",
    "MATERIAL_INSTANCE  ",
    "RENDERER           ",
    "GAME               ",
    "TRANSFORM          ",
    "ENTITY             ",
    "ENTITY_NODE        ",
    "SCENE              "
};

static struct memory_stats stats;

void initialize_memory() {
    platform_zero_memory(&stats, sizeof(stats));
}

void shutdown_memory() {

}

void* vallocate(u64 size, memory_tag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        WARN("vallocate: unknown tag needs reclassification.");
    }

    stats.total_allocated += size;
    stats.tagged_allocated[tag] += size;

    void* block = platform_allocate(size, FALSE);
    platform_zero_memory(block, size);
    return block;
}

void vfree(void* block, u64 size, memory_tag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        WARN("vfree: unknown tag needs reclassification.");
    }

    stats.total_allocated -= size;
    stats.tagged_allocated[tag] -= size;

    platform_free(block, FALSE);
}

void* vzero_memory(void* block, u64 size) {
    return platform_zero_memory(block, size);
}

void* vcopy_memory(void* dest, const void* src, u64 size) {
    return platform_copy_memory(dest, src, size);
}

void* vset_memory(void* dest, i32 value, u64 size) {
    return platform_set_memory(dest, value, size);
}

char* get_memory_usage_str() {
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8192] = "System memory use:\n";
    u64 offset = strlen(buffer);

    for (u32 i=0; i<MEMORY_TAG_MAX_TAGS; i++) {
        char unit[4] = "xiB";
        float amount = 1.0f;
        if (stats.tagged_allocated[i] > gib) {
            unit[0] = 'G';
            amount = (float)stats.tagged_allocated[i] / (float)gib;
        } else if (stats.tagged_allocated[i] > mib) {
            unit[0] = 'M';
            amount = (float)stats.tagged_allocated[i] / (float)mib;
        } else if (stats.tagged_allocated[i] > kib) {
            unit[0] = 'K';
            amount = (float)stats.tagged_allocated[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)stats.tagged_allocated[i];
        }
        i32 length = snprintf(buffer + offset, 8192 - offset, "  %s: %.2f %s\n", memory_tag_strings[i], amount, unit);
        offset += length;
    }
    char* output = _strdup(buffer);
    return output;
}