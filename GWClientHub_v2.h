#pragma once

// Forwards declarations. The header files will be included in the .cpp file
// This speeds up compilation.
namespace GW
{
    struct HookStatus;
}


// Set to the default WindowProc in Init. This is the WindowProc the GW client creates when launched.
inline long DefaultWndProc = 0;

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
    void Update(GW::HookStatus*);
    void Terminate();

    bool can_terminate;
};
