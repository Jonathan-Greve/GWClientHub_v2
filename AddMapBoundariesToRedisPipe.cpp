#include "pch.h"
#include "AddMapBoundariesToRedisPipe.h"
#include "AddPathingMapToRedisPipe.h"
#include "GWClientHub_v2.h"

#include "Context/CharContext.h"
#include "Context/GameContext.h"

#include "Managers/MapMgr.h"

void AddMapBoundariesToRedisPipe(float dt)
{
    if (time_since_updated_map_boundaries < 5)
    {
        time_since_updated_map_boundaries += dt;
        return;
    }

    if (! GW::Map::GetIsMapLoaded())
        return;

    const auto& map_id = GW::CharContext::instance()->current_map_id;
    if (existing_map_boundaries_map_id.contains(map_id))
    {
        time_since_updated_map_boundaries = 0;
        return;
    }

    if (! map_bounds_set.contains(map_id))
    {
        return;
    }

    const auto* const game_context = GW::GameContext::instance();
    if (! game_context)
        return;
    const auto* const map_context = game_context->map;
    if (! map_context)
        return;

    const std::string boundaries_key = std::format("map:{}:bounds", map_id);
    const auto existing_max_x = redis.hget(boundaries_key, "max_x");
    const auto existing_max_y = redis.hget(boundaries_key, "max_y");
    const auto existing_max_z = redis.hget(boundaries_key, "max_z");
    const auto existing_min_x = redis.hget(boundaries_key, "min_x");
    const auto existing_min_y = redis.hget(boundaries_key, "min_y");
    const auto existing_min_z = redis.hget(boundaries_key, "min_z");
    const auto existing_avg_z = redis.hget(boundaries_key, "avg_z");
    const auto existing_center_x = redis.hget(boundaries_key, "center_x");
    const auto existing_center_y = redis.hget(boundaries_key, "center_y");
    const auto existing_center_z = redis.hget(boundaries_key, "center_z");

    const auto existing_boundaries_have_values = existing_max_x && existing_max_y && existing_max_z &&
      existing_min_z && existing_avg_z && existing_center_x && existing_center_y && existing_center_z;
    if (existing_boundaries_have_values)
    {
        const auto boundaries_already_added = map_max_x[map_id] == std::stof(existing_max_x.value()) &&
          map_max_y[map_id] == std::stof(existing_max_y.value()) &&
          map_max_z[map_id] == std::stof(existing_max_z.value()) &&
          map_min_x[map_id] == std::stof(existing_min_x.value()) &&
          map_min_y[map_id] == std::stof(existing_min_y.value()) &&
          map_min_z[map_id] == std::stof(existing_min_z.value()) &&
          map_avg_z[map_id] == std::stof(existing_avg_z.value()) &&
          map_center_x[map_id] == std::stof(existing_center_x.value()) &&
          map_center_y[map_id] == std::stof(existing_center_y.value()) &&
          map_center_z[map_id] == std::stof(existing_center_z.value());

        // We've already added the boundary data for this map.
        if (boundaries_already_added)
        {
            existing_map_boundaries_map_id.insert(map_id);
            return;
        }
    }

    std::unordered_map<std::string, float> bounds_map;
    bounds_map.insert({"max_x", map_max_x[map_id]});
    bounds_map.insert({"max_y", map_max_y[map_id]});
    bounds_map.insert({"max_z", map_max_z[map_id]});
    bounds_map.insert({"min_x", map_min_x[map_id]});
    bounds_map.insert({"min_y", map_min_y[map_id]});
    bounds_map.insert({"min_z", map_min_z[map_id]});
    bounds_map.insert({"center_x", map_center_x[map_id]});
    bounds_map.insert({"center_y", map_center_y[map_id]});
    bounds_map.insert({"center_z", map_center_z[map_id]});
    bounds_map.insert({"avg_z", map_avg_z[map_id]});

    redis_pipe.hmset(boundaries_key, bounds_map.begin(), bounds_map.end());

    time_since_updated_map_boundaries = 0;
}
