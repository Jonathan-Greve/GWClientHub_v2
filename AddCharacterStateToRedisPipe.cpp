#include "pch.h"

#include "AddCharacterPositionToRedisPipe.h"
#include "AddPlayerStateToRedisPipe.h"
#include "GWClientHub_v2.h"

#include "Context/AgentContext.h"

#include "GameEntities/Agent.h"

#include "Managers/AgentMgr.h"


void AddCharacterStateToRedisPipe(float dt)
{
    if (const auto character = GW::Agents::GetCharacter())
    {
        AddCharacterPositionToRedisPipe(character);
        if (auto agent_context = GW::AgentContext::instance())
        {
            if (auto character_agent_movement = agent_context->agent_movement[character->agent_id])
            {
                auto& moving1 = character_agent_movement->moving1;
                const auto key =
                    std::format("client:{}:player:position", GWClientHub_v2::Instance().client_id);
                redis_pipe.hset(key,
                                std::make_pair("is_stuck", std::to_string(moving1)));
            }
        }
    }
}
