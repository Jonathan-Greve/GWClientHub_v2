#pragma once
#include "Context/MapContext.h"
#include <unordered_set>

inline float time_since_updated_pathing_map = 0;
inline std::unordered_set<uint32_t> existing_pathing_map_id;

// Store the position of the max and min vertices of the map.
// Since the highest map_id < 900 we just use an array of sizxe 900
// as our hash table.
inline float map_max_x[900];
inline float map_min_x[900];
inline float map_max_y[900];
inline float map_min_y[900];
inline float map_center_x[900];
inline float map_center_y[900];

inline float grid_width{50};
inline float grid_height{50};
inline float time_since_updated_grid_points = 0;
inline std::unordered_set<uint32_t> existing_grid_points_map_id;

void AddMapTrapezoidsToRedisPipe(float dt);
void AddMapGridPointsToRedisPipe(float dt);
