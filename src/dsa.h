// dsa.h: data structures and algorithms
#ifndef DSA_H
#define DSA_H
#include <assert.h>
#include <stdlib.h>
#include "types.h"
#include "util.h"

// Used to determine if a slot in the table is empty
#define INVALID_KEY (UINT32_MAX)

// Hash table with u32 keys; the keys are the values themselves
// Does not support deletion
// Can not dynamically change in size
// Open addressing is used with simple quadratic probing for collisions
// Probe sequence may not visit all slots, therefore care has to be put into the table size
// TODO: maybe make table size a power of 2 and use (i^2+i)/2 as the probe?
typedef struct {
    u32 capacity;
    u32 n_elements;
    u32 *values;
} Table;

// Quick and dirty hash with low bias
// Two round hash function: https://github.com/skeeto/hash-prospector
// Low bias parameters: https://github.com/skeeto/hash-prospector/issues/19
u32 HashU32(u32 x);

// Allocate a table, the capacity will be made in a specific way to ensure probe sequence works
Table TableInit(u32 max_inserts);

// Free table memory
void TableFree(Table *table);

// Check if table allocation worked
bool TableOkay(Table *table);

// Clears a table of all elements
void TableClear(Table *table);

// Insert key into table
void TableInsert(Table *table, u32 key);

// Check if table has a certain key in it
bool TableHas(Table *table, u32 key);

// Counter with u32 keys
typedef struct {
    u32 capacity;
    u32 *counts;
} Counter;

// Allocate a counter
Counter CounterInit(u32 capacity);

// Free counter memory
void CounterFree(Counter *counter);

// Check if counter allocation worked
bool CounterOkay(Counter *counter);

// Clears all counts
void CounterClear(Counter *counter);

// Returns pointer to counter's count at a specific index
u32 *CounterAt(Counter *counter, u32 idx);

#endif // DSA_H
