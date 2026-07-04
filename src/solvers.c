#include "solvers.h"

Tour SolveBasic(const TSPInstance *tsp_instance) {
    assert(TSPInstanceOkay(tsp_instance));
    Tour tour = TourInit(tsp_instance->n_cities);
    if (!TourOkay(&tour)) {
        return tour;
    }
    for (u32 i = 0; i < tsp_instance->n_cities; i++) {
        *ArrayAt(&tour, i) = i;
    }
    return tour;
}

Tour SolveGreedy(const TSPInstance *tsp_instance, u32 starting_city) {
    assert(TSPInstanceOkay(tsp_instance));
    if (starting_city >= tsp_instance->n_cities) {
        starting_city = 0;
    }
    Tour tour = TourInit(tsp_instance->n_cities);
    if (!TourOkay(&tour)) {
        return tour;
    }

    Table visited_table = TableInit(tsp_instance->n_cities);

    *ArrayAt(&tour, 0) = starting_city;
    TableInsert(&visited_table, *ArrayAt(&tour, 0));
    for (u32 i = 1; i < tsp_instance->n_cities; i++) {
        u32 closest_neighbour = UINT32_MAX;
        f64 shortest_distance = INFINITY;
        for (u32 j = 0; j < tsp_instance->n_cities; j++) {
            if (TableHas(&visited_table, j)) {
                continue;
            }
            f64 distance = Vec2Distance(
                tsp_instance->city_positions[*ArrayAt(&tour, i - 1)],
                tsp_instance->city_positions[j]
            );
            if (distance < shortest_distance) {
                closest_neighbour = j;
                shortest_distance = distance;
            }
        }
        assert(closest_neighbour != UINT32_MAX);
        *ArrayAt(&tour, i) = closest_neighbour;
        TableInsert(&visited_table, closest_neighbour);
    }

    TableFree(&visited_table);

    return tour;
}
