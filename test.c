#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "block.h"
#include "strings.h"
#include "unsigned.h"

static const int page_size = (int)(BLOCK_PAGE_SIZE - sizeof(int));

static void test_block_alloc_with_alloc_size(int alloc_size)
{
    struct block *block = block_new();
    assert(block);

    int total_allocs = page_size * 10 / alloc_size;

    for (int offset = 0, i = 0; i < total_allocs; i++) {
        void *ptr = block_alloc(block, alloc_size);

        if (offset + alloc_size > page_size) {
            if (block->count > 1) {
                void *page = block->pages[block->count - 2];
                int *offset_ptr = (int *)((uintptr_t)page + page_size);
                assert(*offset_ptr = offset);
                assert(page == block_get_page(block, block->count - 2,
                                              offset_ptr));
                assert(*offset_ptr = offset);
            }
            offset = 0;
        }

        void *expected_ptr = \
            (void *)((uintptr_t)block->pages[block->count - 1] + offset);
        assert(ptr == expected_ptr);
        offset += alloc_size;

        for (int j = 0; j < alloc_size; j++)
            ((char *)ptr)[j] = (char)(i % 255);
    }

    block_free(block);
}

static void test_block_alloc()
{
    int alloc_sizes[] = {3, 4, 111, page_size / 2 + 1, page_size};

    for (int i = 0; i < sizeof(alloc_sizes) / sizeof(int); i++)
        test_block_alloc_with_alloc_size(alloc_sizes[i]);
}

static void test_intern()
{
    struct strings *strings = strings_new();
    assert(strings);

    char buffer[12] = {'x'};
    const char *string;
    uint32_t id;

    unsigned count = 1000000;

    // test interning strings
    for (unsigned i = 1; i <= count; i++) {
        to_string(buffer + 1, i);

        assert(!strings_intern(strings, buffer, &id));
        assert(id == i);

        assert(!strings_intern(strings, buffer, &id));
        assert(id == i);

        assert(strings_lookup(strings, buffer) == id);

        string = strings_lookup_id(strings, id);
        assert(string && !strcmp(buffer, string));
    }

#ifdef INLINE_UNSIGNED
    // test interning unsigned int strings
    for (unsigned i = 1; i <= count; i++) {
        to_string(buffer, i);

        assert(!strings_intern(strings, buffer, &id));
        assert(i == (id & 0x7FFFFFFF));

        assert(!strings_intern(strings, buffer, &id));
        assert(i == (id & 0x7FFFFFFF));

        assert(strings_lookup(strings, buffer) == id);

        string = strings_lookup_id(strings, id);
        assert(string && !strcmp(buffer, string));
    }
#endif

    assert(!strings_intern(strings, "2147483648", &id));
    assert(id == count + 1);

    assert(!strings_intern(strings, "4294967295", &id));
    assert(id == count + 2);

    assert(!strings_intern(strings, "4294967296", &id));
    assert(id == count + 3);

    assert(!strings_intern(strings, "2147483647", &id));
#ifdef INLINE_UNSIGNED
    assert(id == 0xFFFFFFFF);
#else
    assert(id == count + 4);
#endif

    strings_free(strings);
}

int main()
{
    test_block_alloc();
    test_intern();
    return 0;
}
