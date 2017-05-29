// WindowBase.hpp --- MZC4 window base
//////////////////////////////////////////////////////////////////////////////

#ifndef MZC4_WINDOW_BASE_HPP_
#define MZC4_WINDOW_BASE_HPP_    9   /* Version 9 */

#if _MSC_VER > 1000
    #pragma once
#endif

//////////////////////////////////////////////////////////////////////////////
// headers

// Win32 API headers
#ifndef _INC_WINDOWS
    #include <windows.h>    // Win32 API
#endif
#ifndef _INC_WINDOWSX
    #include <windowsx.h>   // Win32 Macro APIs
#endif
#ifndef _INC_COMMCTRL
    #include <commctrl.h>   // common controls
#endif
#ifndef _INC_TCHAR
    #include <tchar.h>      // generic text mappings
#endif

#include <dlgs.h>           // dialog control IDs

// standard C/C++ library
#include <string>   // std::string and std::wstring
#include <cassert>  // assert
#include <cstring>  // C string library

//////////////////////////////////////////////////////////////////////////////

#ifndef _countof
    #define _countof(array)     (sizeof(array) / sizeof(array[0]))
#endif

// tstring
#ifndef tstring
    #ifdef UNICODE
        typedef std::wstring    tstring;
    #else
        typedef std::string     tstring;
    #endif
#endif

// NOTE: Old Digital Mars C/C++ Compiler doesn't define INT_PTR type likely.
#ifdef __DMC__
    #ifndef INT_PTR
        #ifdef _WIN64
            #define INT_PTR     LPARAM
        #else
            #define INT_PTR     BOOL
        #endif
    #endif
#endif

struct WindowBase;
struct DialogBase;

//////////////////////////////////////////////////////////////////////////////
// public functions

#ifndef MZCAPI
    #define MZCAPI      WINAPI
#endif

#ifndef MZCAPIV
    #define MZCAPIV     WINAPIV
#endif

void MZCAPIV DebugPrintDx(const char *format, ...);
void MZCAPIV DebugPrintDx(const WCHAR *format, ...);
void MZCAPI GetVirtualScreenRectDx(LPRECT prc);
void MZCAPI RepositionPointDx(LPPOINT ppt, SIZE siz, LPCRECT prc);
void MZCAPI WorkAreaFromWindowDx(LPRECT prcWorkArea, HWND hwnd);
SIZE MZCAPI SizeFromRectDx(LPCRECT prc);
LPTSTR MZCAPI LoadStringDx(INT nID);
LPTSTR MZCAPI LoadStringDx2(INT nID);
LPCTSTR MZCAPI GetStringDx(INT nStringID);
LPCTSTR MZCAPI GetStringDx2(INT nStringID);
LPCTSTR MZCAPI GetStringDx(LPCTSTR psz);
LPCTSTR MZCAPI GetStringDx2(LPCTSTR psz);

//////////////////////////////////////////////////////////////////////////////

struct WindowBase
{
    HWND m_hwnd;
    WNDPROC m_fnOldProc;

    WindowBase() : m_hwnd(NULL), m_fnOldProc(NULL)
    {
    }

    virtual ~WindowBase()
    {
    }

    operator HWND() const
    {
        return m_hwnd;
    }
    operator bool() const
    {
        return m_hwnd != NULL;
    }

    static WindowBase *GetUserData(HWND hwnd)
    {
        return (WindowBase *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    static void SetUserData(HWND hwnd, void *ptr)
    {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)ptr);
    }

    WindowBase *GetUserData() const
    {
        return GetUserData(m_hwnd);
    }

