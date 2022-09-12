#include "pch.h"
#include "AddCharacterPositionToRedisPipe.h"

#include "GWClientHub_v2.h"

#include "GameEntities/Agent.h"

float previous_set_x = 0;
float previous_set_y = 0;
float previous_set_z = 0;
uint32_t previous_set_z_plane = 0;
float previous_set_rotation = 0;

void AddCharacterPositionToRedisPipe(const GW::AgentLiving* character)
{
    const auto& position = character->pos;
    const auto key = std::format("client:{}:player:position",
                                 GWClientHub_v2::Instance().client_id);

    const auto& x = position.x;
    const auto& y = position.y;
    const auto& z = character->z;
    const auto& z_plane = character->plane;
    const auto& rotation = character->rotation_angle;

    if (previous_set_x != x || previous_set_y != y || previous_set_z != z || previous_set_z_plane != z_plane
        || previous_set_rotation != rotation)
    {
        character_position_changed = true;
    }
    if (character_position_changed)
    {
        // x-axis goes from west to south. y-axis from south to north.
        redis_pipe.hset(key, std::make_pair("x", std::to_string(x)));
        redis_pipe.hset(key, std::make_pair("y", std::to_string(y)));
        redis_pipe.hset(key, std::make_pair("z", std::to_string(z)));

        // Seems related to the layer number in the trapezoids. Maybe it is the same as the layer the character is standing on.
        redis_pipe.hset(key, std::make_pair("z_plane", std::to_string(z_plane)));

        // How fast is the character moving in each direction. Unit is inches/sec perhaps.
        redis_pipe.hset(key, std::make_pair("dx", std::to_string(character->move_x)));
        redis_pipe.hset(key, std::make_pair("dy", std::to_string(character->move_y)));

        // From -pi to pi. 0 at East. pi/2 at north. pi at west. -pi/2 at south.
        redis_pipe.hset(key, std::make_pair("rotation", std::to_string(rotation)));

        const auto changed_key = std::format("client:{}:changes", GWClientHub_v2::Instance().client_id);

        // We use a counter to keep track of how many times the character
        // position has changed. Other processes use this to know when to
        // update locally cached position.
        redis_pipe.hincrby(changed_key, "position_velocity_rotation", 1);

        previous_set_x = x;
        previous_set_y = y;
        previous_set_z = z;
        previous_set_z_plane = z_plane;
        previous_set_rotation = rotation;

        character_position_changed = false;
    }
}
