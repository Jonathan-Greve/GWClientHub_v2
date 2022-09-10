#include "pch.h"
#include "AddPathingMapToRedisPipe.h"

#include "GWClientHub_v2.h"

#include "Context/CharContext.h"
#include "Context/GameContext.h"

#include "GameEntities/Pathing.h"

#include "Managers/MapMgr.h"


/**
 * \brief Add the PathingMap for the current map to redis. This contains the trapezoid required to draw
 * the navigation mesh amongst other things. This takes multiple megabytes so we only do this operation
 * if the data does not already exists on the redis server.
 * \param dt Time since function was last called. 
 */
void AddPathingMapToRedisPipe(const float dt)
{
    if (time_since_updated_pathing_map < 5)
    {
        time_since_updated_pathing_map += dt;
        return;
    }

    const GW::PathingMapArray* path_map;

    if (GW::Map::GetIsMapLoaded())
        path_map = GW::Map::GetPathingMap();
    else return;

    // We've aleady added trapezoid info for this map
    const auto& map_id = GW::CharContext::instance()->current_map_id;
    if (existing_pathing_map_id.contains(map_id))
    {
        time_since_updated_pathing_map = 0;
        return;
    }
    uint32_t number_of_trapezoids = 0;
    for (const GW::PathingMap& map : *path_map)
        number_of_trapezoids += map.trapezoid_count;
    if (number_of_trapezoids == 0) return;


    std::string& client_id = GWClientHub_v2::Instance().client_id;
    const std::string trapezoid_key = std::format("client:{}:map:{}:trapezoids", client_id, map_id);

    const auto expected_hash_length = number_of_trapezoids * 12 + 1;
    const auto existing_hash_length = redis.hlen(trapezoid_key);

    // We've already added the trapezoid data for this map.
    if (existing_hash_length == expected_hash_length)
    {
        existing_pathing_map_id.insert(map_id);
        return;
    }

    std::unordered_map<std::string, std::string> trapezoid_map;

    const GW::GameContext* const g = GW::GameContext::instance();
    if (!g) return;

    const GW::MapContext* const map_context = g->map;
    if (!map_context) return;

    const GW::Array<uint32_t>& sub1s = map_context->sub1->pathing_map_block;
    for (size_t i = 0; i < path_map->size(); ++i)
    {
        const GW::PathingMap pmap = path_map->m_buffer[i];
        for (size_t j = 0; j < pmap.trapezoid_count; ++j)
        {
            GW::PathingTrapezoid& trapezoid = pmap.trapezoids[j];
            std::string trapezoid_id = std::to_string(trapezoid.id);
            bool is_traversable = !sub1s[i];

            float radius = 0;

            float altitude = 0;
            GW::GamePos gp{trapezoid.XBL, trapezoid.YB, i};
            GW::Map::QueryAltitude(gp, radius, altitude);
            GW::Vec3f BL{trapezoid.XBL, trapezoid.YB, altitude};

            GW::GamePos gp2{trapezoid.XBR, trapezoid.YB, i};
            GW::Map::QueryAltitude(gp2, radius, altitude);
            GW::Vec3f BR{trapezoid.XBR, trapezoid.YB, altitude};

            GW::GamePos gp3{trapezoid.XTL, trapezoid.YT, i};
            GW::Map::QueryAltitude(gp3, radius, altitude);
            GW::Vec3f TL{trapezoid.XTL, trapezoid.YT, altitude};

            GW::GamePos gp4{trapezoid.XTR, trapezoid.YT, i};
            GW::Map::QueryAltitude(gp4, radius, altitude);
            GW::Vec3f TR{trapezoid.XTR, trapezoid.YT, altitude};

            // Set x coordinates
            trapezoid_map.insert({"x_top_left_" + trapezoid_id, std::to_string(TL.x)});
            trapezoid_map.insert({"x_top_right_" + trapezoid_id, std::to_string(TR.x)});
            trapezoid_map.insert({"x_bottow_left_" + trapezoid_id, std::to_string(BL.x)});
            trapezoid_map.insert({"x_bottom_right_" + trapezoid_id, std::to_string(BR.x)});

            // Set y values
            trapezoid_map.insert({"y_top_" + trapezoid_id, std::to_string(trapezoid.YT)});
            trapezoid_map.insert({"y_bottom_" + trapezoid_id, std::to_string(trapezoid.YB)});

            // Set z coordinates
            trapezoid_map.insert({"z_top_left_" + trapezoid_id, std::to_string(TL.z)});
            trapezoid_map.insert({"z_top_right_" + trapezoid_id, std::to_string(TR.z)});
            trapezoid_map.insert({"z_bottow_left_" + trapezoid_id, std::to_string(BL.z)});
            trapezoid_map.insert({"z_bottom_right_" + trapezoid_id, std::to_string(BR.z)});


            trapezoid_map.insert({"is_traversable_" + trapezoid_id, std::to_string(is_traversable)});
            trapezoid_map.insert({"map_layer_" + trapezoid_id, std::to_string(i)});
        }
    }

    trapezoid_map.insert({"number_of_trapezoids", std::to_string(number_of_trapezoids)});

    // Add all trapezoid data to pipeline
    redis_pipe.hmset(trapezoid_key, trapezoid_map.begin(), trapezoid_map.end());

    time_since_updated_pathing_map = 0;
}