    LRESULT CALLBACK
    CallWindowProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return ::CallWindowProc(m_fnOldProc, hwnd, uMsg, wParam, lParam);
    }

    virtual LRESULT MZCAPI
    DefaultProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (m_fnOldProc)
        {
            return ::CallWindowProc(m_fnOldProc, hwnd, uMsg, wParam, lParam);
        }
        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    virtual LRESULT CALLBACK
    WindowProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return DefaultProcDx(hwnd, uMsg, wParam, lParam);
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
            base->m_hwnd = hwnd;
            SetUserData(hwnd, base);
        }
        else
        {
            base = GetUserData(hwnd);
        }

        if (base)
            return base->WindowProcDx(hwnd, uMsg, wParam, lParam);

        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    virtual LPCTSTR GetWndClassNameDx() const
    {
        return TEXT("katahiromz's WindowBase Class");
    }

    virtual void ModifyWndClassDx(WNDCLASSEX& wcx)
    {
    }

    BOOL RegisterClassDx()
    {
        if (m_hwnd)
            return ::GetClassWord(m_hwnd, GCW_ATOM);

        HMODULE hMod = ::GetModuleHandle(NULL);

        WNDCLASSEX wcx;
        ZeroMemory(&wcx, sizeof(wcx));
        LPCTSTR pszClass = GetWndClassNameDx();
        if (GetClassInfoEx(hMod, pszClass, &wcx))
            return TRUE;

        wcx.cbSize = sizeof(wcx);
        wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wcx.lpfnWndProc = WindowBase::WindowProc;
        wcx.hInstance = hMod;
        wcx.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
        wcx.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wcx.hbrBackground = ::GetSysColorBrush(COLOR_3DFACE);
        wcx.lpszMenuName = NULL;
        wcx.lpszClassName = pszClass;
        wcx.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);

        ModifyWndClassDx(wcx);

        return ::RegisterClassEx(&wcx);
    }

    BOOL CreateWindowDx(HWND hwndParent, LPCTSTR pszText,
                        DWORD Style = WS_OVERLAPPEDWINDOW, DWORD ExStyle = 0,
                        INT x = CW_USEDEFAULT, INT y = CW_USEDEFAULT,
                        INT cx = CW_USEDEFAULT, INT cy = CW_USEDEFAULT,
                        HMENU hMenu = NULL)
    {
        if (!RegisterClassDx())
            return FALSE;

        ::CreateWindowEx(ExStyle, GetWndClassNameDx(),
                         GetStringDx(pszText),
                         Style, x, y, cx, cy, hwndParent,
                         hMenu, GetModuleHandle(NULL), this);
        return (m_hwnd != NULL);
    }

    void SubclassDx(HWND hwnd)
    {
        m_hwnd = hwnd;
        SetUserData(hwnd, this);
        m_fnOldProc = SubclassWindow(hwnd, WindowBase::WindowProc);
    }

    void UnsubclassDx(HWND hwnd)
    {
        m_hwnd = NULL;
        SetUserData(hwnd, NULL);
        SubclassWindow(hwnd, m_fnOldProc);
        m_fnOldProc = NULL;
    }

    INT MsgBoxDx(LPCTSTR pszString, LPCTSTR pszTitle,
                 UINT uType = MB_ICONINFORMATION)
    {
        tstring Title;
        if (pszTitle == NULL)
        {
#ifdef IDS_APPNAME
            Title = LoadStringDx(IDS_APPNAME);
#else
            if (m_hwnd)
                Title = GetWindowTextDx();
            else
                Title = TEXT("ERROR");
#endif
        }
        else
        {
            Title = GetStringDx(pszTitle);
        }

        WindowBase::_doHookCenterMsgBoxDx(TRUE);
        INT nID = ::MessageBox(m_hwnd, GetStringDx(pszString),
                               Title.c_str(), uType);
        WindowBase::_doHookCenterMsgBoxDx(FALSE);

        return nID;
    }

    INT MsgBoxDx(UINT nStringID, UINT nTitleID, UINT uType = MB_ICONINFORMATION)
    {
        return MsgBoxDx(MAKEINTRESOURCE(nStringID), MAKEINTRESOURCE(nTitleID), uType);
    }

    INT MsgBoxDx(UINT nStringID, LPCTSTR pszTitle, UINT uType = MB_ICONINFORMATION)
    {
        return MsgBoxDx(MAKEINTRESOURCE(nStringID), pszTitle, uType);
    }

    INT MsgBoxDx(UINT nStringID, UINT uType = MB_ICONINFORMATION)
    {
        return MsgBoxDx(MAKEINTRESOURCE(nStringID), NULL, uType);
    }

    INT MsgBoxDx(LPCTSTR pszString, UINT uType = MB_ICONINFORMATION)
    {
        return MsgBoxDx(pszString, NULL, uType);
    }

    VOID CenterWindowDx() const
    {
        CenterWindowDx(m_hwnd);
    }

    static tstring GetWindowTextDx(HWND hwnd)
    {
        INT cch = ::GetWindowTextLength(hwnd);
        tstring ret;
        ret.resize(cch);
        ::GetWindowText(hwnd, &ret[0], cch + 1);
        return ret;
    }
    tstring GetWindowTextDx() const
    {
        return GetWindowTextDx(m_hwnd);
    }

    static tstring GetDlgItemTextDx(HWND hwnd, INT nCtrlID)
    {
        return GetWindowTextDx(::GetDlgItem(hwnd, nCtrlID));
    }
    tstring GetDlgItemTextDx(INT nCtrlID) const
    {
        return GetWindowTextDx(::GetDlgItem(m_hwnd, nCtrlID));
    }

    static void CenterWindowDx(HWND hwnd)
    {
        assert(IsWindow(hwnd));

        BOOL bChild = !!(GetWindowStyle(hwnd) & WS_CHILD);

        // get parent
        HWND hwndParent;
        if (bChild)
            hwndParent = GetParent(hwnd);
        else
            hwndParent = GetWindow(hwnd, GW_OWNER);

        RECT rcWorkArea;
        WorkAreaFromWindowDx(&rcWorkArea, hwnd);

        RECT rcParent;
        if (hwndParent)
            GetWindowRect(hwndParent, &rcParent);
        else
            rcParent = rcWorkArea;

        SIZE sizParent = SizeFromRectDx(&rcParent);

        RECT rc;
        GetWindowRect(hwnd, &rc);
        SIZE siz = SizeFromRectDx(&rc);

        POINT pt;
        pt.x = rcParent.left + (sizParent.cx - siz.cx) / 2;
        pt.y = rcParent.top + (sizParent.cy - siz.cy) / 2;

        if (bChild && hwndParent)
        {
            GetClientRect(hwndParent, &rcParent);
            MapWindowRect(hwndParent, NULL, &rcParent);
            RepositionPointDx(&pt, siz, &rcParent);

            ScreenToClient(hwndParent, &pt);
        }
        else
        {
            RepositionPointDx(&pt, siz, &rcWorkArea);

            #if 0
                RECT rcScreen;
                GetVirtualScreenRectDx(&rcScreen);
                RepositionPointDx(&pt, siz, &rcScreen);
            #endif
        }

        SetWindowPos(hwnd, NULL, pt.x, pt.y, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

private:
    static inline LRESULT CALLBACK
    _msgBoxCbtProcDx(INT nCode, WPARAM wParam, LPARAM lParam)
    {
#ifndef MZC_NO_CENTER_MSGBOX
        if (nCode == HCBT_ACTIVATE)
        {
            HWND hwnd = (HWND)wParam;
            TCHAR szClassName[8];
            ::GetClassName(hwnd, szClassName, _countof(szClassName));
            if (lstrcmpi(szClassName, TEXT("#32770")) == 0)
            {
                CenterWindowDx(hwnd);
            }
        }
#endif  // ndef MZC_NO_CENTER_MSGBOX

        return 0;   // allow the operation
    }

    static HHOOK _doHookCenterMsgBoxDx(BOOL bHook)
    {
#ifdef MZC_NO_CENTER_MSGBOX
        return NULL;
#else   // ndef MZC_NO_CENTER_MSGBOX
        static HHOOK s_hHook = NULL;
        if (bHook)
        {
            if (s_hHook == NULL)
            {
                DWORD dwThreadID = GetCurrentThreadId();
                s_hHook = ::SetWindowsHookEx(WH_CBT, _msgBoxCbtProcDx, NULL, dwThreadID);
            }
        }
        else
        {
            if (s_hHook)
            {
                if (::UnhookWindowsHookEx(s_hHook))
                {
                    s_hHook = NULL;
                }
            }
        }
        return s_hHook;
#endif  // ndef MZC_NO_CENTER_MSGBOX
    }

#ifdef MZC_FAT_AND_RICH
public:
    #include "WindowBaseRich.hpp"
#endif
};

//////////////////////////////////////////////////////////////////////////////

struct DialogBase : public WindowBase
{
    virtual LRESULT MZCAPI
    DefaultProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return 0;
    }

    virtual INT_PTR CALLBACK
    DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return 0;
    }

    static INT_PTR CALLBACK
    DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        DialogBase *base;
        if (uMsg == WM_INITDIALOG)
        {
            assert(lParam);
            base = (DialogBase *)lParam;
            base->m_hwnd = hwnd;
            SetUserData(hwnd, base);
        }
        else
        {
            base = GetUserData(hwnd);
        }

        if (base)
        {
            return base->DialogProcDx(hwnd, uMsg, wParam, lParam);
        }

        return 0;
    }

    static DialogBase *GetUserData(HWND hwnd)
    {
        return (DialogBase *)GetWindowLongPtr(hwnd, DWLP_USER);
    }
    static void SetUserData(HWND hwnd, void *ptr)
    {
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)ptr);
    }

    BOOL CreateDialogDx(HWND hwndParent, INT nDialogID)
    {
        ::CreateDialogParam(::GetModuleHandle(NULL),
            MAKEINTRESOURCE(nDialogID), hwndParent, DialogBase::DialogProc,
            (LPARAM)this);
        return (m_hwnd != NULL);
    }

    INT_PTR DialogBoxDx(HWND hwndParent, INT nDialogID)
    {
        INT_PTR nID = ::DialogBoxParam(::GetModuleHandle(NULL),
            MAKEINTRESOURCE(nDialogID), hwndParent,
            DialogBase::DialogProc, (LPARAM)this);
        return nID;
    }

    virtual LPCTSTR GetWndClassNameDx() const
    {
        return TEXT("#32770");
    }

