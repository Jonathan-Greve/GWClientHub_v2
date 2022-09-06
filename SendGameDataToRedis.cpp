#include "pch.h"
#include "SendGameDataToRedis.h"

#include "AddPlayerStateToRedisPipe.h"
#include "GWClientHub_v2.h"


/**
 * \brief Send game data to a redis server on localhost.
 * \param dt Time since last call in milliseconds.
 */
void SendGameDataToRedis(float dt)
{
    AddPlayerStateToRedisPipe(dt);
    redis_pipe.set(std::to_string(dt), std::to_string(dt));

    // Send all the data in the pipeline to the redis server
    redis_pipe.exec();
}
