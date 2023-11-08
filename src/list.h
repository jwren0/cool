#ifndef _COOL_LIST_H
#define _COOL_LIST_H

/**
 * The default quantity of bytes to allocate
 * to lists when using `List_init`.
 */
#ifndef COOL_LIST_DEF_SIZE
#define COOL_LIST_DEF_SIZE 512
#endif

/**
 * The underlying function for allocating
 * memory for a list.
 *
 * This function can be changed, but it must have
 * the same function signature as `malloc(3)`.
 *
 * This function must also return a valid pointer
 * on success, and NULL on failure.
 */
#ifndef COOL_LIST_FUNC_ALLOC
#include <stdlib.h>
#define COOL_LIST_FUNC_ALLOC malloc
#endif

/**
 * The underlying function for reallocating
 * memory for a list.
 *
 * This is currently only used for extending
 * the memory for a list.
 *
 * This function can be changed, but it must have
 * the same function signature as `realloc(3)`
 *
 * This function must also return a valid pointer
 * on success, and NULL on failure.
 */
#ifndef COOL_LIST_FUNC_REALLOC
#include <stdlib.h>
#define COOL_LIST_FUNC_REALLOC realloc
#endif

/**
 * The underlying function for freeing
 * memory for a list.
 *
 * This function can be changed, but it must have
 * the same function signature as `free(3)`
 */
#ifndef COOL_LIST_FUNC_FREE
#include <stdlib.h>
#define COOL_LIST_FUNC_FREE free
#endif

/**
 * Declares a list to hold a given type.
 *
 * For example:
 * ```
 * // Declare the type
 * ListType(MyCharList, char);
 *
 * // Declare an instance of the type
 * MyCharList list;
 * ```
 *
 * @param N The name of the type.
 * @param T The type which this list should store.
 */
#define ListType(N, T) typedef struct { \
    T *buf;                             \
    size_t _alloc_size;                 \
    size_t size;                        \
    int error;                          \
} N

/**
 * Initializes a list with a given size.
 *
 * If an error occurs during allocation, the `error`
 * field will be set to `1`.
 *
 * For example:
 * ```
 * ListType(MyCharList, char);
 * MyCharList list;
 *
 * // Allocate a list to store 1024 chars
 * List_init_sized(list, 1024 * sizeof(char));
 *
 * // Check for errors
 * if (list.error) {
 *     perror("malloc");
 * }
 * ```
 *
 * @param R The list to initialize.
 * @param S The quantity of bytes to be allocated
 *          to the list initially.
 */
#define List_init_sized(R, S) {            \
    (R).size = 0;                          \
    (R)._alloc_size = (S);                 \
    (R).buf = COOL_LIST_FUNC_ALLOC((S));   \
    (R).error = ((R).buf == NULL) ? 1 : 0; \
}

/**
 * Initializes a list with the default
 * size, `COOL_LIST_DEF_SIZE`.
 *
 * If an error occurs during allocation, the `error`
 * field will be set to `1`.
 *
 * For example:
 * ```
 * ListType(MyCharList, char);
 * MyCharList list;
 *
 * // Allocate a list to store chars
 * List_init_sized(list);
 *
 * // Check for errors
 * if (list.error) {
 *     perror("malloc");
 * }
 * ```
 *
 * @param R The list to initialize.
 */
#define List_init(R) List_init_sized(R, COOL_LIST_DEF_SIZE)

/**
 * Frees the underlying allocated memory a list owns,
 * setting the `buf` field to NULL afterwards.
 *
 * For example:
 * ```
 * ListType(MyCharList, char);
 * MyCharList list;
 *
 * // ----
 *
 * // Deallocate the underlying memory
 * List_free(list);
 * ```
 *
 * @param R The list to free memory for.
 */
#define List_free(R) {            \
    COOL_LIST_FUNC_FREE((R).buf); \
    (R).buf = NULL;               \
}

/**
 * Pushes a value onto the end of the list.
 *
 * If an error occurs during pushing, the `error`
 * field will be set to `1`.
 *
 * For example:
 * ```
 * ListType(MyCharList, char);
 * MyCharList list;
 *
 * // ----
 *
 * // Push a value
 * List_push(list, 'A');
 *
 * // Check for errors
 * if (list.error) {
 *     perror("realloc");
 * }
 *
 * // ----
 * ```
 *
 * @param R The list to append to.
 * @param V The value to append.
 */
#define List_push(R, V) {                          \
    (R).error = 0;                                 \
    if ((R).size >= (R)._alloc_size - 1) {         \
        void *_list_temp = COOL_LIST_FUNC_REALLOC( \
            (R).buf, (R)._alloc_size << 1          \
        );                                         \
        if (_list_temp == NULL) {                  \
            (R).error = 1;                         \
        } else {                                   \
            (R).buf = _list_temp;                  \
            (R)._alloc_size <<= 1;                 \
            (R).buf[(R).size++] = V;               \
        }                                          \
    } else {                                       \
        (R).buf[(R).size++] = V;                   \
    }                                              \
}

/**
 * "Pops" a value from the end of the list.
 *
 * This does not check if there is even a value to pop.
 * You must check manually.
 *
 * Furthermore, the value is not removed from memory,
 * see `List_drop` if you need this behavior.
 *
 * For example:
 * ```
 * ListType(MyCharList, char);
 * MyCharList list;
 *
 * // ----
 *
 * // Check there are values to pop, then pop
 * if (list.size > 0) {
 *     char c = List_pop(list);
 * }
 *
 * // ----
 * ```
 *
 * @param R The list to "pop" from.
 * @return The value which was "popped"
 */
#define List_pop(R) (R).buf[--(R).size]

/**
 * Drops a value from the end of the list.
 * This will set the last stored value in the list to 0,
 * and then decrease the size of the list.
 *
 * This does not check if there is even a value which
 * can be dropped.
 * You must check manually.
 *
 * For example:
 * ```
 * ListType(MyCharList, char);
 * MyCharList list;
 *
 * // ----
 *
 * // Check there are values to drop
 * if (list.size > 0) {
 *     // Store the char which is about to be dropped
 *     char c = list.buf[list.size - 1];
 *
 *     // Drop the value
 *     List_drop(list);
 * }
 *
 * // ----
 * ```
 *
 * @param R The list to drop from.
 */
#define List_drop(R) (R).buf[--(R).size] = 0

/**
 * Dumps useful information about the supplied
 * list to stdout.
 *
 * Quite useful for debugging.
 *
 * For example:
 * ```
 * ListType(MyCharList, char);
 * MyCharList list;
 *
 * // ----
 *
 * // Dump the list
 * List_dump(list);
 *
 * // ----
 * ```
 *
 * @param R The list to dump to stdout.
 */
#define List_dump(R) {                                          \
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
        puts("Buf is blank");                                   \
    }                                                           \
}

#endif // _COOL_LIST_H
