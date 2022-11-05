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
void AddMapTrapezoidsToRedisPipe(const float dt)
{
    if (time_since_updated_pathing_map < 5)
    {
        time_since_updated_pathing_map += dt;
        return;
    }

    const GW::PathingMapArray* path_map;

    if (GW::Map::GetIsMapLoaded())
        path_map = GW::Map::GetPathingMap();
    else
        return;

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
    if (number_of_trapezoids == 0)
        return;

    const std::string trapezoid_key{std::format("map:{}:trapezoids", map_id)};

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
    if (! g)
        return;

    const GW::MapContext* const map_context = g->map;
    if (! map_context)
        return;

    const GW::Array<uint32_t>& sub1s = map_context->sub1->pathing_map_block;
    float min_x = 1000000000;
    float max_x = -1000000000;
    float min_y = 1000000000;
    float max_y = -1000000000;
    float min_z = 1000000000;
    float max_z = -1000000000;
    float avg_z = 0;
    uint32_t number_of_points = 0;

    for (size_t i = 0; i < path_map->size(); ++i)
    {
        const GW::PathingMap pmap = path_map->m_buffer[i];
        for (size_t j = 0; j < pmap.trapezoid_count; ++j)
        {
            GW::PathingTrapezoid& trapezoid = pmap.trapezoids[j];
            std::string trapezoid_id = std::to_string(trapezoid.id);
            bool is_traversable = ! sub1s[i];

            float radius = 10;

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

            // Compute average z value
            avg_z += TL.z + TR.z + BL.z + BR.z;
            number_of_points += 4;

            if (BL.x < min_x)
                min_x = BL.x;
            else if (BL.x > max_x)
                max_x = BL.x;
            if (BL.y < min_y)
                min_y = BL.y;
            else if (BL.y > max_y)
                max_y = BL.y;
            if (BL.z < min_z)
                min_z = BL.z;
            else if (BL.z > max_z)
                max_z = BL.z;

            if (BR.x < min_x)
                min_x = BR.x;
            else if (BR.x > max_x)
                max_x = BR.x;
            if (BR.y < min_y)
                min_y = BR.y;
            else if (BR.y > max_y)
                max_y = BR.y;
            if (BR.z < min_z)
                min_z = BR.z;
            else if (BR.z > max_z)
                max_z = BR.z;

            if (TL.x < min_x)
                min_x = TL.x;
            else if (TL.x > max_x)
                max_x = TL.x;
            if (TL.y < min_y)
                min_y = TL.y;
            else if (TL.y > max_y)
                max_y = TL.y;
            if (TL.z < min_z)
                min_z = TL.z;
            else if (TL.z > max_z)
                max_z = TL.z;

            if (TR.x < min_x)
                min_x = TR.x;
            else if (TR.x > max_x)
                max_x = TR.x;
            if (TR.y < min_y)
                min_y = TR.y;
            else if (TR.y > max_y)
                max_y = TR.y;
            if (TR.z < min_z)
                min_z = TR.z;
            else if (TR.z > max_z)
                max_z = TR.z;
        }
    }

    map_max_x[map_id] = max_x;
    map_min_x[map_id] = min_x;
    map_max_y[map_id] = max_y;
    map_min_y[map_id] = min_y;
    map_max_z[map_id] = max_z;
    map_min_z[map_id] = min_z;
    map_center_x[map_id] = (max_x + min_x) / 2.0f;
    map_center_y[map_id] = (max_y + min_y) / 2.0f;
    map_center_z[map_id] = (max_z + min_z) / 2.0f;
    avg_z = avg_z / number_of_points;
    map_avg_z[map_id] = avg_z;
    map_bounds_set.insert(map_id);

    trapezoid_map.insert({"number_of_trapezoids", std::to_string(number_of_trapezoids)});

    // Add all trapezoid data to pipeline
    redis_pipe.hmset(trapezoid_key, trapezoid_map.begin(), trapezoid_map.end());

    time_since_updated_pathing_map = 0;
}

void AddMapGridPointsToRedisPipe(float dt)
{
    if (time_since_updated_grid_points < 5)
    {
        time_since_updated_grid_points += dt;
        return;
    }

    if (! GW::Map::GetIsMapLoaded())
        return;

    const auto& map_id = GW::CharContext::instance()->current_map_id;

    if (existing_grid_points_map_id.contains(map_id))
    {
        time_since_updated_grid_points = 0;
        return;
    }

    const std::string grid_key{std::format("map:{}:grid_points", map_id)};

    float map_width{map_max_x[map_id] - map_min_x[map_id]};
    float map_height{map_max_y[map_id] - map_min_y[map_id]};

    uint32_t num_grid_points_x{static_cast<uint32_t>(map_width / grid_width) + 1};
    uint32_t num_grid_points_y{static_cast<uint32_t>(map_height / grid_height) + 1};

    const auto num_grid_points{(num_grid_points_x * num_grid_points_y)};
    if (redis.hlen(grid_key) == static_cast<long long>(num_grid_points) * 3 + 3)
    {
        existing_grid_points_map_id.insert(map_id);
        return;
    }

    std::unordered_map<std::string, std::string> grid_point_map;
    for (uint32_t i = 0; i < num_grid_points_x; i++)
    {
        for (uint32_t j = 0; j < num_grid_points_y; j++)
        {
            float x{map_min_x[map_id] + i * grid_width};
            float y{map_min_y[map_id] + j * grid_height};

            GW::GamePos gp{x, y, 0};
            float altitude{0};
            // 0, 0x41200000, 0x3f800000, 0x40a00000
            GW::Map::QueryAltitude(gp, 10, altitude);
            GW::Vec3f grid_point{x, y, altitude};

            std::string grid_point_id{std::to_string(i) + "_" + std::to_string(j) + "_"};

            grid_point_map.insert({grid_point_id + "x", std::to_string(grid_point.x)});
            grid_point_map.insert({grid_point_id + "y", std::to_string(grid_point.y)});
            grid_point_map.insert({grid_point_id + "z", std::to_string(grid_point.z)});
        }
    }

    redis_pipe.hmset(grid_key, grid_point_map.begin(), grid_point_map.end());
    redis_pipe.hset(grid_key, "grid_width", std::to_string(grid_width));
    redis_pipe.hset(grid_key, "grid_height", std::to_string(grid_height));
    redis_pipe.hset(grid_key, "number_of_grid_points", std::to_string(num_grid_points));

    time_since_updated_grid_points = 0;
}