#ifdef MZC_FAT_AND_RICH
public:
    #include "DialogBaseRich.hpp"
#endif
};

//////////////////////////////////////////////////////////////////////////////
// public inline functions

inline void MZCAPIV DebugPrintDx(const char *format, ...)
{
    #ifdef _DEBUG
        char buffer[512];
        va_list va;
        va_start(va, format);
        ::wvsprintfA(buffer, format, va);
        va_end(va);
        OutputDebugStringA(buffer);
    #endif
}

inline void MZCAPIV DebugPrintDx(const WCHAR *format, ...)
{
    #ifdef _DEBUG
        WCHAR buffer[512];
        va_list va;
        va_start(va, format);
        ::wvsprintfW(buffer, format, va);
        va_end(va);
        OutputDebugStringW(buffer);
    #endif
}

inline void MZCAPI GetVirtualScreenRectDx(LPRECT prc)
{
    INT x = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
    INT y = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
    INT cx = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
    INT cy = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (cx == 0)
        cx = ::GetSystemMetrics(SM_CXSCREEN);
    if (cy == 0)
        cy = ::GetSystemMetrics(SM_CYSCREEN);
    SetRect(prc, x, y, x + cx, y + cy);
}

inline void MZCAPI RepositionPointDx(LPPOINT ppt, SIZE siz, LPCRECT prc)
{
    if (ppt->x + siz.cx > prc->right)
        ppt->x = prc->right - siz.cx;
    if (ppt->y + siz.cy > prc->bottom)
        ppt->y = prc->bottom - siz.cy;
    if (ppt->x < prc->left)
        ppt->x = prc->left;
    if (ppt->y < prc->top)
        ppt->y = prc->top;
}

