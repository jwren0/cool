#ifndef _COOL_ARENA_H
#define _COOL_ARENA_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

typedef struct Arena {
    uintptr_t *_region;
    uintptr_t _size;
    uintptr_t _alloc_size;
    struct Arena *_next;
} Arena;

/**
 * Initializes an `Arena` struct.
 *
 * For example:
 * ```
 * Arena arena;
 * Arena_init(&arena);
 *
 * // ----
 * ```
 *
 * @param arena The arena to initialize.
 */
void Arena_init(Arena *arena);

/**
 * Allocates a given quantity of memory
 * within the arena.
 *
 * If there is a region with enough free memory,
 * then that region will be used for allocation.
 *
 * If no regions have enough memory available and the
 * memory being allocated is less than `COOL_ARENA_DEF_SIZE`,
 * then a new region will be allocated of size `COOL_ARENA_DEF_SIZE`.
 *
 * If no regions have enough memory available and the
 * memory being allocated is greater than `COOL_ARENA_DEF_SIZE`,
 * then a region will be allocated. In this case, however,
 * the allocator will start at size `COOL_ARENA_DEF_SIZE` and keep
 * doubling this size, until a value which is larger than the requested
 * size `size` is reached. A new region will then be allocated with this size.
 *
 * For example:
 * ```
 * Arena arena;
 *
 * // ----
 *
 * char *mem = (char *) Arena_alloc(&arena, 1024);
 *
 * // Check for failure
 * if (mem == NULL) {
 *     perror("malloc");
 * }
 *
 * // ----
 * ```
 *
 * @param arena The arena to allocate memory in.
 * @param size The quantity of bytes to allocate.
 * @return A pointer on success, NULL otherwise.
 */
uintptr_t *Arena_alloc(Arena *arena, uintptr_t size);

/**
 * Resets the arena.
 *
 * All currently allocated regions will remain, but
 * they will be reset, so that memory can be allocated within
 * them once more.
 *
 * This does not reset the memory in the regions.
 *
 * For example:
 * ```
 * Arena arena;
 *
 * // ----
 *
 * Arena_reset(&arena);
 *
 * // ----
 * ```
 *
 * @param arena The arena to reset.
 */
void Arena_reset(Arena *arena);

/**
 * Frees the arena.
 *
 * Each region will individually be freed and
 * the root of the arena will be reinitialized.
 *
 * For example:
 * ```
 * Arena arena;
 *
 * // ----
 *
 * Arena_free(&arena);
 *
 * ```
 */
void Arena_free(Arena *arena);

/**
 * Dumps the arena to stdout.
 *
 * Quite useful for debugging.
 *
 * For example:
 * ```
 * Arena arena;
 *
 * // ----
 *
 * Arena_dump(&arena);
 *
 * // ----
 *
 * ```
 */
void Arena_dump(Arena *arena);

#ifdef COOL_ARENA_IMPL

/**
 * The default quantity of bytes to allocate
 * to each region in the arena.
 */
#ifndef COOL_ARENA_DEF_SIZE
#define COOL_ARENA_DEF_SIZE 8 * 1024
#endif

/**
 * The underlying function for allocating
 * memory for Arena structs and regions.
 *
 * This function can be changed, but it must have
 * the same function signature as `malloc(3)`.
 *
 * This function must also return a valid pointer
 * on success, and NULL on failure.
 */
#ifndef COOL_ARENA_FUNC_ALLOC
#include <stdlib.h>
#define COOL_ARENA_FUNC_ALLOC malloc
#endif

/**
 * The underlying function for freeing
 * memory for a region.
 *
 * This function can be changed, but it must have
 * the same function signature as `free(3)`
 */
#ifndef COOL_ARENA_FUNC_FREE
#include <stdlib.h>
#define COOL_ARENA_FUNC_FREE free
#endif

void Arena_init(Arena *arena) {
    arena->_region = NULL;
    arena->_size = 0;
    arena->_alloc_size = 0;
    arena->_next = NULL;
}

uintptr_t *Arena_alloc(Arena *arena, uintptr_t size) {
    uintptr_t new_capacity;
    uintptr_t *mem = NULL;
    uintptr_t *new_region;
    Arena *new_arena;

    // If size is 0, do nothing
    if (size == 0) return NULL;

    // Switch size to accomodate bytes
    size = (size >> 2) + 1;

    for (;;) {
        // Stop if there is no next value, or if
        // this region has enough free memory
        if (arena->_next == NULL
            || size <= arena->_alloc_size - arena->_size) {
            break;
        }

        // Otherwise, use the next region
        arena = arena->_next;
    }

    // If this region has enough free memory, use it
    if (size <= arena->_alloc_size - arena->_size) {
        mem = arena->_region + arena->_size;
        arena->_size += size;
        return mem;
    }

    // Otherwise, this means this region doesn't have enough
    // free memory, and is the end of the arena (no next region).
    // So, a new region must be allocated.

    // Calculate how much to allocate
    new_capacity = COOL_ARENA_DEF_SIZE;

    while (new_capacity < size) {
        // Check for overflows
        if (new_capacity > new_capacity << 1) {
            return NULL;
        }

        new_capacity <<= 1;
    }

    // Try allocating a region for the arena
    new_region = (uintptr_t *) COOL_ARENA_FUNC_ALLOC(new_capacity);
    if (new_region == NULL) return NULL;

    // If the region is not blank, create a new arena
    // and use it
    if (arena->_region != NULL) {
        new_arena = (Arena *) COOL_ARENA_FUNC_ALLOC(sizeof(Arena));

        if (new_arena == NULL) {
            free(new_region);
            return NULL;
        }

        Arena_init(new_arena);

        arena->_next = new_arena;
        arena = new_arena;
    }

    // Update this arena
    arena->_region = new_region;
    arena->_alloc_size = new_capacity;
    arena->_size += size;

    return arena->_region;
}

void Arena_reset(Arena *arena) {
    // Iterate over all regions and reset size
    while (arena != NULL) {
        arena->_size = 0;
        arena = arena->_next;
    }
}

void Arena_free(Arena *arena) {
    Arena *prev = NULL;

    // Iterate over all regions and free them
    while (arena != NULL) {
        // Free the region
        COOL_ARENA_FUNC_FREE(arena->_region);
        arena->_region = NULL;

        // Store the current arena
        prev = arena;

        // Move to the next, set the current arena's
        // next value
        arena = arena->_next;
        prev->_next = NULL;
    }
}

void Arena_dump(Arena *arena) {
    printf(
        "_region:     %p\n"
        "_size:       %lu\n"
        "_alloc_size: %lu\n"
        "_next:       %p\n"
        "== region contents ==\n",
        (void *) arena->_region, arena->_size,
        arena->_alloc_size, (void *) arena->_next
    );

    if (arena->_region != NULL) {
        for (uintptr_t i = 0; i < arena->_alloc_size; i++) {
            if ((i + 1) % 20 == 0) printf("%#lx\n", arena->_region[i]);
            else printf("%#lx ", arena->_region[i]);
        }
    } else {
        puts("Region is blank");
    }
    puts("");
}

#endif // COOL_ARENA_IMPL

#endif // _COOL_ARENA_H
