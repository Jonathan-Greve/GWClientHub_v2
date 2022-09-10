#pragma once
namespace GW
{
    struct AgentLiving;
}

bool character_position_changed = false;

void AddCharacterPositionToRedisPipe(GW::AgentLiving* character);
