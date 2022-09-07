#pragma once
#include "Utilities/Hook.h"


// Forwards declarations. The header files will be included in the .cpp file
// This speeds up compilation.
namespace GW
{
    struct HookStatus;
}

using namespace sw::redis;


// Set to the default WindowProc in Init. This is the WindowProc the GW client creates when launched.
// We store a copy so we can later restore current_GW_window_handle to the default WindowProc.
inline long DefaultWndProc = 0;
inline HWND current_GW_window_handle;

// Handle to keyboard hook. We use this when intercepting 'end' key press to terminate our dll.
inline HHOOK keyboard_hook_handle;

// Store the hook for the update method which is called once per frame.
inline GW::HookEntry Update_Entry;

// Connect to redis
inline Redis redis = Redis("tcp://127.0.0.1:6379");
// And also create a redis pipe on a separate connection.
// I.e. the variable 'redis' uses a separate connection to the redis server
// than the 'redis_pipe' variable. So even if one is blocking the other can
// be used. But since we are operating from inside the game thread we will
// try to avoid any blocking operations.
inline Pipeline redis_pipe = redis.pipeline(true);

class GWClientHub_v2
{
    GWClientHub_v2() = default;

public:
    // Delete copy constructor and operator
    GWClientHub_v2(const GWClientHub_v2&) = delete;
    void operator=(const GWClientHub_v2&) = delete;

    static GWClientHub_v2& Instance()
    {
        static GWClientHub_v2 instance;
        return instance;
    }

    void Init();
    void Terminate();

    // Must be static because it is called from a hooked function.
    static void Update(GW::HookStatus*);

    bool GW_is_closing = false;
    bool has_freed_resources = false;

    uint32_t client_id;
};
