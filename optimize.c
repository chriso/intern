#include <stdlib.h>
#include <string.h>

#include "optimize.h"
#include "branch.h"

struct id_count {
    uint32_t id;
    uint32_t count;
};

struct strings_frequency {
    struct id_count *counts;
    size_t size;
    uint32_t max_id;
    bool sorted_by_count;
};

struct strings_frequency *strings_frequency_new() {
    struct strings_frequency *frequency = malloc(sizeof(*frequency));
    if (!frequency) {
        return NULL;
    }
    frequency->counts = malloc(sizeof(*frequency->counts));
    if (!frequency->counts) {
        free(frequency);
        return NULL;
    }
    frequency->counts[0].count = 0;
    frequency->max_id = 0;
    frequency->size = 1;
    frequency->sorted_by_count = false;
    return frequency;
}

void strings_frequency_free(struct strings_frequency *frequency) {
    free(frequency->counts);
    free(frequency);
}

__attribute__ ((noinline))
static bool resize_counts(struct strings_frequency *frequency,
                          size_t required_size) {
    size_t new_size = frequency->size;
    while (new_size < required_size) {
        new_size *= 2;
    }
    struct id_count *counts = realloc(frequency->counts,
                                      sizeof(*counts) * new_size);
    if (UNLIKELY(!counts)) {
        return false;
    }
    memset(counts + frequency->size, 0,
           (new_size - frequency->size) * sizeof(struct id_count));
    frequency->counts = counts;
    frequency->size = new_size;
    return true;
}

static int count_comparator(const void *a_, const void *b_) {
    const struct id_count *a = (const struct id_count *)a_;
    const struct id_count *b = (const struct id_count *)b_;
    return b->count > a->count ? 1 : (a->count > b->count ? -1 : 0);
}

static int id_comparator(const void *a_, const void *b_) {
    const struct id_count *a = (const struct id_count *)a_;
    const struct id_count *b = (const struct id_count *)b_;
    return b->id > a->id ? -1 : (a->id > b->id ? 1 : 0);
}

static void sort_by_count(struct strings_frequency *frequency) {
    frequency->sorted_by_count = true;
    qsort(frequency->counts, frequency->max_id, sizeof(struct id_count),
          count_comparator);
}

static void sort_by_id(struct strings_frequency *frequency) {
    frequency->sorted_by_count = false;
    qsort(frequency->counts, frequency->max_id, sizeof(struct id_count),
          id_comparator);
}

bool strings_frequency_add(struct strings_frequency *frequency, uint32_t id) {
#ifdef INLINE_UNSIGNED
    if (id > unsigned_tag) {
        return true;
    }
#endif
    if (UNLIKELY(!id)) {
        return false;
    }
    if (UNLIKELY(frequency->sorted_by_count)) {
        sort_by_id(frequency);
    }
    if (frequency->max_id < id) {
        if (UNLIKELY(frequency->size < id)) {
            if (!resize_counts(frequency, id)) {
                return false;
            }
        }
        frequency->max_id = id;
    }
    frequency->counts[id - 1].count++;
    return true;
}

bool strings_frequency_add_all(struct strings_frequency *frequency,
                               const struct strings *strings) {
    if (frequency->sorted_by_count) {
        sort_by_id(frequency);
    }
    uint32_t total = strings_count(strings);
    if (frequency->size < total && !resize_counts(frequency, total)) {
        return false;
    }
    frequency->max_id = total;
    for (uint32_t id = 0; id < total; id++) {
        frequency->counts[id].count++;
    }
    return true;
}

struct strings *strings_optimize(const struct strings *strings,
                                 struct strings_frequency *frequency) {
    struct strings *optimized = strings_new();
    if (!optimized) {
        return NULL;
    }

    if (!frequency->sorted_by_count) {
        for (uint32_t i = 0; i < frequency->max_id; i++) {
            frequency->counts[i].id = i + 1;
        }
        sort_by_count(frequency);
    }

    for (uint32_t i = 0; i < frequency->max_id; i++) {
        const struct id_count *id_count = &frequency->counts[i];
        if (!id_count->count) {
            break;
        }
        const char *string = strings_lookup_id(strings, id_count->id);
        uint32_t new_id;
        if (UNLIKELY(!string || !strings_intern(optimized, string, &new_id))) {
            goto error;
        }
    }
    return optimized;
error:
    strings_free(optimized);
    return NULL;
}
