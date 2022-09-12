#pragma once
#include "Context/MapContext.h"
#include <unordered_set>

inline float time_since_updated_pathing_map = 0;
inline std::unordered_set<uint32_t> existing_pathing_map_id;

void AddMapTrapezoidsToRedisPipe(float dt);
