#include "pch.h"

#include <Context/PreGameContext.h>

LRESULT NewWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    // WM_CLOSE is sent as the message when the window is closed through pressing X
    // in the top right corner of the window or when pressing Alt-f4.
    if (Message == WM_CLOSE)
    {
        // We want to make sure that all resources acquired by our bot are closed.
        // before the close the window/application. So we defer closing and manually
        // resend the WM_CLOSE message when all resources are freed.
        GwClientHub_v2::Instance().StartSelfDestruct();
        return 0;
    }

    if (!(!GW::PreGameContext::instance() && imgui_initialized &&
        gw_client_hub_initialized && !gw_client_hub_destroyed))
    {
        return CallWindowProc(static_cast<WNDPROC>(OldWndProc), hWnd, Message,
                              wParam, lParam);
    }

    return CallWindowProc(static_cast<WNDPROC>(OldWndProc), hWnd, Message,
                          wParam, lParam);
}
