
#include "tsp.h"

TSPInstance TSPInstanceInit(const char *tsp_path) {
    assert(tsp_path);

    TSPInstance tsp_instance = (TSPInstance){
        .n_cities = 0,
        .city_positions = NULL
    };

    bool good_so_far = false;
    FILE *file = fopen(tsp_path, "r");
    if (!file) {
        return tsp_instance;
    }
    char *line = NULL;
    size length = 0;
    i32 read;
    while ((read = getline(&line, &length, file)) != -1) {
        if (strstr(line, "DIMENSION")) {
            if (sscanf(line, "DIMENSION : %u", &tsp_instance.n_cities) == 1) {
                good_so_far = true;
                break;
            }
            break;
        }
    }

    if (!good_so_far) {
        fclose(file);
        if (line) {
            free(line);
        }
        return tsp_instance;
    }
    good_so_far = false;

    tsp_instance.city_positions = calloc(
        tsp_instance.n_cities,
        sizeof(*tsp_instance.city_positions)
    );
    if (!tsp_instance.city_positions) {
        fclose(file);
        if (line) {
            free(line);
        }
        tsp_instance.n_cities = 0;
        return tsp_instance;
    }

    while ((read = getline(&line, &length, file)) != -1) {
        if (strstr(line, "NODE_COORD_SECTION")) {
            good_so_far = true;
            break;
        }
    }

    if (!good_so_far) {
        tsp_instance.n_cities = 0;
        free(tsp_instance.city_positions);
        tsp_instance.city_positions = NULL;
        fclose(file);
        if (line) {
            free(line);
        }
        return tsp_instance;
    }
    good_so_far = false;

    u32 n_positions_read = 0;
    while ((read = getline(&line, &length, file)) != -1) {
        u32 idx;
        vec2 position;
        if (sscanf(line, "%u %lf %lf", &idx, &position.x, &position.y) == 3) {
            if (idx > tsp_instance.n_cities) {
                break;
            }
            tsp_instance.city_positions[idx - 1] = position;
            n_positions_read++;
        }
    }

    fclose(file);
    if (line) {
        free(line);
    }

    if (n_positions_read != tsp_instance.n_cities) {
        tsp_instance.n_cities = 0;
        free(tsp_instance.city_positions);
        tsp_instance.city_positions = NULL;
        return tsp_instance;
    }

    tsp_instance.shortest_edge_length = INFINITY;
    tsp_instance.longest_edge_length = -INFINITY;
    for (u32 i = 0; i < tsp_instance.n_cities - 1; i++) {
        for (u32 j = i + 1; j < tsp_instance.n_cities; j++) {
            f64 edge_length = Vec2Distance(tsp_instance.city_positions[i], tsp_instance.city_positions[j]);
            if (edge_length < tsp_instance.shortest_edge_length) {
                tsp_instance.shortest_edge_length = edge_length;
            }
            if (edge_length > tsp_instance.longest_edge_length) {
                tsp_instance.longest_edge_length = edge_length;
            }
        }
    }

    return tsp_instance;
}

void TSPInstanceFree(TSPInstance *tsp_instance) {
    if (tsp_instance->city_positions) {
        free(tsp_instance->city_positions);
    }
}

bool TSPInstanceOkay(const TSPInstance *tsp_instance) {
    return tsp_instance->city_positions != NULL;
}

Tour TourInit(u32 n_cities) {
    return ArrayInit(n_cities);
}

void TourFree(Tour *tour) {
    ArrayFree(tour);
}

bool TourOkay(const Tour *tour) {
    return ArrayOkay(tour);
}

bool TourReadFromFile(Tour *tour, const char *file_path) {
    assert(TourOkay(tour) && file_path);

    u32 n_cities = tour->capacity;

    bool dimension_okay = false;
    bool type_okay = false;
    FILE *file = fopen(file_path, "r");
    if (!file) {
        return false;
    }
    char *line = NULL;
    size length = 0;
    i32 read;
    while ((read = getline(&line, &length, file)) != -1) {
        if (strstr(line, "DIMENSION")) {
            u32 file_n_cities;
            if (
                sscanf(line, "DIMENSION : %u", &file_n_cities) == 1 &&
                file_n_cities == n_cities
            ) {
                dimension_okay = true;
            } else {
                break;
            }
        } else if (strstr(line, "TYPE")) {
            if (sscanf(line, "TYPE : TOUR") == 0) {
                type_okay = true;
            } else {
                break;
            }
        }
        if (dimension_okay && type_okay) {
            break;
        }
    }

    if (!dimension_okay || !type_okay) {
        fclose(file);
        if (line) {
            free(line);
        }
        return false;
    }

    bool good_so_far = false;

    while ((read = getline(&line, &length, file)) != -1) {
        if (strstr(line, "TOUR_SECTION")) {
            good_so_far = true;
            break;
        }
    }

    if (!good_so_far) {
        fclose(file);
        if (line) {
            free(line);
        }
        return false;
    }

    good_so_far = false;

    u32 n_cities_read = 0;
    while ((read = getline(&line, &length, file)) != -1) {
        i32 city;
        if (sscanf(line, "%i", &city) == 1) {
            if (city > (i32)n_cities || city < 0) {
                break;
            }
            if (n_cities_read >= n_cities) {
                n_cities_read = 0;
            }
            *ArrayAt(tour, n_cities_read++) = city - 1;
        } else {
            // Shouldn't be here... TODO: is this needed really?
            assert(false);
        }
    }

    fclose(file);
    if (line) {
        free(line);
    }

    return n_cities_read == n_cities;
}

