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

Table TableInit(size max_inserts) {
    // TODO: Slow algorithm, maybe there's better?
    size capacity = 2*max_inserts + 1;
    while (!IsPrime(capacity)) {
        capacity += 2;
    }
    Table table = (Table){
        .capacity = capacity,
        .n_elements = 0,
        .values = calloc(capacity, sizeof(u32))
    };
    if (table.values) {
        for (size i = 0; i < capacity; i++) {
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
    for (size i = 0; i < table->capacity; i++) {
        table->values[i] = INVALID_KEY;
    }
    table->n_elements = 0;
}

bool TableInsert(Table *table, u32 key) {
    // Make sure we have space
    assert(
        TableOkay(table) &&
        table->n_elements < table->capacity
    );

    size hk = HashU32(key)%table->capacity;
    for (size i = 0; i <= table->capacity; i++) {
        size idx = (hk + i*i)%table->capacity;
        if (table->values[idx] == INVALID_KEY) {
            table->values[idx] = key;
            table->n_elements++;
            return false;
        } else if (table->values[idx] == key) {
            return true;
        }
    }

    // If we are here then it means that the probe did not find an empty slot
    assert(false);
    return false;
}

bool TableHas(const Table *table, u32 key) {
    assert(TableOkay(table));

    // Early return if table is empty
    if (table->n_elements == 0) {
        return false;
    }

    size hk = HashU32(key)%table->capacity;
    for (size i = 0; i <= table->capacity; i++) {
        size idx = (hk + i*i)%table->capacity;
        if (table->values[idx] == INVALID_KEY) {
            return false;
        } else if (table->values[idx] == key) {
            return true;
        }
    }
    return false;
}

Array ArrayInit(size capacity) {
    assert(capacity > 0);
    return (Array){
        .capacity = capacity,
        .data = calloc(capacity, sizeof(u32))
    };
}

Array ArraySlice(Array *array, size offset, size length) {
    assert(ArrayOkay(array));
    Array slice = (Array){
        .capacity = length,
        .data = array->data + offset
    };
    if (offset + length > array->capacity) {
        slice.data = NULL;
    }
    return slice;
}

void ArrayFree(Array *array) {
    if (array->data) {
        free(array->data);
        array->data = NULL;
    }
}

bool ArrayOkay(const Array *array) {
    return array->data != NULL;
}

u32 *ArrayAt(Array *array, u32 idx) {
    return &array->data[idx];
}

void ArrayReverse(Array *array, size i, size j) {
    assert(
        ArrayOkay(array) &&
        i < array->capacity &&
        j < array->capacity
    );

    size length = (i <= j) ? (j - i + 1) : (array->capacity - j + i + 1);
    if (length <= 1) {
        return;
    }

    for (size a = 0; a < length/2; a++) {
        u32 temp = array->data[i];
        array->data[i] = array->data[j];
        array->data[j] = temp;
        i = (i + 1)%array->capacity;
        if (j == 0) {
            j = array->capacity - 1;
        } else {
            j--;
        }
    }
}

void ArrayShuffle(Array *array, size i, size j) {
    assert(
        ArrayOkay(array) &&
        i < array->capacity &&
        j < array->capacity
    );

    size length = (i <= j) ? (j - i + 1) : (array->capacity - j + i + 1);
    if (length <= 1) {
        return;
    }

    for (size a = 0; a <= length - 2; a++) {
        size b = RandomInt(a, length - 1);
        size idx1 = (i + a)%array->capacity;
        size idx2 = (i + b)%array->capacity;
        u32 temp = array->data[idx1];
        array->data[idx1] = array->data[idx2];
        array->data[idx2] = temp;
    }
}

void ArrayCopy(Array *dst, const Array *src, size n_elements) {
    assert(
        ArrayOkay(dst) &&
        ArrayOkay(src) &&
        dst->capacity >= n_elements &&
        src->capacity >= n_elements
    );
    if (dst == src) {
        return;
    }
    memcpy(dst->data, src->data, n_elements*sizeof(u32));
}
