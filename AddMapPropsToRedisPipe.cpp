#include "pch.h"
#include "AddMapPropsToRedisPipe.h"
#include "GWClientHub_v2.h"

#include "Context/CharContext.h"
#include "Context/GameContext.h"
#include "Context/MapContext.h"
#include "Managers/MapMgr.h"

void AddMapPropsToRedisPipe(float dt)
{
    if (time_since_updated_map_props < 5)
    {
        time_since_updated_map_props += dt;
        return;
    }

    if (! GW::Map::GetIsMapLoaded())
        return;

    const auto& map_id = GW::CharContext::instance()->current_map_id;
    if (existing_map_props.contains(map_id))
    {
        time_since_updated_map_props = 0;
        return;
    }

    const auto* const game_context = GW::GameContext::instance();
    if (! game_context)
        return;
    const auto* const map_context = game_context->map;
    if (! map_context)
        return;

    const std::string boundaries_key = std::format("map:{}:boundaries", map_id);
    const auto existing_boundary0 = redis.hget(boundaries_key, "boundary0");
    const auto existing_boundary1 = redis.hget(boundaries_key, "boundary1");
    const auto existing_boundary2 = redis.hget(boundaries_key, "boundary2");
    const auto existing_boundary3 = redis.hget(boundaries_key, "boundary3");
    const auto existing_boundary4 = redis.hget(boundaries_key, "boundary4");

    const auto& boundary0 = map_context->map_boundaries[0];
    const auto& boundary1 = map_context->map_boundaries[1];
    const auto& boundary2 = map_context->map_boundaries[2];
    const auto& boundary3 = map_context->map_boundaries[3];
    const auto& boundary4 = map_context->map_boundaries[4];

    const auto existing_boundaries_have_values = existing_boundary0 || existing_boundary1 ||
      existing_boundary2 || existing_boundary3 || existing_boundary4;
    if (existing_boundaries_have_values)
    {
        const auto boundaries_already_added = std::stof(existing_boundary0.value()) == boundary0 &&
          std::stof(existing_boundary1.value()) == boundary1 &&
          std::stof(existing_boundary2.value()) == boundary2 &&
          std::stof(existing_boundary3.value()) == boundary3 &&
          std::stof(existing_boundary4.value()) == boundary4;

        // We've already added the boundary data for this map.
        if (boundaries_already_added)
        {
            existing_map_props.insert(map_id);
            return;
        }
    }

    std::unordered_map<std::string, float> boundaries_map;
    boundaries_map.insert({"boundary0", boundary0});
    boundaries_map.insert({"boundary1", boundary1});
    boundaries_map.insert({"boundary2", boundary2});
    boundaries_map.insert({"boundary3", boundary3});
    boundaries_map.insert({"boundary4", boundary4});

    redis_pipe.hmset(boundaries_key, boundaries_map.begin(), boundaries_map.end());

    time_since_updated_map_props = 0;
}
