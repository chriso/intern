#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "block.h"
#include "branch.h"

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

    block->offsets = malloc(sizeof(*block->offsets));
    if (!block->offsets)
        goto error;

    block->offsets[0] = 0;
    block->size = 1;
    block->count = 1;

    return block;

error:
    if (block->pages)
        free(block->pages);
    if (block->offsets)
        free(block->offsets);
    free(block);
    return NULL;
}

void block_free(struct block *block)
{
    for (int i = 0; i < block->count; i++)
        free_page(block->pages[i]);

    free(block->offsets);
    free(block->pages);
    free(block);
}

__attribute__ ((noinline))
static void *add_page(struct block *block)
{
    if (UNLIKELY(block->count == block->size)) {
        size_t new_size = block->size * 2;
        void **pages = realloc(block->pages, sizeof(*pages) * new_size);
        if (!pages)
            return NULL;
        block->pages = pages;
        size_t *offsets = realloc(block->offsets, sizeof(*offsets) * new_size);
        if (!offsets)
            return NULL;
        block->offsets = offsets;
        block->size = new_size;
    }
    void *page = alloc_page();
    if (!page)
        return NULL;
    block->pages[block->count] = page;
    block->offsets[block->count] = 0;
    block->count++;
    return page;
}

void *block_alloc(struct block *block, size_t size)
{
    assert(size <= BLOCK_PAGE_SIZE);
    size_t offset = block->offsets[block->count - 1];
    void *page;
    if (UNLIKELY(BLOCK_PAGE_SIZE - offset < size)) {
        page = add_page(block);
        if (UNLIKELY(!page))
            return NULL;
        offset = 0;
    } else {
        page = block->pages[block->count - 1];
    }
    block->offsets[block->count - 1] += size;
    return (void *)((uintptr_t)page + offset);
}
