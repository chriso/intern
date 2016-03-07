#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "config.h"
#include "strings.h"
#include "tree.h"

#ifdef INLINE_UNSIGNED
#include "unsigned.h"

const static uint32_t unsigned_tag = 0x80000000;
const static uint32_t id_overflow = 0x80000000;

static int is_small_unsigned(const char *string) {
    char c;
    int numeric = 0;
    const char *ptr = string;
    while ((c = *ptr++)) {
        if (c < '0' || c > '9' || numeric > 10) {
            numeric = -1;
            break;
        }
        numeric++;
    }
    return numeric >= 1 && \
        (numeric < 10 || (numeric == 10 && string[0] <= '2'));
}

static uint32_t to_unsigned(const char *string) {
    uint32_t number = 0;
    char c;
    while ((c = *string++)) {
        number = number * 10 + (c - '0');
    }
    return number;
}

#else
const static uint32_t id_overflow = 0;
#endif

typedef struct tree_node_s tree_node_t;
struct tree_node_s {
    uint32_t hash;
    uint32_t id;
    const char *string;
    rb_node(tree_node_t) tree_link;
    tree_node_t *next;
};

typedef rb_tree(tree_node_t) tree_t;

static int tree_cmp(tree_node_t *a, tree_node_t *b) {
    return a->hash < b->hash ? -1 : (a->hash > b->hash ? 1 : 0);
}

rb_gen(static, tree_, tree_t, tree_node_t, tree_link, tree_cmp)

struct strings {
    struct block *hashes;
    struct block *strings;
    struct block *index;
    tree_t hash_map;
    uint32_t total;
    uint32_t hash_seed;
#ifdef INLINE_UNSIGNED
    char buffer[11];
#endif
};

struct strings *strings_new() {
    struct strings *strings = malloc(sizeof(*strings));
    if (!strings) {
        return NULL;
    }

    strings->hashes = block_new(PAGE_SIZE);
    strings->strings = block_new(PAGE_SIZE);
    strings->index = block_new(PAGE_SIZE);
    if (!strings->hashes || !strings->strings || !strings->index) {
        goto error;
    }

    tree_new(&strings->hash_map);

    strings->total = 0;
    strings->hash_seed = 5381;

    return strings;

error:
    if (strings->hashes) {
        block_free(strings->hashes);
    }
    if (strings->strings) {
        block_free(strings->strings);
    }
    if (strings->index) {
        block_free(strings->index);
    }
    free(strings);
    return NULL;
}

void strings_free(struct strings *strings) {
    block_free(strings->hashes);
    block_free(strings->strings);
    block_free(strings->index);
    free(strings);
}

static uint32_t strings_hash(const struct strings *strings,
                             const char *string) {
    uint32_t hash = strings->hash_seed;
    char c;
    while ((c = *string++)) {
        hash = ((hash << 5) + hash) + (uint32_t)c;
    }
    return hash;
}

bool strings_hash_seed(struct strings *strings, uint32_t seed) {
    if (strings->total) {
        return false;
    }
    strings->hash_seed = seed;
    return true;
}

static tree_node_t *create_node(struct strings *strings, uint32_t hash,
                                const char *string) {
    tree_node_t *node = block_alloc(strings->index, sizeof(*node));
    if (!node) {
        return NULL;
    }
    node->hash = hash;
    node->next = NULL;
    node->id = strings->total + 1;
    if (node->id == id_overflow) {
        return NULL;
    }

    size_t len = strlen(string);
    void *string_ptr = block_alloc(strings->strings, len + 1);
    if (!string_ptr) {
        return NULL;
    }
    memcpy(string_ptr, string, len + 1);
    node->string = (const char *)string_ptr;

    uint32_t *hash_ptr = block_alloc(strings->hashes, sizeof(*hash_ptr));
    if (!hash_ptr) {
        return NULL;
    }
    *hash_ptr = hash;

    strings->total++;

    return node;
}

static tree_node_t *find_node(const struct strings *strings, uint32_t hash) {
    tree_node_t key;
    key.hash = hash;
    return tree_search(&strings->hash_map, &key);
}

__attribute__ ((noinline))
static bool strings_intern_collision(struct strings *strings,
                                     tree_node_t *node, const char *string,
                                     uint32_t hash) {
    for (;;) {
        if (!node->next) {
            node = node->next = create_node(strings, hash, string);
            if (!node) {
                return 0;
            }
            break;
        }
        node = node->next;
        if (!strcmp(node->string, string)) {
            break;
        }
    }
    return node->id;
}

uint32_t strings_count(const struct strings *strings) {
    return strings->total;
}