inline void MZCAPI WorkAreaFromWindowDx(LPRECT prcWorkArea, HWND hwnd)
{
#if (WINVER >= 0x0500)
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (GetMonitorInfo(hMonitor, &mi))
    {
        *prcWorkArea = mi.rcWork;
        return;
    }
#endif
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, prcWorkArea, 0);
}

inline SIZE MZCAPI SizeFromRectDx(LPCRECT prc)
{
    SIZE siz = { prc->right - prc->left, prc->bottom - prc->top };
    return siz;
}

inline LPTSTR MZCAPI LoadStringDx(INT nID)
{
    static TCHAR s_sz[1024];
    s_sz[0] = 0;
    ::LoadString(::GetModuleHandle(NULL), nID, s_sz, _countof(s_sz));
    return s_sz;
}

inline LPTSTR MZCAPI LoadStringDx2(INT nID)
{
    static TCHAR s_sz[1024];
    s_sz[0] = 0;
    ::LoadString(::GetModuleHandle(NULL), nID, s_sz, _countof(s_sz));
    return s_sz;
}

inline LPCTSTR MZCAPI GetStringDx(LPCTSTR psz)
{
    if (psz == NULL)
        return NULL;
    if (IS_INTRESOURCE(psz))
        return LoadStringDx(LOWORD(psz));
    return psz;
}

inline LPCTSTR MZCAPI GetStringDx2(LPCTSTR psz)
{
    if (psz == NULL)
        return NULL;
    if (IS_INTRESOURCE(psz))
        return LoadStringDx2(LOWORD(psz));
    return psz;
}

inline LPCTSTR MZCAPI GetStringDx(INT nStringID)
{
    return GetStringDx(MAKEINTRESOURCE(nStringID));
}

inline LPCTSTR MZCAPI GetStringDx2(INT nStringID)
{
    return GetStringDx2(MAKEINTRESOURCE(nStringID));
}

//////////////////////////////////////////////////////////////////////////////

#ifdef MZC_FAT_AND_RICH
    #include "Button.hpp"
    #include "ComboBox.hpp"
    #include "Edit.hpp"
    #include "ListBox.hpp"
    #include "ScrollBar.hpp"
    #include "Static.hpp"
    #include "CommCtrl.hpp"
    #include "CommDlg.hpp"
#endif

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef MZC4_WINDOW_BASE_HPP_

//////////////////////////////////////////////////////////////////////////////
