// dsa.h: data structures and algorithms
#ifndef DSA_H
#define DSA_H
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "util.h"

// Used to determine if a slot in the table is empty
#define INVALID_KEY (UINT32_MAX)

// Hash table with u32 keys; the keys are the values themselves
// Does not support deletion
// Can not dynamically change in size
// Open addressing is used with simple quadratic probing for collisions
// Probe sequence may not visit all slots, therefore care has to be put into the table size
typedef struct {
    size capacity;
    size n_elements;
    u32 *values;
} Table;

// Quick and dirty hash with low bias
// Two round hash function: https://github.com/skeeto/hash-prospector
// Low bias parameters: https://github.com/skeeto/hash-prospector/issues/19
u32 HashU32(u32 x);

// Allocate a table, the capacity will be made in a specific way to ensure probe sequence works
Table TableInit(size max_inserts);

// Free table memory
void TableFree(Table *table);

// Check if table allocation worked
bool TableOkay(const Table *table);

// Clears a table of all elements
void TableClear(Table *table);

// Insert key into table and returns if that key was already there or not
bool TableInsert(Table *table, u32 key);

// Check if table has a certain key in it
bool TableHas(const Table *table, u32 key);

// Array of u32s
typedef struct {
    size capacity;
    u32 *data;
} Array;

// Allocate an array
Array ArrayInit(size capacity);

// Creates array slice (must not be freed)
Array ArraySlice(Array *array, size offset, size length);

// Free array memory
void ArrayFree(Array *array);

// Check if array allocation worked
bool ArrayOkay(const Array *array);

// Returns pointer to array element at index
u32 *ArrayAt(Array *array, u32 idx);

// Reverses array from i to j (i and j can wrap around)
void ArrayReverse(Array *array, size i, size j);

// Shuffles array from i to j (i and j can wrap around)
void ArrayShuffle(Array *array, size i, size j);

// Copy from src to dst
void ArrayCopy(Array *dst, const Array *src, size n_elements);

#endif // DSA_H
