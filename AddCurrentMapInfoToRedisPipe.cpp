#include "pch.h"
#include "AddCurrentMapInfoToRedisPipe.h"

#include "GWClientHub_v2.h"

#include "Context/CharContext.h"

#include "Managers/MapMgr.h"

void AddCurrentMapInfoToRedisPipe()
{
    if (!GW::Map::GetIsMapLoaded()) return;

    static const auto map_info_key{
        std::format("client:{}:current_map_info", GWClientHub_v2::Instance().client_id)
    };
    static const auto changed_key{std::format("client:{}:changes", GWClientHub_v2::Instance().client_id)};


    const auto& current_map_id{GW::CharContext::instance()->current_map_id};
    const auto& current_is_explorable{GW::CharContext::instance()->is_explorable};
    const auto& current_world_flags{GW::CharContext::instance()->world_flags};
    const auto& current_world_id{GW::CharContext::instance()->token1};

    bool map_info_changed{false};
    if (current_map_id != map_id)
    {
        map_id = current_map_id;
        redis_pipe.hset(map_info_key, std::make_pair("map_id", std::to_string(map_id)));

        map_info_changed = true;
    }
    if (current_is_explorable != is_explorable)
    {
        is_explorable = current_is_explorable;
        redis_pipe.hset(map_info_key, std::make_pair("is_explorable", std::to_string(is_explorable)));

        map_info_changed = true;
    }
    if (world_flags != current_world_flags)
    {
        world_flags = current_world_flags;
        redis_pipe.hset(map_info_key, std::make_pair("world_flags", std::to_string(world_flags)));

        map_info_changed = true;
    }
    if (world_id != current_world_id)
    {
        world_id = current_world_id;
        redis_pipe.hset(map_info_key, std::make_pair("world_id", std::to_string(world_id)));

        map_info_changed = true;
    }

    if (map_info_changed)
    {
        redis_pipe.hincrby(changed_key, "current_map_info", 1);
    }
}
