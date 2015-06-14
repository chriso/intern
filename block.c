#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#include "block.h"

#ifdef BLOCK_MMAP
#include <sys/mman.h>

static inline void *alloc_page()
{
    return mmap(NULL, BLOCK_PAGE_SIZE, PROT_READ | PROT_WRITE,
                MAP_ANON | MAP_PRIVATE, -1, 0);
}

static inline void free_page(void *ptr)
{
    munmap(ptr, BLOCK_PAGE_SIZE);
}

#else

static inline void *alloc_page()
{
    return malloc(BLOCK_PAGE_SIZE);
}

static inline void free_page(void *ptr)
{
    free(ptr);
}

#endif

static const int page_size = (int)(BLOCK_PAGE_SIZE - sizeof(int));

struct block *block_new() {
    struct block *block = malloc(sizeof(*block));
    if (!block)
        return NULL;

    block->pages = malloc(sizeof(*block->pages));
    if (!block->pages)
        goto error;

    block->pages[0] = alloc_page();
    if (!block->pages[0])
        goto error;

    block->count = 1;
    block->offset = 0;

    return block;
error:
    if (block->pages)
        free(block->pages);
    free(block);
    return NULL;
}

void block_free(struct block *block)
{
    for (int i = 0; i < block->count; i++)
        free_page(block->pages[i]);

    free(block->pages);
    free(block);
}

static inline void *last_page(const struct block *block)
{
    return block->pages[block->count - 1];
}

static inline int *get_offset_ptr(const void *page)
{
    return (int *)((ptrdiff_t)page + page_size);
}

static inline void reset_offset(struct block *block)
{
    void *page = last_page(block);
    int *offset = get_offset_ptr(page);
    *offset = block->offset;
    block->offset = 0;
}

static inline int next_pow_2(int num)
{
    num--;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num++;
    return num;
}

__attribute__ ((noinline))
static void *add_page(struct block *block)
{
    if (block->count == next_pow_2(block->count)) {
        size_t new_size = (size_t)(block->count * 2);
        void **pages = realloc(block->pages, sizeof(*pages) * new_size);
        if (!pages)
            return NULL;
        block->pages = pages;
    }
    void *page = alloc_page();
    if (!page)
        return NULL;
    reset_offset(block);
    return block->pages[block->count++] = page;
}

void *block_alloc(struct block *block, int size)
{
    assert(size && size <= page_size);
    void *page;
    if (page_size - block->offset >= size)
        page = last_page(block);
    else if (!(page = add_page(block)))
        return NULL;
    void *ptr = (void *)((ptrdiff_t)page + block->offset);
    block->offset += size;
    return ptr;
}

const void *block_get_page(const struct block *block, int page_num,
                           int *bytes_used)
{
    assert(page_num < block->count);
    void *page = block->pages[page_num];
    if (page_num == block->count - 1)
        *bytes_used = block->offset;
    else
        *bytes_used = *get_offset_ptr(page);
    return page;
}
