#include "hash.h"

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

bool TableOkay(Table *table) {
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

bool TableHas(Table *table, u32 key) {
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

Counter CounterInit(u32 max_inserts) {
    // TODO: Slow algorithm, maybe there's better?
    u32 capacity = 2*max_inserts + 1;
    while (!IsPrime(capacity)) {
        capacity += 2;
    }
    Counter counter = (Counter){
        .capacity = capacity,
        .n_elements = 0,
        .keys_and_counts = calloc(2*capacity, sizeof(u32))
    };
    if (counter.keys_and_counts) {
        for (u32 i = 0; i < capacity; i++) {
            counter.keys_and_counts[(size)2*i] = INVALID_KEY;
            counter.keys_and_counts[(size)2*i + 1] = 0;
        }
    }
    return counter;
}

void CounterFree(Counter *counter) {
    if (counter->keys_and_counts) {
        free(counter->keys_and_counts);
        counter->keys_and_counts = NULL;
    }
}

bool CounterOkay(Counter *counter) {
    return counter->keys_and_counts != NULL;
}

void CounterClear(Counter *counter) {
    assert(CounterOkay(counter));
    for (u32 i = 0; i < counter->capacity; i++) {
        counter->keys_and_counts[(size)2*i] = INVALID_KEY;
        counter->keys_and_counts[(size)2*i + 1] = 0;
    }
    counter->n_elements = 0;
}

void CounterIncrement(Counter *counter, u32 key, u32 amount) {
    // Make sure we have space
    assert(
        CounterOkay(counter) &&
        counter->n_elements < counter->capacity
    );

    // TODO: would i*i overflow and cause the modulus result to be wrong?
    u32 hk = HashU32(key)%counter->capacity;
    for (u32 i = 0; i <= counter->capacity; i++) {
        size idx = (size)2*((hk + i*i)%counter->capacity);
        if (counter->keys_and_counts[idx] == INVALID_KEY) {
            counter->keys_and_counts[idx] = key;
            counter->keys_and_counts[idx + 1] = amount;
            counter->n_elements++;
            return;
        } else if (counter->keys_and_counts[idx] == key) {
            counter->keys_and_counts[idx + 1] += amount;
            return;
        }
    }

    // If we are here then it means that the probe did not find an empty slot
    assert(false);
}

u32 CounterCount(Counter *counter, u32 key) {
    assert(CounterOkay(counter));

    // Early return if counter is empty
    if (counter->n_elements == 0) {
        return false;
    }

    // TODO: would i*i overflow and cause the modulus result to be wrong?
    u32 hk = HashU32(key)%counter->capacity;
    for (u32 i = 0; i <= counter->capacity; i++) {
        size idx = (size)2*((hk + i*i)%counter->capacity);
        if (counter->keys_and_counts[idx] == INVALID_KEY) {
            break;
        }
        if (counter->keys_and_counts[idx] == key) {
            return counter->keys_and_counts[idx + 1];
        }
    }
    return 0;
}
