#pragma once
#include "Utilities/Hook.h"


// Forwards declarations. The header files will be included in the .cpp file
// This speeds up compilation.
namespace GW
{
    struct HookStatus;
}



// Set to the default WindowProc in Init. This is the WindowProc the GW client creates when launched.
// We store a copy so we can later restore current_GW_window_handle to the default WindowProc.
inline long DefaultWndProc = 0;
inline HWND current_GW_window_handle;

// Store the hook for the update method which is called once per frame.
inline GW::HookEntry Update_Entry;

// Store the redis class which maintainsa connection to the redis server.
// If the connection is broken it automatically reconnects.
inline sw::redis::Redis* redis;

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

    bool can_terminate;
};
