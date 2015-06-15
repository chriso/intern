#ifndef INTERN_H_
#define INTERN_H_

#include <stdint.h>

struct strings;

struct strings *strings_new(void);

void strings_free(struct strings*);

int strings_intern(struct strings*, const char *string, uint32_t *id);

uint32_t strings_lookup(const struct strings*, const char *string);

const char *strings_lookup_id(struct strings*, uint32_t id);

#endif
