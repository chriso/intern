#ifndef INTERN_STRINGS_H_
#define INTERN_STRINGS_H_

#include <stdint.h>
#include <stdbool.h>

struct strings;

// Create a new repository of strings
struct strings *strings_new(void);

// Free the string repository
void strings_free(struct strings*);

// Intern a string and get back a unique ID. Strings must be NULL-terminated.
// This function returns true if the string was successfully interned. Note
// that IDs start at 1 and increment towards UINT_MAX
bool strings_intern(struct strings*, const char *string, uint32_t *id);

// Lookup the ID for a string. This function returns zero if the string
// does not exist in the repository
uint32_t strings_lookup(const struct strings*, const char *string);

// Lookup the string associated with an ID. This function returns NULL
// if there is no string with that ID in the repository. If INLINE_UNSIGNED
// is defined then the repository will inline unsigned integers into the ID
// itself. When looking up the string associated with such an ID, the integer
// is written into an internal buffer which persists until the function is
// called again
#ifdef INLINE_UNSIGNED
  const char *strings_lookup_id(struct strings *strings, uint32_t id);
#else
  const char *strings_lookup_id(const struct strings *strings, uint32_t id);
#endif

struct strings_cursor {
    const struct strings *strings;
    size_t page;
    size_t offset;
    uint32_t id;
};

// Initialize a string cursor
void strings_cursor_init(struct strings_cursor *, const struct strings *);

// Advance the cursor, e.g. while (strings_cursor_next(&cursor)) { ... }.
bool strings_cursor_next(struct strings_cursor *);

// Select the string/ID that the cursor currently points to, or NULL/0 if
// the cursor is invalid
const char *strings_cursor_string(const struct strings_cursor *);
uint32_t strings_cursor_id(const struct strings_cursor *);

#endif
