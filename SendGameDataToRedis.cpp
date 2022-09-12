#include "pch.h"
#include "SendGameDataToRedis.h"

#include "AddMapBoundariesToRedisPipe.h"
#include "AddPathingMapToRedisPipe.h"
#include "AddPlayerStateToRedisPipe.h"
#include "GWClientHub_v2.h"


/**
 * \brief Send game data to a redis server on localhost.
 * \param dt Time since last call in milliseconds.
 */
void SendGameDataToRedis(const float dt)
{
    AddCharacterStateToRedisPipe(dt);
    AddMapTrapezoidsToRedisPipe(dt);
    AddMapBoundariesToRedisPipe(dt);

    // Send all the data in the pipeline to the redis server
    redis_pipe.exec();
}
