#ifndef INTERN_H_
#define INTERN_H_

#include <stdint.h>

struct strings;

// Create a new repository of strings
struct strings *strings_new(void);

// Free the string repository
void strings_free(struct strings*);

// Intern a string and get back a unique ID. Strings must be NULL-terminated.
// This function returns zero if the string was successfully interned. Note
// that IDs start at 1 and increment towards UINT_MAX
int strings_intern(struct strings*, const char *string, uint32_t *id);

// Lookup the ID for a string. This function returns zero if the string
// does not exist in the repository
uint32_t strings_lookup(const struct strings*, const char *string);

// Lookup the string associated with an ID. This function returns NULL
// if there is no string with that ID in the repository
const char *strings_lookup_id(struct strings*, uint32_t id);

#endif
