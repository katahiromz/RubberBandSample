#ifndef WINDOW_BASE_HPP_
#define WINDOW_BASE_HPP_

#ifndef _INC_WINDOWS
    #include <windows.h>
#endif

#include <dlgs.h>
#include <cassert>

struct WindowBase
{
    HWND m_hwnd;
    WNDPROC m_fnOldProc;

    WindowBase() : m_hwnd(NULL), m_fnOldProc(NULL)
    {
    }

    LRESULT CALLBACK
    CallWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return ::CallWindowProc(m_fnOldProc, hwnd, uMsg, wParam, lParam);
    }

    LRESULT DefaultProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (m_fnOldProc)
        {
            return ::CallWindowProc(m_fnOldProc, hwnd, uMsg, wParam, lParam);
        }
        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    virtual LRESULT CALLBACK
    WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return DefaultProc(hwnd, uMsg, wParam, lParam);
    }

    static LRESULT CALLBACK
    WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WindowBase *base;
        if (uMsg == WM_CREATE)
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
            assert(pcs->lpCreateParams);
            base = (WindowBase *)pcs->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)base);
        }
        else
        {
            base = (WindowBase *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        if (base)
            return base->WndProc(hwnd, uMsg, wParam, lParam);

        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    virtual INT_PTR CALLBACK
    DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return 0;
    }

    static INT_PTR CALLBACK
    DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WindowBase *base;
        if (uMsg == WM_INITDIALOG)
        {
            assert(lParam);
            base = (WindowBase *)lParam;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)base);
        }
        else
        {
            base = (WindowBase *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        if (base)
            return base->DlgProc(hwnd, uMsg, wParam, lParam);

        return 0;
    }

    // debug output
    void debug_print(const TCHAR *format, ...) const
    {
#ifdef _DEBUG
        TCHAR buffer[512];
        va_list va;
        va_start(va, format);
        ::wvsprintf(buffer, format, va);
        va_end(va);
        OutputDebugString(buffer);
#endif
    }

    void Subclass(HWND hwnd)
    {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
        m_fnOldProc = SubclassWindow(hwnd, WindowBase::WindowProc);
    }

    void Unsubclass(HWND hwnd)
    {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        SubclassWindow(hwnd, m_fnOldProc);
        m_fnOldProc = NULL;
    }
};

#endif  // ndef WINDOW_BASE_HPP_
