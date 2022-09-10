#pragma once
namespace GW
{
    struct AgentLiving;
}

inline bool character_position_changed = false;

void AddCharacterPositionToRedisPipe(GW::AgentLiving* character);
