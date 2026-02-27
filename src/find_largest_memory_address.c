#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define PAGE_SIZE 4096

typedef struct {
    uintptr_t start;
    uintptr_t end;
} MemoryRange;

MemoryRange* get_memory_ranges(size_t *count) {
    FILE *file = fopen("/proc/self/maps", "r");
    if (!file) {
        perror("fopen");
        return NULL;
    }

    char line[256];
    MemoryRange *ranges = NULL;
    size_t range_count = 0;

    while (fgets(line, sizeof(line), file)) {
        uintptr_t start, end;
        sscanf(line, "%lx-%lx", &start, &end);

        ranges = realloc(ranges, (range_count + 1) * sizeof(MemoryRange));
        ranges[range_count].start = start;
        ranges[range_count].end = end;
        range_count++;
    }

    fclose(file);
    *count = range_count;
    return ranges;
}

uintptr_t find_largest_free_area(uint64_t *largest_free_size) {
    size_t count;
    MemoryRange *ranges = get_memory_ranges(&count);
    if (!ranges) return 0;

    uintptr_t largest_free_start = 0;

    for (size_t i = 1; i < count; i++) {
        uintptr_t end_of_prev = ranges[i - 1].end;
        uintptr_t start_of_curr = ranges[i].start;

        if (start_of_curr > end_of_prev) {
            uintptr_t free_size = start_of_curr - end_of_prev;
            if (free_size > *largest_free_size) {
                *largest_free_size = free_size;
                largest_free_start = end_of_prev;
            }
        }
    }

    free(ranges);
    return largest_free_start;
}