void TourWriteToFile(Tour *tour, const char *name, const char *comment, const char *file_path) {
    assert(TourIsValid(tour, NULL));
    assert(name && comment && file_path);
    FILE *file = fopen(file_path, "w");
    if (!file) {
        return;
    }

    u32 n_cities = tour->capacity;

    fprintf(file, "NAME: %s\n", name);
    fprintf(file, "COMMENT: %s\n", comment);
    fprintf(file, "TYPE: TOUR\n");
    fprintf(file, "DIMENSION: %u\n", n_cities);
    fprintf(file, "TOUR_SECTION\n");
    for (u32 i = 0; i < n_cities; i++) {
        fprintf(file, "%u\n", *ArrayAt(tour, i) + 1);
    }
    fprintf(file, "-1");

    fclose(file);
}

void TourRandomize(Tour *tour) {
    assert(TourOkay(tour));
    u32 n_cities = tour->capacity;
    for (u32 i = 0; i < n_cities; i++) {
        *ArrayAt(tour, i) = i;
    }
    ArrayShuffle(tour, 0, n_cities - 1);
}

bool TourIsValid(Tour *tour, Table *table) {
    assert(TourOkay(tour));

    u32 n_cities = tour->capacity;

    // Make sure that when we do provide a table, it has space
    assert(!table || (TableOkay(table) && table->capacity >= 2*n_cities));

    bool free_table = false;
    Table table_struct;
    if (!table) {
        table_struct = TableInit(n_cities);
        table = &table_struct;
        free_table = true;
    } else {
        TableClear(table);
    }

    bool is_valid = true;
    for (u32 i = 0; i < n_cities; i++) {
        u32 city = *ArrayAt(tour, i);
        if (city >= n_cities) {
            is_valid = false;
            break;
        }
        if (TableInsert(table, city)) {
            is_valid = false;
            break;
        }
    }

    if (free_table) {
        TableFree(table);
    }

    return is_valid;
}

f64 TourEvaluate(const TSPInstance *tsp_instance, Tour *tour) {
    assert(TSPInstanceOkay(tsp_instance) && TourOkay(tour));
    return RoundNearest(tsp_instance->longest_edge_length)/TourLength(tsp_instance, tour);
}

f64 TourLength(const TSPInstance *tsp_instance, Tour *tour) {
    assert(TSPInstanceOkay(tsp_instance) && TourOkay(tour));
    f64 total_distance = 0.f;
    vec2 last_city = tsp_instance->city_positions[*ArrayAt(tour, 0)];
    for (u32 i = 1; i < tsp_instance->n_cities; i++) {
        vec2 current_city = tsp_instance->city_positions[*ArrayAt(tour, i)];
        total_distance += RoundNearest(Vec2Distance(last_city, current_city));
        last_city = current_city;
    }
    total_distance += RoundNearest(Vec2Distance(
        last_city,
        tsp_instance->city_positions[*ArrayAt(tour, 0)]
    ));
    return total_distance;
}

void TourCopy(Tour *dst_tour, const Tour *src_tour) {
    assert(dst_tour->capacity == src_tour->capacity);
    u32 n_cities = dst_tour->capacity;
    ArrayCopy(dst_tour, src_tour, n_cities);
}

void TourPrint(Tour *tour) {
    for (u32 i = 0; i < tour->capacity; i++) {
        printf("%u\n", *ArrayAt(tour, i));
    }
}

u32 TourArrayFix(TourArray *tour_array, u32 n_cities, Table *table) {
    assert(!table || (TableOkay(table) && table->capacity >= 2*n_cities));

    bool free_table = false;
    Table table_struct;
    if (!table) {
        table_struct = TableInit(n_cities);
        table = &table_struct;
        free_table = true;
    } else {
        TableClear(table);
    }

    u32 n_fix = 0;
    size n_tours = tour_array->capacity/n_cities;
    for (size i = 0; i < n_tours; i++) {
        Tour tour = TourArrayAt(tour_array, n_cities, i);
        if (TourIsValid(&tour, table)) {
            continue;
        }
        n_fix++;
        TourRandomize(&tour);
    }

    if (free_table) {
        TableFree(table);
    }
    return n_fix;
}

TourArray TourArrayInit(u32 n_cities, size n_tours) {
    return ArrayInit(n_cities*n_tours);
}

void TourArrayFree(TourArray *tour_array) {
    ArrayFree(tour_array);
}

bool TourArrayOkay(const TourArray *tour_array) {
    return ArrayOkay(tour_array);
}

void TourArrayCopy(TourArray *dst_array, const TourArray *src_array, u32 n_cities, size n_tours) {
    ArrayCopy(dst_array, src_array, n_cities*n_tours);
}

Tour TourArrayAt(TourArray *tour_array, u32 n_cities, size idx) {
    return ArraySlice(tour_array, n_cities*idx, n_cities);
}

TourArray TourArraySlice(TourArray *tour_array, u32 n_cities, size idx, size n_tours) {
    return ArraySlice(tour_array, n_cities*idx, n_cities*n_tours);
}

bool TourArrayIsValid(TourArray *tour_array, u32 n_cities) {
    assert(TourArrayOkay(tour_array));
    size n_tours = tour_array->capacity/n_cities;
    for (size i = 0; i < n_tours; i++) {
        Tour tour = TourArrayAt(tour_array, n_cities, i);
        if (!TourIsValid(&tour, NULL)) {
            return false;
        }
    }
    return true;
}
