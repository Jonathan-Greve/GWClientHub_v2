#include "pch.h"
#include "GWClientHub_v2.h"

#include "KeyboardProc.h"
#include "SafeWndProc.h"
#include "SendGameDataToRedis.h"

#include "Context/CharContext.h"

#include "Managers/GameThreadMgr.h"
#include "Managers/MemoryMgr.h"

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

    // Get a unique id for the client. We could use email. But a small integer key should
    // be slightly faster.
    if (auto existing_client_id = redis.hget("clients:email_to_client_id", email))
        client_id = existing_client_id.value();
    else
    {
        client_id = std::to_string(redis.incr("client_id_counter"));
        redis.hset("clients:email_to_client_id",
                   {email, client_id});
    }

    // Register our Update method to be called on each frame from within the game thread.
    // Note that the game thread is separate from the current thread. It is the thread
    // controlled by the GW client.
    GW::GameThread::RegisterGameThreadCallback(&Update_Entry, Update);

    // Set our dll to
    keyboard_hook_handle = SetWindowsHookExA(WH_KEYBOARD, &KeyboardProc, nullptr,
                                             GetCurrentThreadId());
}

// Remove all hooks. Free all resources. Disconnect any connections to external processes.
void GWClientHub_v2::Terminate()
{
    GW::GameThread::RemoveGameThreadCallback(&Update_Entry);

    UnhookWindowsHookEx(keyboard_hook_handle);


    // Restore the window handle to be the default one that GW launched with.
    SetWindowLongPtr(current_GW_window_handle, GWL_WNDPROC, DefaultWndProc);

    // Let ThreadEntry know that it can finish terminating our dll thread.
    has_freed_resources = true;

    // If terminate was called because the window is closing (i.e. Alt-f4 or pressed close)
    // Then resend the WM_CLOSE signal that we intercepted earlier in NewWndProc.
    if (GW_is_closing)
    {
        SendMessageW(current_GW_window_handle, WM_CLOSE, NULL, NULL);
    }
}

void GWClientHub_v2::Update(GW::HookStatus*)
{
    static DWORD last_tick_count;
    if (last_tick_count == 0)
        last_tick_count = GetTickCount();

    const DWORD tick = GetTickCount();
    const DWORD delta = tick - last_tick_count;
    const float dt = static_cast<float>(delta) / 1000.f;

    SendGameDataToRedis(dt);

    last_tick_count = tick;
}