uint32_t strings_intern(struct strings *strings, const char *string) {
#ifdef INLINE_UNSIGNED
    if (is_small_unsigned(string)) {
        uint32_t number = to_unsigned(string);
        if (number < unsigned_tag) {
            return number | unsigned_tag;
        }
    }
#endif

    uint32_t hash = strings_hash(strings, string);
    tree_node_t *node = find_node(strings, hash);
    if (node) {
        if (strcmp(node->string, string)) {
            return strings_intern_collision(strings, node, string, hash);
        }
    } else {
        if (!(node = create_node(strings, hash, string))) {
            return 0;
        }
        tree_insert(&strings->hash_map, node);
    }

    return node->id;
}

uint32_t strings_lookup(const struct strings *strings, const char *string) {
#ifdef INLINE_UNSIGNED
    if (is_small_unsigned(string)) {
        uint32_t number = to_unsigned(string);
        if (number < unsigned_tag) {
            return number | unsigned_tag;
        }
    }
#endif

    uint32_t hash = strings_hash(strings, string);
    tree_node_t *node = find_node(strings, hash);
    if (node) {
        do {
            if (!strcmp(node->string, string)) {
                return node->id;
            }
        } while ((node = node->next));
    }

    return 0;
}

const char *strings_lookup_id(struct strings *strings, uint32_t id) {
#ifdef INLINE_UNSIGNED
    if (id & unsigned_tag) {
        unsigned_string(strings->buffer, id & ~unsigned_tag);
        return strings->buffer;
    }
#endif

    if (id > strings->total) {
        return NULL;
    }

    size_t hashes_per_page = PAGE_SIZE / sizeof(uint32_t);
    size_t offset = (size_t)(id - 1);
    size_t page_num = offset / hashes_per_page;
    size_t page_offset = offset % hashes_per_page;
    void *page = strings->hashes->pages[page_num];
    uint32_t hash =
        *(const uint32_t *)((uintptr_t)page + page_offset * sizeof(uint32_t));

    tree_node_t *node = find_node(strings, hash);
    do {
        if (node->id == id) {
            return node->string;
        }
    } while ((node = node->next));

    return NULL;
}

void strings_cursor_init(struct strings_cursor *cursor,
                         const struct strings *strings) {
    cursor->strings = strings->strings;
    cursor->page = 0;
    cursor->offset = 0;
    cursor->id = 0;
}

bool strings_cursor_next(struct strings_cursor *cursor) {
    const struct block *block = cursor->strings;
    if (cursor->id) {
        const char *string = strings_cursor_string(cursor);
        cursor->offset += strlen(string) + 1;
        if (cursor->offset >= block->offsets[cursor->page]) {
            cursor->page++;
            cursor->offset = 0;
        }
    }
    if (cursor->page >= block->count ||
            cursor->offset >= block->offsets[cursor->page]) {
        cursor->id = 0;
        return false;
    }
    cursor->id++;
    return true;
}

const char *strings_cursor_string(const struct strings_cursor *cursor) {
    if (!cursor->id) {
        return NULL;
    }
    const void *page = cursor->strings->pages[cursor->page];
    return (const char *)((uintptr_t)page + cursor->offset);
}

uint32_t strings_cursor_id(const struct strings_cursor *cursor) {
    return cursor->id;
}

void strings_snapshot(const struct strings *strings,
                      struct strings_snapshot *snapshot) {
    block_snapshot(strings->strings, &snapshot->strings);
    block_snapshot(strings->hashes, &snapshot->hashes);
    block_snapshot(strings->index, &snapshot->index);
    snapshot->total = strings->total;
}

bool strings_restore(struct strings *strings,
                     const struct strings_snapshot *snapshot) {
    struct block *block = strings->index;
    if (snapshot->index.count == block->count &&
            snapshot->index.offset == block->offsets[block->count - 1]) {
        return true;
    }

    if (!block_restore(strings->strings, &snapshot->strings) ||
            !block_restore(strings->hashes, &snapshot->hashes) ||
            !block_restore(strings->index, &snapshot->index)) {
        return false;
    }
    strings->total = snapshot->total;
    tree_new(&strings->hash_map);
    tree_node_t *node, *existing;
    for (size_t page = 0; page < block->count; page++) {
        for (size_t offset = 0; offset < block->offsets[page];
                offset += sizeof(tree_node_t)) {
            node = (tree_node_t *)((uintptr_t)block->pages[page] + offset);
            existing = find_node(strings, node->hash);
            if (existing) {
                while (existing->next) {
                    existing = existing->next;
                }
                existing->next = node;
            } else {
                tree_insert(&strings->hash_map, node);
            }
        }
    }
    return true;
}

size_t strings_allocated_bytes(const struct strings *strings) {
    return block_allocated_bytes(strings->strings) +
        block_allocated_bytes(strings->hashes) +
        block_allocated_bytes(strings->index) +
        sizeof(*strings);
}

size_t strings_page_size() {
    return PAGE_SIZE;
}
