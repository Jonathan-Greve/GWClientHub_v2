#pragma once
namespace GW
{
    struct AgentLiving;
}

inline bool character_position_changed = false;

void AddCharacterPositionToRedisPipe(const GW::AgentLiving* character);
