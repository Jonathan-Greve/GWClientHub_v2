﻿#include "pch.h"

#include "GWCA.h"
#include "SafeThreadEntry.h"

#include "Managers/GameThreadMgr.h"
#include "Utilities/Hooker.h"

#include "GWClientHub_v2.h"

DWORD __stdcall ThreadEntry(LPVOID)
{
    GW::HookBase::Initialize();
    if (!GW::Initialize())
    {
        if (MessageBoxA(nullptr,
                        "Initialize Failed at finding all addresses, contact Developers about this.",
                        "GWToolbox++ API Error", 0) == IDOK)
        {
        }
        goto leave;
    }

    GW::HookBase::EnableHooks();
    GW::GameThread::Enqueue([]()
    {
            GWClientHub_v2::Instance().Init();
    });

    while (!GWClientHub_v2::Instance().can_terminate)
    {
        // wait until destruction
        Sleep(100);
    }


    // @Remark:
    // Hooks are disable from Guild Wars thread (safely), so we just make sure we exit the last hooks
    while (GW::HookBase::GetInHookCount())
        Sleep(16);

    // @Remark:
    // We can't guarantee that the code in Guild Wars thread isn't still in the trampoline, but
    // practically a short sleep is fine.
    Sleep(16);
leave:
    GW::Terminate();


    FreeLibraryAndExitThread(dll_module, EXIT_SUCCESS);
}
