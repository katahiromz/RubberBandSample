// RubberBandSample.cpp --- A Win32 application                -*- C++ -*-
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////

// the class name of the main window
static const TCHAR  s_szClassName[] = TEXT(VERSION_INFO_STRING);
static const TCHAR  s_szRubberBandClass[] = TEXT("katahiromz's Rubber Band Class");

//////////////////////////////////////////////////////////////////////////////

// the Win32 application
struct WinApp : public WindowBase
{
    INT         m_argc;         // number of command line parameters
    TCHAR **    m_targv;        // command line parameters

    HINSTANCE   m_hInst;        // the instance handle
    HICON       m_hIcon;        // the icon handle
    HACCEL      m_hAccel;       // the accelerator handle

    RubberBand  m_rubber_band;

    // constructors
    WinApp(int argc, TCHAR **targv) :
        m_argc(argc),
        m_targv(targv),
        m_hInst(::GetModuleHandleA(NULL)),
        m_hIcon(NULL),
        m_hAccel(NULL)
    {
    }

    WinApp(int argc, TCHAR **targv, HINSTANCE hInst) :
        m_argc(argc),
        m_targv(targv),
        m_hInst(hInst),
        m_hIcon(NULL),
        m_hAccel(NULL)
    {
    }

    // register window classes
    BOOL RegisterClassesDx();

    virtual void ModifyWndClassDx(WNDCLASSEX& wcx)
    {
        WindowBase::ModifyWndClassDx(wcx);
        wcx.lpszMenuName = MAKEINTRESOURCE(1);
    }

    // start up
    BOOL StartDx(INT nCmdShow)
    {
        // load accessories
        m_hIcon = ::LoadIcon(m_hInst, MAKEINTRESOURCE(1));
        m_hAccel = ::LoadAccelerators(m_hInst, MAKEINTRESOURCE(1));

        if (!CreateWindowDx(NULL, LoadStringDx(1),
                            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN))
        {
            MsgBoxDx(TEXT("failure of CreateWindow"), NULL, MB_ICONERROR);
            return FALSE;
        }

        // show the window
        ::ShowWindow(*this, nCmdShow);
        ::UpdateWindow(*this);

        return TRUE;
    }

    // message loop
    INT_PTR RunDx()
    {
        MSG msg;
        while (::GetMessage(&msg, NULL, 0, 0))
        {
            if (!::TranslateAccelerator(m_hwnd, m_hAccel, &msg))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }
        return INT(msg.wParam);
    }

    // WM_CREATE
    BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
    {
        m_hwnd = hwnd;
        ::DragAcceptFiles(m_hwnd, TRUE);

        m_rubber_band.m_hwndTarget = ::CreateWindow(
            TEXT("BUTTON"), TEXT("Test Me!"),
            BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
            50, 50, 150, 150, hwnd, NULL, m_hInst, NULL);
        if (m_rubber_band.m_hwndTarget == NULL)
            return FALSE;

        if (!m_rubber_band.CreateWindowDx(hwnd, NULL,
            WS_CHILD | WS_VISIBLE | WS_THICKFRAME, WS_EX_TOOLWINDOW,
            50, 50, 150, 150))
        {
            return FALSE;
        }

        return TRUE;
    }

    // WM_DROPFILES
    void OnDropFiles(HWND hwnd, HDROP hdrop)
    {
        TCHAR szPath[MAX_PATH];
        UINT cFiles = ::DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
        for (UINT i = 0; i < cFiles; ++i)
        {
            ::DragQueryFile(hdrop, i, szPath, MAX_PATH);
            ::MessageBox(m_hwnd, szPath, TEXT("File"), MB_ICONINFORMATION);
        }
        ::DragFinish(hdrop);
    }

    // WM_DESTROY
    void OnDestroy(HWND hwnd)
    {
        ::PostQuitMessage(0);
    }

