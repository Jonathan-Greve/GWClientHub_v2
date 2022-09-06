#include "pch.h"
#include "GWClientHub_v2.h"

#include "GWCA.h"
#include "SafeThreadEntry.h"
#include "SafeWndProc.h"
#include "SendGameDataToRedis.h"

#include "Context/CharContext.h"

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

    // Get client email
    std::wstring email_wchar = GW::CharContext::instance()->player_email;
    std::string email{email_wchar.begin(), email_wchar.end()};

    // Get a unique id for the client.
    auto existing_client_id =
        redis.get(std::format("client:{}:id", email));
    if (existing_client_id)
        client_id = std::stoi(existing_client_id.value());
    else {
        client_id = redis.incr("client_id_counter");
        redis.set(std::format("client:{}:id", email), std::to_string(client_id));
    }

    // Register our Update method to be called on each frame from within the game thread.
    // Note that the game thread is separate from the current thread. It is the thread
    // controlled by the GW client.
    GW::GameThread::RegisterGameThreadCallback(&Update_Entry, Update);
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
