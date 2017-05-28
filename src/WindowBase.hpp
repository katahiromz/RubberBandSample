// WindowBase.hpp --- MZC4 window base
//////////////////////////////////////////////////////////////////////////////

#ifndef WINDOW_BASE_HPP_
#define WINDOW_BASE_HPP_    6   /* Version 6 */

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

// standard C library
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
// DebugPrintDx --- debug output

inline void DebugPrintDx(const char *format, ...)
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

inline void DebugPrintDx(const WCHAR *format, ...)
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

    virtual LRESULT DefaultProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

        m_hwnd = ::CreateWindowEx(ExStyle, GetWndClassNameDx(), GetStringDx(pszText),
                                  Style, x, y, cx, cy, hwndParent, hMenu,
                                  GetModuleHandle(NULL), this);
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
        SetUserData(hwnd, NULL);
        SubclassWindow(hwnd, m_fnOldProc);
        m_fnOldProc = NULL;
        m_hwnd = NULL;
    }

    // WARNING: This function is not thread-safe!
    LPTSTR LoadStringDx(UINT nID) const
    {
        static TCHAR s_sz[1024];
        s_sz[0] = 0;
        ::LoadString(::GetModuleHandle(NULL), nID, s_sz, _countof(s_sz));
        return s_sz;
    }
    LPTSTR LoadStringDx2(UINT nID) const
    {
        static TCHAR s_sz[1024];
        s_sz[0] = 0;
        ::LoadString(::GetModuleHandle(NULL), nID, s_sz, _countof(s_sz));
        return s_sz;
    }

    LPCTSTR GetStringDx(LPCTSTR psz) const
    {
        if (psz == NULL)
            return NULL;
        if (IS_INTRESOURCE(psz))
            return LoadStringDx(LOWORD(psz));
        return psz;
    }
    LPCTSTR GetStringDx2(LPCTSTR psz) const
    {
        if (psz == NULL)
            return NULL;
        if (IS_INTRESOURCE(psz))
            return LoadStringDx2(LOWORD(psz));
        return psz;
    }

    LPCTSTR GetStringDx(INT nStringID) const
    {
        return GetStringDx(MAKEINTRESOURCE(nStringID));
    }
    LPCTSTR GetStringDx2(INT nStringID) const
    {
        return GetStringDx2(MAKEINTRESOURCE(nStringID));
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
        INT nID = ::MessageBox(m_hwnd, GetStringDx2(pszString),
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

    static VOID CenterWindowDx(HWND hwnd);
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

private:
    static inline LRESULT CALLBACK
    _msgBoxCbtProcDx(INT nCode, WPARAM wParam, LPARAM lParam)
    {
#ifndef NO_CENTER_MSGBOX
        if (nCode == HCBT_ACTIVATE)
        {
            HWND hwnd = (HWND)wParam;

            TCHAR szClassName[16];
            ::GetClassName(hwnd, szClassName, 16);
            if (lstrcmpi(szClassName, TEXT("#32770")) == 0)
            {
                WindowBase::CenterWindowDx(hwnd);
            }
        }
#endif  // ndef NO_CENTER_MSGBOX

        return 0;   // allow the operation
    }

    static HHOOK _doHookCenterMsgBoxDx(BOOL bHook)
    {
#ifdef NO_CENTER_MSGBOX
        return NULL;
#else   // ndef NO_CENTER_MSGBOX
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
#endif  // ndef NO_CENTER_MSGBOX
    }
};

//////////////////////////////////////////////////////////////////////////////

struct DialogBase : public WindowBase
{
    virtual LRESULT
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
            SetUserData(hwnd, base);
        }
        else
        {
            base = GetUserData(hwnd);
        }

        if (base)
            return base->DialogProcDx(hwnd, uMsg, wParam, lParam);

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
        m_hwnd = ::CreateDialogParam(::GetModuleHandle(NULL),
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
};

//////////////////////////////////////////////////////////////////////////////

/*static*/ inline VOID
WindowBase::CenterWindowDx(HWND hwnd)
{
    BOOL bChild = !!(GetWindowStyle(hwnd) & WS_CHILD);

    // get parent
    HWND hwndOwner;
    hwndOwner = (bChild ? ::GetParent(hwnd) : ::GetWindow(hwnd, GW_OWNER));

    RECT rc, rcOwner, rcWorkArea;

    // get work area
#if (WINVER >= 0x0500)
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (GetMonitorInfo(hMonitor, &mi))
    {
        rcWorkArea = mi.rcWork;
    }
    else
    {
        ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
    }
#else
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
#endif

    if (hwndOwner)
    {
        ::GetWindowRect(hwndOwner, &rcOwner);
    }
    else
    {
        rcOwner = rcWorkArea;
    }

    // calculate the position
    POINT pt;
    ::GetWindowRect(hwnd, &rc);
    SIZE siz = { rc.right - rc.left, rc.bottom - rc.top };
    pt.x = rcOwner.left + ((rcOwner.right - rcOwner.left) - siz.cx) / 2;
    pt.y = rcOwner.top + ((rcOwner.bottom - rcOwner.top) - siz.cy) / 2;

    // fit to work area
    if (pt.x + siz.cx > rcWorkArea.right)
        pt.x = rcWorkArea.right - siz.cx;
    if (pt.y + siz.cy > rcWorkArea.bottom)
        pt.y = rcWorkArea.bottom - siz.cy;
    if (pt.x < rcWorkArea.left)
        pt.x = rcWorkArea.left;
    if (pt.y < rcWorkArea.top)
        pt.y = rcWorkArea.top;

    // fit to virtual screen
    INT xScreen = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
    INT yScreen = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
    INT cxScreen = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
    INT cyScreen = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (cxScreen == 0)
        cxScreen = ::GetSystemMetrics(SM_CXSCREEN);
    if (cyScreen == 0)
        cyScreen = ::GetSystemMetrics(SM_CYSCREEN);
    if (pt.x + siz.cx > xScreen + cxScreen)
        pt.x = (xScreen + cxScreen) - siz.cx;
    if (pt.y + siz.cy > yScreen + cyScreen)
        pt.y = (yScreen + cyScreen) - siz.cy;
    if (pt.x < xScreen)
        pt.x = xScreen;
    if (pt.y < yScreen)
        pt.y = yScreen;

    if (bChild && hwndOwner)
        ::ScreenToClient(hwndOwner, &pt);

    // move it
    ::SetWindowPos(hwnd, NULL, pt.x, pt.y, 0, 0,
                   SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef WINDOW_BASE_HPP_

//////////////////////////////////////////////////////////////////////////////
