#pragma once
#include <unordered_set>

inline float time_since_updated_map_boundaries = 0;
inline std::unordered_set<uint32_t> existing_map_boundaries_map_id;

void AddMapBoundariesToRedisPipe(float dt);
