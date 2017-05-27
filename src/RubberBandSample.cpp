// RubberBandSample.cpp --- A Win32 application                -*- C++ -*-
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////

// the class name of the main window
static const TCHAR  s_szClassName[] = TEXT(VERSION_INFO_STRING);
static const TCHAR  s_szRubberBandClass[] = TEXT("katahiromz's Rubber Band Class");

// maximum length of resource string plus one
static const INT    s_nMaxLoadString = 512;

//////////////////////////////////////////////////////////////////////////////

struct RubberBand : public WindowBase
{
    HRGN m_hRgn;
    HWND m_hwndTarget;
    INT m_nGripSize;

    RubberBand() : m_hRgn(NULL), m_hwndTarget(NULL), m_nGripSize(3)
    {
    }

    virtual LRESULT CALLBACK
    WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
            HANDLE_MSG(hwnd, WM_SIZE, OnSize);
            HANDLE_MSG(hwnd, WM_NCPAINT, OnNCPaint);
            HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
            HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
            HANDLE_MSG(hwnd, WM_NCHITTEST, OnNCHitTest);
            HANDLE_MSG(hwnd, WM_SETCURSOR, OnSetCursor);
        default:
            return DefaultProc(hwnd, uMsg, wParam, lParam);
        }
        return 0;
    }

    BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
    {
        m_hwnd = hwnd;
        return TRUE;
    }

    void OnDestroy(HWND hwnd)
    {
        DeleteObject(m_hRgn);
        m_hRgn = NULL;
    }

    void GetRect(HWND hwnd, LPRECT prc)
    {
        GetWindowRect(hwnd, prc);
        OffsetRect(prc, -prc->left, -prc->top);
    }

    void OnNCPaint(HWND hwnd, HRGN hrgn)
    {
        RECT rc;
        GetRect(hwnd, &rc);

        HDC hDC = GetWindowDC(hwnd);
        FillRect(hDC, &rc, GetStockBrush(BLACK_BRUSH));
        ReleaseDC(hwnd, hDC);
    }

    void OnPaint(HWND hwnd)
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hwnd, &ps);
        if (hDC)
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hDC, &rc, GetStockBrush(BLACK_BRUSH));
            EndPaint(hwnd, &ps);
        }
    }

    UINT OnNCHitTest(HWND hwnd, int x, int y)
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        x -= rc.left;
        y -= rc.top;

        INT cx = (rc.right - rc.left) / 2;
        INT cy = (rc.bottom - rc.top) / 2;

        if (x < cx - m_nGripSize)
        {
            if (y < cy - m_nGripSize)
            {
                return HTTOPLEFT;
            }
            else if (cy - m_nGripSize <= y && y <= cy + m_nGripSize)
            {
                return HTLEFT;
            }
            else
            {
                return HTBOTTOMLEFT;
            }
        }
        else if (cx - m_nGripSize <= x && x <= cx + m_nGripSize)
        {
            if (y < cy - m_nGripSize)
            {
                return HTTOP;
            }
            else if (cy - m_nGripSize <= y && y <= cy + m_nGripSize)
            {
                ;
            }
            else
            {
                return HTBOTTOM;
            }
        }
        else
        {
            if (y < cy - m_nGripSize)
            {
                return HTTOPRIGHT;
            }
            else if (cy - m_nGripSize <= y && y <= cy + m_nGripSize)
            {
                return HTRIGHT;
            }
            else
            {
                return HTBOTTOMRIGHT;
            }
        }
        return HTCLIENT;
    }

    BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
    {
        switch (codeHitTest)
        {
        case HTTOPLEFT:         SetCursor(LoadCursor(NULL, IDC_SIZENWSE)); break;
        case HTLEFT:            SetCursor(LoadCursor(NULL, IDC_SIZEWE)); break;
        case HTBOTTOMLEFT:      SetCursor(LoadCursor(NULL, IDC_SIZENESW)); break;
        case HTTOP:             SetCursor(LoadCursor(NULL, IDC_SIZENS)); break;
        case HTBOTTOM:          SetCursor(LoadCursor(NULL, IDC_SIZENS)); break;
        case HTTOPRIGHT:        SetCursor(LoadCursor(NULL, IDC_SIZENESW)); break;
        case HTRIGHT:           SetCursor(LoadCursor(NULL, IDC_SIZEWE)); break;
        case HTBOTTOMRIGHT:     SetCursor(LoadCursor(NULL, IDC_SIZENWSE)); break;
        }
        return TRUE;
    }

    void FitToTarget(HWND hwnd)
    {
        HWND hwndParent = GetParent(hwnd);

        RECT rc;
        GetWindowRect(hwnd, &rc);
        MapWindowRect(NULL, hwndParent, &rc);

        InflateRect(&rc, -2 * m_nGripSize, -2 * m_nGripSize);

        MoveWindow(m_hwndTarget, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top, TRUE);
    }

    void FitToBand(HWND hwnd)
    {
        HWND hwndParent = GetParent(hwnd);

        RECT rc;
        GetWindowRect(m_hwndTarget, &rc);
        MapWindowRect(NULL, hwndParent, &rc);

        InflateRect(&rc, 2 * m_nGripSize, 2 * m_nGripSize);

        MoveWindow(hwnd, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top, TRUE);
    }

    void OnSize(HWND hwnd, UINT state, int cx, int cy)
    {
        RECT rc;

        if (m_hwndTarget)
        {
            FitToTarget(hwnd);
        }

        GetRect(hwnd, &rc);
        cx = rc.right - rc.left;
        cy = rc.bottom - rc.top;

        INT ax[] = { m_nGripSize, cx / 2, cx - m_nGripSize };
        INT ay[] = { m_nGripSize, cy / 2, cy - m_nGripSize };

        HRGN hRgn = CreateRectRgn(0, 0, 0, 0);
        for (INT i = 0; i < 3; ++i)
        {
            for (INT k = 0; k < 3; ++k)
            {
                if (i == 1 && k == 1)
                    continue;

                HRGN hRgn2 = CreateRectRgn(
                    ax[i] - m_nGripSize, ay[k] - m_nGripSize,
                    ax[i] + m_nGripSize, ay[k] + m_nGripSize);
                UnionRgn(hRgn, hRgn, hRgn2);
                DeleteObject(hRgn2);
            }
        }

        DeleteObject(m_hRgn);

        m_hRgn = hRgn;
        SetWindowRgn(hwnd, m_hRgn, TRUE);
        InvalidateRect(hwnd, NULL, TRUE);
    }
};

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
        ::CreateWindow(s_szClassName, load_string(1),
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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

        m_rubber_band.m_hwndTarget = CreateWindow(TEXT("BUTTON"), TEXT("Test Me!"),
            BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
            10, 10, 100, 100, hwnd, NULL, m_hInst, NULL);

        CreateWindowEx(WS_EX_TOOLWINDOW, s_szRubberBandClass, NULL,
            WS_CHILD | WS_VISIBLE | WS_THICKFRAME,
            10, 10, 100, 100, hwnd, NULL, m_hInst, &m_rubber_band);

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
            return DefaultProc(hwnd, uMsg, wParam, lParam);
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

    ZeroMemory(&wcx, sizeof(wcx));
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcx.lpfnWndProc = WindowBase::WindowProc;
    wcx.hInstance = m_hInst;
    wcx.hIcon = NULL;
    wcx.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground = GetStockBrush(NULL_BRUSH);
    wcx.lpszMenuName = MAKEINTRESOURCE(1);
    wcx.lpszClassName = s_szRubberBandClass;
    wcx.hIconSm = NULL;
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
