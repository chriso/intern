#ifndef INTERN_OPTIMIZE_H_
#define INTERN_OPTIMIZE_H_

#include "strings.h"

struct strings_frequency;

// Create a new, optimized string repository which stores the most frequently
// seen strings together. The string with the lowest ID (1) is the most
// frequently seen string
struct strings *strings_optimize(const struct strings*,
                                 struct strings_frequency*);

// Create a new string frequency tracker
struct strings_frequency *strings_frequency_new(void);

// Free the string frequency tracker
void strings_frequency_free(struct strings_frequency*);

// Add a string ID. This should be called after interning a string and
// getting back the ID. This function returns true if the string ID was
// tracked, and false if an error occurred
bool strings_frequency_add(struct strings_frequency*, uint32_t id);

// Add all string IDs, to ensure that each string is present in the optimized
// repository. This function returns true if all strings were tracked, and
// false if an error occurred
bool strings_frequency_add_all(struct strings_frequency*,
                               const struct strings*);

#endif
