#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "block.h"

#ifdef BLOCK_MMAP
#include <sys/mman.h>

static inline void *alloc_page()
{
    void *ptr = mmap(NULL, BLOCK_PAGE_SIZE, PROT_READ | PROT_WRITE,
                     MAP_ANON | MAP_PRIVATE, -1, 0);
    if (!ptr)
        return NULL;
    memset(ptr, 0, BLOCK_PAGE_SIZE);
    return ptr;
}

static inline void free_page(void *ptr)
{
    munmap(ptr, BLOCK_PAGE_SIZE);
}

#else

static inline void *alloc_page()
{
    return calloc(1, BLOCK_PAGE_SIZE);
}

static inline void free_page(void *ptr)
{
    free(ptr);
}

#endif

static const int page_size = (int)(BLOCK_PAGE_SIZE - sizeof(int));

struct block *block_new()
{
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
    return (int *)((uintptr_t)page + page_size);
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
    return block->pages[block->count++] = page;
}

void *block_alloc(struct block *block, int size)
{
    assert(size && size <= page_size);
    void *page = last_page(block);
    int *page_offset = get_offset_ptr(page);
    if (page_size - *page_offset < size)
        if (!(page = add_page(block)))
            return NULL;
    page_offset = get_offset_ptr(page);
    void *ptr = (void *)((uintptr_t)page + *page_offset);
    *page_offset += size;
    return ptr;
}

const void *block_get_page(const struct block *block, int page_num,
                           int *bytes_used)
{
    assert(page_num < block->count);
    void *page = block->pages[page_num];
    *bytes_used = *get_offset_ptr(page);
    return page;
}

const void *block_get_offset(const struct block *block, size_t offset,
                             int bytes)
{
    size_t allocs_per_page = (BLOCK_PAGE_SIZE - sizeof(int)) / bytes;
    size_t page_num = offset / allocs_per_page;
    size_t page_offset = offset % allocs_per_page;

    assert(page_num <= block->count - 1);

    void *page = block->pages[page_num];

    assert(page_num < block->count - 1 || \
           (page_num == block->count - 1 && \
            page_offset + bytes <= *get_offset_ptr(page)));

    return (const void *)((uintptr_t)page + page_offset * bytes);
}

void block_stats(const struct block *block, size_t *bytes_allocated,
                 size_t *bytes_used)
{
    int page_bytes_used = *bytes_allocated = *bytes_used = 0;
    for (int i = 0; i < block->count; i++) {
        block_get_page(block, i, &page_bytes_used);
        *bytes_allocated += BLOCK_PAGE_SIZE;
        *bytes_used += page_bytes_used;
    }
}
