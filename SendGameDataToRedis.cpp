#include "pch.h"
#include "SendGameDataToRedis.h"

#include "AddCurrentMapInfoToRedisPipe.h"
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
    // Functions where the data can change on every call.
    AddCharacterStateToRedisPipe(dt);
    AddCurrentMapInfoToRedisPipe();

    // Functions which will send the same data once at most.
    AddMapTrapezoidsToRedisPipe(dt);
    AddMapBoundariesToRedisPipe(dt);
    AddMapGridPointsToRedisPipe(dt);

    // Send all the data in the pipeline to the redis server
    redis_pipe.exec();
}
