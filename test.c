#include <stddef.h>
#include <assert.h>

#include "block.h"

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
                int *offset_ptr = (int *)((ptrdiff_t)page + page_size);
                assert(*offset_ptr = offset);
                assert(page == block_get_page(block, block->count - 2,
                                              offset_ptr));
                assert(*offset_ptr = offset);
            }
            offset = 0;
        }
        void *expected_ptr = \
            (void *)((ptrdiff_t)block->pages[block->count - 1] + offset);
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

int main()
{
    test_block_alloc();
    return 0;
}
