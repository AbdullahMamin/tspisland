#include "str.h"

size g_capacity;
size g_next_location;
static char *g_arena;

bool StrInit(size n_bytes) {
    g_arena = calloc(n_bytes, 1);
    if (!g_arena) {
        return false;
    }
    g_capacity = n_bytes;
    g_next_location = 0;
    return true;
}

void StrDeinit(void) {
    free(g_arena);
    g_arena = NULL;
}

char *Str(size n_bytes) {
    n_bytes++; // for '\0'
    assert(g_arena && n_bytes > 0);
    size new_next = g_next_location + n_bytes;
    if (new_next > g_capacity) {
        return NULL;
    }
    char *str = g_arena + g_next_location;
    g_next_location = new_next;
    return str;
}

char *StrConcatenate(int n_strs, ...) {
    assert(n_strs > 0);
    size length = 0;

    va_list list;
    va_start(list, n_strs);
    for (int i = 0; i < n_strs; i++) {
        length += strlen(va_arg(list, const char *));
    }
    va_end(list);

    char *str = Str(length);
    if (!str) {
        return NULL;
    }

    va_start(list, n_strs);
    strcpy(str, va_arg(list, const char *));
    for (int i = 1; i < n_strs; i++) {
        strcat(str, va_arg(list, const char *));
    }
    va_end(list);

    return str;
}
