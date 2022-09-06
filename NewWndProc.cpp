#include "pch.h"

#include "GWClientHub_v2.h"


LRESULT NewWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    // WM_CLOSE is sent as the message when the window is closed through pressing X
    // in the top right corner of the window or when pressing Alt-f4.
    if (Message == WM_CLOSE)
    {
        // We want to make sure that all resources acquired by our bot are closed.
        // before the close the window/application. So we defer closing and manually
        // resend the WM_CLOSE message when all resources are freed.
        GWClientHub_v2::Instance().Terminate();
        return 0;
    }

    // Call the initial/default WndProc that was set by the game.
    return CallWindowProc(reinterpret_cast<WNDPROC>(DefaultWndProc), hWnd, Message,
                          wParam, lParam);
}
