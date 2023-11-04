#include <stdio.h>
#include <string.h>

// Override default size
#define COOL_REGION_DEF_SIZE 8
#include "../src/region.h"

int main(void) {
    char msg[] = "Hello, world!";

    // Declare what type the CharRegion struct
    // should store
    RegionType(CharRegion, char);

    // Initialize a CharRegion struct
    CharRegion region;
    Region_init(region);

    if (region.error != 0) {
        perror("malloc");
        return 1;
    }

    // Display the region
    Region_dump(region);

    // Use the region for storing
    // some data
    for (size_t i = 0; i < strlen(msg); i++) {
        Region_push(region, msg[i]);

        // Check for errors
        if (region.error != 0) {
            perror("realloc");
            goto cleanup;
        }
    }

    // Display the region again, after leaving a gap
    puts("");
    Region_dump(region);

    // Print the buffer
    puts("");
    puts(region.buf);

    // Pop values off the region
    puts("");
    while (region.size > 0) {
        printf("Popped: %c\n", Region_pop(region));
    }

cleanup:
    // Free the region
    Region_free(region);
}
