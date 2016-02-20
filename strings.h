#ifndef INTERN_STRINGS_H_
#define INTERN_STRINGS_H_

#include <stdint.h>

#include "block.h"

struct strings;

// Create a new repository of strings
struct strings *strings_new(void);

// Free a string repository
void strings_free(struct strings*);

// Count the number of unique strings in the repository
uint32_t strings_count(const struct strings*);

// Intern a string and get back a unique ID. Strings must be NULL-terminated.
// Note that IDs start at 1 and increment to either UINT_MAX or INT_MAX if
// INLINE_UNSIGNED was defined at compile time. This function returns 0 if an
// error occurred
uint32_t strings_intern(struct strings*, const char *string);

// Lookup the ID for a string. This function returns zero if the string
// does not exist in the repository
uint32_t strings_lookup(const struct strings*, const char *string);

// Lookup the string associated with an ID. This function returns NULL
// if there is no string with that ID in the repository. If INLINE_UNSIGNED
// was defined at compile time then the repository will inline unsigned
// integers into the ID itself. When looking up the string associated with such
// an ID, the integer is written into an internal buffer which persists until
// the function is called again
const char *strings_lookup_id(struct strings*, uint32_t id);

struct strings_cursor {
    const struct block *strings;
    size_t page;
    size_t offset;
    uint32_t id;
};

// Initialize a string cursor
void strings_cursor_init(struct strings_cursor*, const struct strings*);

// Advance the cursor, e.g. while (strings_cursor_next(&cursor)) { ... }
bool strings_cursor_next(struct strings_cursor*);

// Select the string/ID that the cursor currently points to, or NULL/0 if
// the cursor is invalid
const char *strings_cursor_string(const struct strings_cursor*);
uint32_t strings_cursor_id(const struct strings_cursor*);

struct strings_snapshot {
    struct block_snapshot strings;
    struct block_snapshot hashes;
    struct block_snapshot tree_nodes;
    uint32_t total;
};

// Create a snapshot. The string repository can later be restored to this
// position
void strings_snapshot(const struct strings*, struct strings_snapshot*);

// Restore the string repository to a previous position. Any strings added
// after the snapshot are removed. This function returns true if the restore
// was successful, and false if an error occurred
bool strings_restore(struct strings*, const struct strings_snapshot*);

// Get the total bytes allocated, including overhead
size_t strings_allocated_bytes(const struct strings*);

// Get the page size
size_t strings_page_size();

// Set the hash seed. This must be done before adding strings to the
// repository
bool strings_hash_seed(struct strings *strings, uint32_t seed);

#endif
