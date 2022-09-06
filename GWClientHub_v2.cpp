#include "pch.h"
#include "GWClientHub_v2.h"

#include "GWCA.h"
#include "SafeThreadEntry.h"
#include "SafeWndProc.h"
#include "SendGameDataToRedis.h"

#include "Managers/GameThreadMgr.h"
#include "Managers/MemoryMgr.h"

#include "Utilities/Hooker.h"

using namespace sw::redis;


void GWClientHub_v2::Init()
{
    // Set up our own NewWndHandle and store a copy of the default WndProc in DefaultWndProc
    // We can then later restore current_GW_window_handle to be the default when we terminate.
    current_GW_window_handle = GW::MemoryMgr::GetGWWindowHandle();
    DefaultWndProc = SetWindowLongPtrW(current_GW_window_handle, GWL_WNDPROC,
                                       reinterpret_cast<long>(SafeWndProc));

    // Connect to redis
    ConnectionOptions connection_options;
    connection_options.host = "127.0.0.1";
    connection_options.port = 6379;
    *redis = Redis(connection_options);

    // Register our Update method to be called on each frame from within the game thread.
    // Note that the game thread is separate from the current thread. It is the thread
    // controlled by the GW client.
    GW::GameThread::RegisterGameThreadCallback(&Update_Entry, Update);

    // Busy wait until this thread is ready to exit.
    while (!can_terminate)
        Sleep(1000);

    // Make sure all hooks are removed before be exit.
    while (GW::HookBase::GetInHookCount())
        Sleep(50);

    // In GWToolbox they sleep a bit here. We do the same to be safe without knowing why.
    Sleep(50);

    GW::Terminate();

    FreeLibraryAndExitThread(dll_module, EXIT_SUCCESS);
}

void GWClientHub_v2::Terminate()
{
    // Remove all hooks. Free all resources. Disconnect any connections to external processes.

    GW::GameThread::RemoveGameThreadCallback(&Update_Entry);

    can_terminate = true;
}

void GWClientHub_v2::Update(GW::HookStatus*)
{
    static DWORD last_tick_count;
    if (last_tick_count == 0)
        last_tick_count = GetTickCount();

    DWORD tick = GetTickCount();
    DWORD delta = tick - last_tick_count;
    float dt = delta / 1000.f;

    SendGameDataToRedis(dt);

    last_tick_count = tick;
}
