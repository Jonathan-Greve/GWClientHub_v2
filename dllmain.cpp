// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "GWCA/Include/GWCA/Utilities/Scanner.h"

DWORD APIENTRY init(HMODULE hModule)
{
    auto found = (DWORD**)GW::Scanner::Find(
                                            "\xA3\x00\x00\x00\x00\xFF\x75\x0C\xC7\x05",
                                            "x????xxxxx", +1);
    if (!(found && *found))
    {
        MessageBoxA(nullptr,
                    "We can't determine if the character is ingame.\nContact the "
                    "developers.",
                    "GWToolbox++ - Clientside Error Detected", 0);
        FreeLibraryAndExitThread(hModule, EXIT_SUCCESS);
    }

    DWORD* is_ingame = *found;
    while (*is_ingame == 0)
    {
        Sleep(100);
    }

    SafeThreadEntry(hModule);

    return 0;
}

void create_thread_for_bot(HMODULE hModule)
{
    // Create new thread to run GWCA in safely.
    const HANDLE hThread = CreateThread(nullptr, 0,
                                        reinterpret_cast<LPTHREAD_START_ROUTINE>(init),
        hModule, 0, nullptr);
    if (hThread != nullptr)
        CloseHandle(hThread);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
    DisableThreadLibraryCalls(hModule);
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        create_thread_for_bot(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    default: break;
    }
    return TRUE;
}
