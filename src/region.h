#ifndef _COOL_REGION_H
#define _COOL_REGION_H

/**
 * The default quantity of bytes to allocate
 * to regions when using `Region_init`.
 */
#ifndef COOL_REGION_DEF_SIZE
#define COOL_REGION_DEF_SIZE 512
#endif

/**
 * The underlying function for allocating
 * memory for a region.
 *
 * This function can be changed, but it must have
 * the same function signature as `malloc(3)`.
 *
 * This function must also return a valid pointer
 * on success, and NULL on failure.
 */
#ifndef COOL_REGION_FUNC_ALLOC
#include <stdlib.h>
#define COOL_REGION_FUNC_ALLOC malloc
#endif

/**
 * The underlying function for reallocating
 * memory for a region.
 *
 * This is currently only used for extending
 * the memory for a region.
 *
 * This function can be changed, but it must have
 * the same function signature as `realloc(3)`
 *
 * This function must also return a valid pointer
 * on success, and NULL on failure.
 */
#ifndef COOL_REGION_FUNC_REALLOC
#include <stdlib.h>
#define COOL_REGION_FUNC_REALLOC realloc
#endif

/**
 * The underlying function for freeing
 * memory for a region.
 *
 * This function can be changed, but it must have
 * the same function signature as `free(3)`
 */
#ifndef COOL_REGION_FUNC_FREE
#include <stdlib.h>
#define COOL_REGION_FUNC_FREE free
#endif

/**
 * Declares a region to hold a given type.
 *
 * For example:
 * ```
 * // Declare the type
 * RegionType(MyCharRegion, char);
 *
 * // Declare an instance of the type
 * MyCharRegion region;
 * ```
 *
 * @param N The name of the type.
 * @param T The type which this region should store.
 */
#define RegionType(N, T) typedef struct { \
    T *buf;                               \
    size_t _alloc_size;                   \
    size_t size;                          \
    int error;                            \
} N

/**
 * Initializes a region with a given size.
 *
 * If an error occurs during allocation, the `error`
 * field will be set to `1`.
 *
 * For example:
 * ```
 * RegionType(MyCharRegion, char);
 * MyCharRegion region;
 *
 * // Allocate a region to store 1024 chars
 * Region_init_sized(region, 1024 * sizeof(char));
 *
 * // Check for errors
 * if (region.error) {
 *     perror("malloc");
 * }
 * ```
 *
 * @param R The region to initialize.
 * @param S The quantity of bytes to be allocated
 *          to the region initially.
 */
#define Region_init_sized(R, S) {          \
    (R).size = 0;                          \
    (R)._alloc_size = (S);                 \
    (R).buf = COOL_REGION_FUNC_ALLOC((S)); \
    (R).error = ((R).buf == NULL) ? 1 : 0; \
}

/**
 * Initializes a region with the default
 * size, `COOL_REGION_DEF_SIZE`.
 *
 * If an error occurs during allocation, the `error`
 * field will be set to `1`.
 *
 * For example:
 * ```
 * RegionType(MyCharRegion, char);
 * MyCharRegion region;
 *
 * // Allocate a region to store chars
 * Region_init_sized(region);
 *
 * // Check for errors
 * if (region.error) {
 *     perror("malloc");
 * }
 * ```
 *
 * @param R The region to initialize.
 */
#define Region_init(R) Region_init_sized(R, COOL_REGION_DEF_SIZE)

/**
 * Frees the underlying allocated memory a region owns,
 * setting the `buf` field to NULL afterwards.
 *
 * For example:
 * ```
 * RegionType(MyCharRegion, char);
 * MyCharRegion region;
 *
 * // ----
 *
 * // Deallocate the underlying memory
 * Region_free(region);
 * ```
 *
 * @param R The region to free memory for.
 */
#define Region_free(R) {            \
    COOL_REGION_FUNC_FREE((R).buf); \
    (R).buf = NULL;                 \
}

/**
 * Pushes a value onto the end of the underlying buffer.
 *
 * If an error occurs during pushing, the `error`
 * field will be set to `1`.
 *
 * For example:
 * ```
 * RegionType(MyCharRegion, char);
 * MyCharRegion region;
 *
 * // ----
 *
 * // Push a value
 * Region_push(region, 'A');
 *
 * // Check for errors
 * if (region.error) {
 *     perror("realloc");
 * }
 *
 * // ----
 * ```
 *
 * @param R The region to append to.
 * @param V The value to append.
 */
#define Region_push(R, V) {                            \
    (R).error = 0;                                     \
    if ((R).size >= (R)._alloc_size - 1) {             \
        void *_region_temp = COOL_REGION_FUNC_REALLOC( \
            (R).buf, (R)._alloc_size << 1              \
        );                                             \
        if (_region_temp == NULL) {                    \
            (R).error = 1;                             \
        } else {                                       \
            (R).buf = _region_temp;                    \
            (R)._alloc_size <<= 1;                     \
            (R).buf[(R).size++] = V;                   \
        }                                              \
    } else {                                           \
        (R).buf[(R).size++] = V;                       \
    }                                                  \
}

/**
 * "Pops" a value from the end of the region.
 *
 * This does not check if there is even a value to pop.
 * You must check manually.
 *
 * Furthermore, the value is not removed from memory,
 * see `Region_drop` if you need this behavior.
 *
 * For example:
 * ```
 * RegionType(MyCharRegion, char);
 * MyCharRegion region;
 *
 * // ----
 *
 * // Check there are values to pop, then pop
 * if (region.size > 0) {
 *     char c = Region_pop(region);
 * }
 *
 * // ----
 * ```
 *
 * @param R The region to "pop" from.
 * @return The value which was "popped"
 */
#define Region_pop(R) (R).buf[--(R).size]

/**
 * Drops a value from the end of the region.
 * This will set the last stored value in memory to 0.
 *
 * This does not check if there is even a value which
 * can be dropped.
 * You must check manually.
 *
 * For example:
 * ```
 * RegionType(MyCharRegion, char);
 * MyCharRegion region;
 *
 * // ----
 *
 * // Check there are values to drop
 * if (region.size > 0) {
 *     // Store the char which is about to be dropped
 *     char c = region.buf[region.size - 1];
 *
 *     // Drop the value
 *     Region_drop(region);
 * }
 *
 * // ----
 * ```
 *
 * @param R The region to drop from.
 */
#define Region_drop(R) (R).buf[--(R).size] = 0

/**
 * Dumps useful information about the supplied
 * region to stdout.
 *
 * Quite useful for debugging.
 *
 * For example:
 * ```
 * RegionType(MyCharRegion, char);
 * MyCharRegion region;
 *
 * // ----
 *
 * // Dump the region
 * Region_dump(region);
 *
 * // ----
 * ```
 *
 * @param R The region to dump to stdout.
 */
#define Region_dump(R) {                                        \
    printf(                                                     \
        "buf:         %p\n"                                     \
        "error:       %d\n"                                     \
        "size:        %lu\n"                                    \
        "_alloc_size: %lu\n"                                    \
        "== buf contents ==\n",                                 \
        (R).buf, (R).error,                                     \
        (R).size, (R)._alloc_size                               \
    );                                                          \
    if ((R).buf != NULL) {                                      \
        for (size_t i = 0; i < (R)._alloc_size; i++) {          \
            if ((i + 1) % 20 == 0) printf("%#x\n", (R).buf[i]); \
            else printf("%#x ", (R).buf[i]);                    \
        }                                                       \
        puts("");                                               \
    } else {                                                    \
        puts("Buf is blank\n");                                 \
    }                                                           \
}

#endif // _COOL_REGION_H
