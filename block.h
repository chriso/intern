#ifndef BLOCK_H_
#define BLOCK_H_

#ifndef BLOCK_PAGE_SIZE
# define BLOCK_PAGE_SIZE 65536
#endif

struct block {
    void **pages;
    int count;
};

struct block *block_new(void);

void block_free(struct block*);

void *block_alloc(struct block*, int bytes);

const void *block_get_page(const struct block*, int page, int *bytes_used);

const void *block_get_offset(const struct block *block, size_t offset,
                             int bytes);

void block_stats(const struct block *block, size_t *bytes_allocated,
                 size_t *bytes_used);

#endif
