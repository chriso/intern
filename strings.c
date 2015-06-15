#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "strings.h"
#include "block.h"

#define RB_COMPACT
#include "rb.h"

#define LIKELY(cond) __builtin_expect(!!(cond), 1)
#define UNLIKELY(cond) __builtin_expect(!!(cond), 0)

typedef struct tree_node_s tree_node_t;
struct tree_node_s {
    uint32_t hash;
    rb_node(tree_node_t) tree_link;
    uint32_t id;
    const char *string;
    tree_node_t *next;
};

typedef rb_tree(tree_node_t) tree_t;

static int tree_cmp(tree_node_t *a, tree_node_t *b)
{
    return a->hash < b->hash ? -1 : (a->hash > b->hash ? 1 : 0);
}

rb_gen(static, tree_, tree_t, tree_node_t, tree_link, tree_cmp);

struct strings {
    uint32_t total;
    struct block *hashes;
    struct block *strings;
    struct block *tree_nodes;
    tree_t hash_map;
};

struct strings *strings_new() {
    struct strings *strings = malloc(sizeof(*strings));
    if (!strings)
        return NULL;

    strings->hashes = block_new();
    strings->strings = block_new();
    strings->tree_nodes = block_new();
    if (!strings->hashes || !strings->strings || !strings->tree_nodes)
        goto error;

    tree_new(&strings->hash_map);

    strings->total = 0;

    return strings;
error:
    if (strings->hashes)
        block_free(strings->hashes);
    if (strings->strings)
        block_free(strings->hashes);
    if (strings->tree_nodes)
        block_free(strings->tree_nodes);
    free(strings);
    return NULL;
}

void strings_free(struct strings *strings)
{
    block_free(strings->hashes);
    block_free(strings->strings);
    block_free(strings->tree_nodes);
    free(strings);
}

static uint32_t strings_hash(const char *string)
{
    uint32_t hash = 5381;
    char c;
    while ((c = *string++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

static tree_node_t *create_node(struct strings *strings, uint32_t hash,
                                const char *string)
{
    tree_node_t *node = block_alloc(strings->tree_nodes, sizeof(*node));
    if (UNLIKELY(!node))
        return NULL;

    uint32_t *hash_ptr = block_alloc(strings->hashes, sizeof(uint32_t));
    if (UNLIKELY(!hash_ptr))
        goto error;
    *hash_ptr = hash;

    int len = strlen(string);
    void *string_ptr = block_alloc(strings->strings, len + 1);
    if (UNLIKELY(!string_ptr))
        goto error;
    memcpy(string_ptr, string, len + 1);

    node->hash = hash;
    node->id = ++strings->total;
    node->string = (const char *)string_ptr;
    node->next = NULL;

    return node;
error:
    free(node);
    return NULL;
}

static tree_node_t *find_node(const struct strings *strings, uint32_t hash)
{
    tree_node_t key = {hash};
    return tree_search((tree_t *)&strings->hash_map, &key);
}

__attribute__ ((noinline))
static int strings_intern_collision(struct strings *strings, tree_node_t *node,
                                    const char *string, uint32_t hash,
                                    uint32_t *id)
{
    for (;;) {
        if (!strcmp(node->string, string))
            break;
        if (!node->next) {
            node = node->next = create_node(strings, hash, string);
            if (!node)
                return 1;
            break;
        }
        node = node->next;
    }
    *id = node->id;
    return 0;
}

int strings_intern(struct strings *strings, const char *string, uint32_t *id)
{
    uint32_t hash = strings_hash(string);
    tree_node_t *node = find_node(strings, hash);
    if (node) {
        if (UNLIKELY(strcmp(node->string, string)))
            return strings_intern_collision(strings, node, string, hash, id);
    } else {
        if (UNLIKELY(!(node = create_node(strings, hash, string))))
            return 1;
        tree_insert(&strings->hash_map, node);
    }
    *id = node->id;
    return 0;
}

uint32_t strings_lookup(const struct strings *strings, const char *string)
{
    uint32_t hash = strings_hash(string);
    tree_node_t *node = find_node(strings, hash);
    if (node)
        do {
            if (!strcmp(node->string, string))
                return node->id;
        } while ((node = node->next));
    return 0;
}

const char *strings_lookup_id(const struct strings *strings, uint32_t id)
{
    if (UNLIKELY(id > strings->total))
        return NULL;

    int hashes_per_page = (BLOCK_PAGE_SIZE - sizeof(int)) / sizeof(uint32_t);
    int page_num = (id - 1) / hashes_per_page;
    int page_offset = (id - 1) % hashes_per_page;

    const void *page = strings->hashes->pages[page_num];
    uint32_t hash = \
        *(uint32_t *)((uintptr_t)page + page_offset * sizeof(uint32_t));

    tree_node_t *node = find_node(strings, hash);
    if (LIKELY(node))
        do {
            if (node->id == id)
                return node->string;
        } while ((node = node->next));

    return NULL;
}