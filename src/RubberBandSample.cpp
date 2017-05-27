// RubberBandSample.cpp --- A Win32 application                -*- C++ -*-
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////

// the class name of the main window
static const TCHAR  s_szClassName[] = TEXT(VERSION_INFO_STRING);

// maximum length of resource string plus one
static const INT    s_nMaxLoadString = 512;

//////////////////////////////////////////////////////////////////////////////

// the Win32 application
struct WinApp : public WindowBase
{
    INT         m_argc;         // number of command line parameters
    TCHAR **    m_targv;        // command line parameters

    HINSTANCE   m_hInst;        // the instance handle
    HICON       m_hIcon;        // the icon handle
    HACCEL      m_hAccel;       // the accelerator handle

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

    // WARNING: This function is not thread-safe!
    LPTSTR load_string(UINT nID)
    {
        static TCHAR s_sz[s_nMaxLoadString];
        s_sz[0] = 0;
        ::LoadString(m_hInst, nID, s_sz, s_nMaxLoadString);
        return s_sz;
    }

    // WARNING: This function is not thread-safe!
    LPTSTR load_string2(UINT nID)
    {
        static TCHAR s_sz[s_nMaxLoadString];
        s_sz[0] = 0;
        ::LoadString(m_hInst, nID, s_sz, s_nMaxLoadString);
        return s_sz;
    }

    // register window classes
    BOOL registerClasses();

    // start up
    BOOL startup(INT nCmdShow)
    {
        // load accessories
        m_hIcon = ::LoadIcon(m_hInst, MAKEINTRESOURCE(1));
        m_hAccel = ::LoadAccelerators(m_hInst, MAKEINTRESOURCE(1));

        // create the main window
        ::CreateWindow(s_szClassName, load_string(1), WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, 
            640, 400, 
            NULL, NULL, m_hInst, this);
        if (m_hwnd == NULL)
        {
            MessageBoxA(NULL, "ERROR: failure of CreateWindow",
                        PROGRAM_NAME, MB_ICONERROR);
            return FALSE;
        }

        // show the window
        ::ShowWindow(m_hwnd, nCmdShow);
        ::UpdateWindow(m_hwnd);

        return TRUE;
    }

    // message loop
    INT run()
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
    WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        return 0;
    }
}; // WinApp

//////////////////////////////////////////////////////////////////////////////

struct AboutDialog : public WindowBase
{
    virtual INT_PTR CALLBACK
    DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
            HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        }
        return 0;
    }

    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
    {
        m_hwnd = hwnd;
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
    ::DialogBoxParam(m_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), m_hwnd,
        WindowBase::DialogProc, (LPARAM)&about);
}

BOOL WinApp::registerClasses()
{
    WNDCLASSEX wcx;

    ZeroMemory(&wcx, sizeof(wcx));
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcx.lpfnWndProc = WindowBase::WindowProc;
    wcx.hInstance = m_hInst;
    wcx.hIcon = ::LoadIcon(m_hInst, MAKEINTRESOURCE(1));
    wcx.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1);
    wcx.lpszMenuName = MAKEINTRESOURCE(1);
    wcx.lpszClassName = s_szClassName;
    wcx.hIconSm = ::LoadIcon(m_hInst, MAKEINTRESOURCE(1));
    if (!::RegisterClassEx(&wcx))
    {
        return FALSE;
    }
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
        try
        {
            WinApp app(__argc, __targv, hInstance);

            if (app.registerClasses())
            {
                if (app.startup(nCmdShow))
                {
                    ret = app.run();
                }
                else
                {
                    ret = 2;
                }
            }
            else
            {
                ::MessageBoxA(NULL,
                    "ERROR: RegisterClass failed", PROGRAM_NAME,
                    MB_ICONERROR);
                ret = 1;
            }
        }
        catch (const std::bad_alloc&)
        {
            ::MessageBoxA(NULL, "ERROR: Out of memory", PROGRAM_NAME,
                          MB_ICONERROR);
            ret = -1;
        }
    }

#if defined(_MSC_VER) && !defined(NDEBUG)
    // for detecting memory leak (MSVC only)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    return ret;
}

//////////////////////////////////////////////////////////////////////////////
