#include "dsa.h"

// Credit for two round hash function: https://github.com/skeeto/hash-prospector
// Credit for low bias parameters: https://github.com/skeeto/hash-prospector/issues/19
inline u32 HashU32(u32 x) {
    x ^= x >> 16;
    x *= 0x21f0aaad;
    x ^= x >> 15;
    x *= 0xd35a2d97;
    x ^= x >> 15;
    return x;
}

Table TableInit(u32 max_inserts) {
    // TODO: Slow algorithm, maybe there's better?
    u32 capacity = 2*max_inserts + 1;
    while (!IsPrime(capacity)) {
        capacity += 2;
    }
    Table table = (Table){
        .capacity = capacity,
        .n_elements = 0,
        .values = calloc(capacity, sizeof(u32))
    };
    if (table.values) {
        for (u32 i = 0; i < capacity; i++) {
            table.values[i] = INVALID_KEY;
        }
    }
    return table;
}

void TableFree(Table *table) {
    if (table->values) {
        free(table->values);
        table->values = NULL;
    }
}

bool TableOkay(const Table *table) {
    return table->values != NULL;
}

void TableClear(Table *table) {
    assert(TableOkay(table));
    for (u32 i = 0; i < table->capacity; i++) {
        table->values[i] = INVALID_KEY;
    }
    table->n_elements = 0;
}

void TableInsert(Table *table, u32 key) {
    // Make sure we have space
    assert(
        TableOkay(table) &&
        table->n_elements < table->capacity
    );

    // TODO: would i*i overflow and cause the modulus result to be wrong?
    u32 hk = HashU32(key)%table->capacity;
    for (u32 i = 0; i <= table->capacity; i++) {
        u32 idx = (hk + i*i)%table->capacity;
        if (table->values[idx] == INVALID_KEY) {
            table->values[idx] = key;
            table->n_elements++;
            return;
        }
    }

    // If we are here then it means that the probe did not find an empty slot
    assert(false);
}

bool TableHas(const Table *table, u32 key) {
    assert(TableOkay(table));

    // Early return if table is empty
    if (table->n_elements == 0) {
        return false;
    }

    // TODO: would i*i overflow and cause the modulus result to be wrong?
    u32 hk = HashU32(key)%table->capacity;
    for (u32 i = 0; i <= table->capacity; i++) {
        u32 idx = (hk + i*i)%table->capacity;
        if (table->values[idx] == INVALID_KEY) {
            break;
        }
        if (table->values[idx] == key) {
            return true;
        }
    }
    return false;
}

Counter CounterInit(u32 capacity) {
    Counter counter = (Counter){
        .capacity = capacity,
        .counts = calloc(capacity, sizeof(u32))
    };
    if (counter.counts) {
        for (u32 i = 0; i < capacity; i++) {
            counter.counts[i] = 0;
        }
    }
    return counter;
}

void CounterFree(Counter *counter) {
    if (counter->counts) {
        free(counter->counts);
        counter->counts = NULL;
    }
}

bool CounterOkay(const Counter *counter) {
    return counter->counts != NULL;
}

void CounterClear(Counter *counter) {
    assert(CounterOkay(counter));
    for (u32 i = 0; i < counter->capacity; i++) {
        counter->counts[i] = 0;
    }
}

void CounterIncrement(Counter *counter, u32 idx, u32 amount) {
    assert(CounterOkay(counter) && idx < counter->capacity);
    counter->counts[idx] += amount;
}

u32 CounterCount(const Counter *counter, u32 idx) {
    assert(CounterOkay(counter) && idx < counter->capacity);
    return counter->counts[idx];
}
