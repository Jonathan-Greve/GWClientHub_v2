#pragma once
DWORD WINAPI safe_thread_entry(LPVOID mod);
DWORD WINAPI thread_entry(LPVOID);

LRESULT WINAPI safe_wnd_proc(HWND hWnd, UINT Message, WPARAM wParam,
    LPARAM lParam) noexcept;
LRESULT WINAPI WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

class GWClientHub_v2
{
};

