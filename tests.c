#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "config.h"
#include "strings.h"
#include "optimize.h"
#include "unsigned.h"

#ifdef INLINE_UNSIGNED
# include <limits.h>
#endif

int main() {
    struct strings *strings = strings_new();
    assert(strings);

    char buffer[12] = {'x'};
    const char *string;
    unsigned count = 100000;

    // test interning strings
    for (unsigned i = 1; i <= count; i++) {
        unsigned_string(buffer + 1, i);
        assert(strings_intern(strings, buffer) == i);
    }
    assert(strings_count(strings) == count);

    // test idempotency
    for (unsigned i = 1; i <= count; i++) {
        unsigned_string(buffer + 1, i);
        assert(strings_intern(strings, buffer) == i);
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
    uint32_t id = 0;
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
    strings_free(optimized);
    assert(strings_frequency_add(freq, 3));
    assert(strings_frequency_add(freq, 3));
    optimized = strings_optimize(strings, freq);
    assert(optimized);
    assert(strings_count(optimized) == count);
    assert(strings_lookup(optimized, "x3") == 1);
    assert(strings_lookup(optimized, "x5") == 2);
    strings_frequency_free(freq);
    strings_free(optimized);

    // test interning unsigned int strings
#ifdef INLINE_UNSIGNED
    for (unsigned i = 1; i <= count; i++) {
        unsigned_string(buffer, i);
        assert((strings_intern(strings, buffer) & 0x7FFFFFFF) == i);
        assert((strings_intern(strings, buffer) & 0x7FFFFFFF) == i);
        assert((strings_lookup(strings, buffer) & 0x7FFFFFFF) == i);
        string = strings_lookup_id(strings, i | 0x80000000);
        assert(string && !strcmp(buffer, string));
    }
#endif
    assert(strings_intern(strings, "2147483648") == count + 1);
    assert(strings_intern(strings, "4294967295") == count + 2);
    assert(strings_intern(strings, "4294967296") == count + 3);
#ifdef INLINE_UNSIGNED
    uint32_t expected = UINT_MAX;
#else
    uint32_t expected = count + 4;
#endif
    assert(strings_intern(strings, "2147483647") == expected);

    // test strings larger than the page size
    char *large_string = malloc(PAGE_SIZE + 1);
    assert(large_string);
    memset(large_string, 'x', PAGE_SIZE);
    large_string[PAGE_SIZE] = '\0';
    assert(!strings_intern(strings, large_string));
    free(large_string);

    strings_free(strings);

    // test snapshots
    strings = strings_new();
    assert(strings);
    assert(!strings_count(strings));
    struct strings_snapshot middle_snapshot, end_snapshot;
    buffer[0] = 'x';
    for (unsigned i = 1; i <= count; i++) {
        unsigned_string(buffer + 1, i);
        assert(strings_intern(strings, buffer) == i);
        if (i == count / 2) {
            strings_snapshot(strings, &middle_snapshot);
        }
    }
    strings_snapshot(strings, &end_snapshot);
    assert(strings_count(strings) == count);
    assert(strings_restore(strings, &end_snapshot));
    assert(strings_count(strings) == count);
    assert(strings_restore(strings, &middle_snapshot));
    assert(strings_count(strings) == count / 2);
    assert(!strings_restore(strings, &end_snapshot));
    for (unsigned i = 1; i <= count / 2; i++) {
        unsigned_string(buffer + 1, i);
        assert(strings_lookup(strings, buffer) == i);
    }
    for (unsigned i = count / 2 + 1; i <= count; i++) {
        unsigned_string(buffer + 1, i);
        assert(!strings_lookup(strings, buffer));
    }
    assert(strings_intern(strings, "foobar") == count / 2 + 1);

    strings_free(strings);

    printf("ok\n");
    return 0;
}
