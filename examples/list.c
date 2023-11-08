#include <stdio.h>
#include <string.h>

// Override default size
#define COOL_LIST_DEF_SIZE 8
#include "../src/list.h"

int main(void) {
    char msg[] = "Hello, world!";

    // Declare what type the CharList struct
    // should store
    ListType(CharList, char);

    // Initialize a CharList struct
    CharList list;
    List_init(list);

    if (list.error != 0) {
        perror("malloc");
        return 1;
    }

    // Display the list
    List_dump(list);

    // Use the list for storing
    // some data
    for (size_t i = 0; i < strlen(msg); i++) {
        List_push(list, msg[i]);

        // Check for errors
        if (list.error != 0) {
            perror("realloc");
            goto cleanup;
        }
    }

    // Ensure null termination
    List_push(list, '\0');

    // Display the list again, after leaving a gap
    puts("");
    List_dump(list);

    // Print the buffer
    puts("");
    puts(list.buf);

    // Pop values off the list
    puts("");
    while (list.size > 0) {
        printf("Popped: %c\n", List_pop(list));
    }

cleanup:
    // Free the list
    List_free(list);
}
