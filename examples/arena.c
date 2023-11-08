#include <string.h>

// Override default size of regions
#define COOL_ARENA_DEF_SIZE 16

#define COOL_ARENA_IMPL
#include "../src/arena.h"

int main(void) {
    char *mem0 = NULL;
    char *mem1 = NULL;
    char msg0[] = "Hello, world!";
    char msg1[] = "Yet another hello!";
    Arena arena;
    Arena *arena_ptr = &arena;

    // Initialize an arena
    Arena_init(&arena);

    // Use the memory (reserving an extra byte for null)
    mem0 = (char *) Arena_alloc(&arena, strlen(msg0) + 1);
    for (size_t i = 0; i < strlen(msg0); i++) {
        mem0[i] = msg0[i];
    }
    mem0[strlen(msg0)] = '\0';

    // Use the memory
    mem1 = (char *) Arena_alloc(&arena, 32 * 4);
    for (size_t i = 0; i < strlen(msg1); i++) {
        mem1[i] = msg1[i];
    }
    mem1[strlen(msg1)] = '\0';

    // Dump arenas
    while (arena_ptr != NULL) {
        puts("");
        Arena_dump(arena_ptr);
        arena_ptr = arena_ptr->_next;
    }

    // Show the string
    puts("");
    puts(mem0);

    // Show the other string
    puts("");
    puts(mem1);

    // Free the arena
    Arena_free(&arena);
}