    // WM_PAINT
    void OnPaint(HWND hwnd)
    {
        RECT rc;
        ::GetClientRect(m_hwnd, &rc);

        PAINTSTRUCT ps;
        HDC hdc = ::BeginPaint(m_hwnd, &ps);
        if (hdc != NULL)
        {
            // draw something...
            SelectObject(hdc, ::GetStockObject(BLACK_PEN));
            ::MoveToEx(hdc, rc.left, rc.top, NULL);
            ::LineTo(hdc, rc.right, rc.bottom);
            ::MoveToEx(hdc, rc.right, rc.top, NULL);
            ::LineTo(hdc, rc.left, rc.bottom);

            static const char s_text[] = PROGRAM_NAME;
            ::SetBkMode(hdc, TRANSPARENT);
            ::TextOutA(hdc, 0, 0, s_text, ::lstrlenA(s_text));

            ::EndPaint(m_hwnd, &ps);
        }
    }

    // WM_SIZE
    void OnSize(HWND hwnd, UINT state, int cx, int cy)
    {
        // NOTE: m_hwnd can be NULL at here. Needs check.
        if (m_hwnd == NULL)
        {
            return;
        }
        ::InvalidateRect(m_hwnd, NULL, TRUE);
    }

    // IDM_EXIT
    void OnExit()
    {
        ::DestroyWindow(m_hwnd);
    }

    // IDM_ABOUT
    void OnAbout();

    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
    {
        switch (id)
        {
        case IDM_EXIT:
            OnExit();
            break;
        case IDM_ABOUT:
            OnAbout();
            break;
        }
    }

    virtual LRESULT CALLBACK
    WindowProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
            HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
            HANDLE_MSG(hwnd, WM_SIZE, OnSize);
            HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
            HANDLE_MSG(hwnd, WM_DROPFILES, OnDropFiles);
            HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        default:
            return DefaultProcDx(hwnd, uMsg, wParam, lParam);
        }
        return 0;
    }
}; // WinApp

//////////////////////////////////////////////////////////////////////////////

struct AboutDialog : public DialogBase
{
    virtual INT_PTR CALLBACK
    DialogProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
            HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        }
        return DefaultProcDx(hwnd, uMsg, wParam, lParam);
    }

    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
    {
        return TRUE;
    }

    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
    {
        switch (id)
        {
        case IDOK:
            ::EndDialog(hwnd, IDOK);
            break;
        case IDCANCEL:
            ::EndDialog(hwnd, IDCANCEL);
            break;
        default:
            break;
        }
    }
};

void WinApp::OnAbout()
{
    AboutDialog about;
    about.DialogBoxDx(m_hwnd, IDD_ABOUTBOX);
}

BOOL WinApp::RegisterClassesDx()
{
    if (RegisterClassDx() &&
        m_rubber_band.RegisterClassDx())
    {
        return TRUE;
    }
    MsgBoxDx(TEXT("RegisterClass failed."), NULL, MB_ICONERROR);
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// Win32 App main function

extern "C"
INT APIENTRY _tWinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPTSTR      lpCmdLine,
    INT         nCmdShow)
{
    int ret;

    {
        WinApp app(__argc, __targv, hInstance);

        if (app.RegisterClassesDx())
        {
            if (app.StartDx(nCmdShow))
            {
                ret = INT(app.RunDx());
            }
            else
            {
                ret = 2;
            }
        }
        else
        {
            ret = 1;
        }
    }

#if (WINVER >= 0x0500)
    HANDLE hProcess = GetCurrentProcess();
    DebugPrintDx(TEXT("Count of GDI objects: %ld\n"),
                 GetGuiResources(hProcess, GR_GDIOBJECTS);
    DebugPrintDx(TEXT("Count of USER objects: %ld\n"),
                 GetGuiResources(hProcess, GR_USEROBJECTS);
#endif

#if defined(_MSC_VER) && !defined(NDEBUG)
    // for detecting memory leak (MSVC only)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    return ret;
}

//////////////////////////////////////////////////////////////////////////////
