#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "intern.h"
#include "block.h"
#include "unsigned.h"

int main() {
    struct strings *strings = strings_new();
    assert(strings);

    char buffer[12] = {'x'};
    const char *string;
    uint32_t id;

    unsigned count = 1000000;

    // test interning strings
    for (unsigned i = 1; i <= count; i++) {
        unsigned_string(buffer + 1, i);
        assert(strings_intern(strings, buffer, &id));
        assert(id == i);
    }
    assert(strings_count(strings) == count);

    // test idempotency
    for (unsigned i = 1; i <= count; i++) {
        unsigned_string(buffer + 1, i);
        assert(strings_intern(strings, buffer, &id));
        assert(id == i);
    }
    assert(strings_count(strings) == count);

    // test looking up ids
    for (unsigned i = 1; i <= count; i++) {
        unsigned_string(buffer + 1, i);
        string = strings_lookup_id(strings, i);
        assert(string && !strcmp(buffer, string));
    }

    // test looking up strings
    for (unsigned i = 1; i <= count; i++) {
        unsigned_string(buffer + 1, i);
        assert(strings_lookup(strings, buffer) == i);
    }

    // test iterating strings
    struct strings_cursor cursor;
    strings_cursor_init(&cursor, strings);

    assert(!strings_cursor_id(&cursor));
    assert(!strings_cursor_string(&cursor));

    id = 0;
    while (strings_cursor_next(&cursor)) {
        id = strings_cursor_id(&cursor);
        string = strings_cursor_string(&cursor);
        assert(id && string);
        unsigned_string(buffer + 1, id);
        assert(!strcmp(buffer, string));
    }
    assert(id == count);

    assert(!strings_cursor_id(&cursor));
    assert(!strings_cursor_string(&cursor));

    // test optimizing strings
    struct strings *optimized;
    struct strings_frequency *freq = strings_frequency_new();
    assert(freq);

    assert(strings_frequency_add(freq, 3));
    assert(strings_frequency_add(freq, 3));
    assert(strings_frequency_add(freq, 3));
    assert(strings_frequency_add(freq, 2));
    assert(strings_frequency_add(freq, 2));
    assert(strings_frequency_add(freq, 5));

    optimized = strings_optimize(strings, freq);
    assert(optimized);
    assert(strings_count(optimized) == 3);

    assert(strings_lookup(optimized, "x3") == 1);
    assert(strings_lookup(optimized, "x2") == 2);
    assert(strings_lookup(optimized, "x5") == 3);

    strings_frequency_free(freq);
    strings_free(optimized);

    // test optimizing strings, and making sure all are included
    freq = strings_frequency_new();
    assert(freq);
    assert(strings_frequency_add_all(freq, strings));
    assert(strings_frequency_add(freq, 5));
    optimized = strings_optimize(strings, freq);
    assert(optimized);
    assert(strings_count(optimized) == count);
    assert(strings_lookup(optimized, "x5") == 1);
    strings_frequency_free(freq);
    strings_free(optimized);

#ifdef INLINE_UNSIGNED
    // test interning unsigned int strings
    for (unsigned i = 1; i <= count; i++) {
        unsigned_string(buffer, i);

        assert(strings_intern(strings, buffer, &id));
        assert(i == (id & 0x7FFFFFFF));

        assert(strings_intern(strings, buffer, &id));
        assert(i == (id & 0x7FFFFFFF));

        assert(strings_lookup(strings, buffer) == id);

        string = strings_lookup_id(strings, id);
        assert(string && !strcmp(buffer, string));
    }
#endif

    assert(strings_intern(strings, "2147483648", &id));
    assert(id == count + 1);

    assert(strings_intern(strings, "4294967295", &id));
    assert(id == count + 2);

    assert(strings_intern(strings, "4294967296", &id));
    assert(id == count + 3);

    assert(strings_intern(strings, "2147483647", &id));
#ifdef INLINE_UNSIGNED
    assert(id == 0xFFFFFFFF);
#else
    assert(id == count + 4);
#endif

    // test strings larger than the page size
    char *large_string = malloc(PAGE_SIZE + 1);
    assert(large_string);
    memset(large_string, 'x', PAGE_SIZE);
    large_string[PAGE_SIZE] = '\0';
    assert(!strings_intern(strings, large_string, &id));
    free(large_string);

    strings_free(strings);
    return 0;
}
