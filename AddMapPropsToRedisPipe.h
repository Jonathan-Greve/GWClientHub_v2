#pragma once
#include <unordered_set>

inline float time_since_updated_map_props = 0;
inline std::unordered_set<uint32_t> existing_map_props;

void AddMapPropsToRedisPipe(float dt);
