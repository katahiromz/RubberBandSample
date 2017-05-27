// RubberBand.hpp -- Rubber Band for Win32
//////////////////////////////////////////////////////////////////////////////

#ifndef RUBBER_BAND_HPP_
#define RUBBER_BAND_HPP_

#include "WindowBase.hpp"

//////////////////////////////////////////////////////////////////////////////

struct RubberBand : public WindowBase
{
    HRGN m_hRgn;
    HWND m_hwndTarget;
    INT m_nGripSize;

    RubberBand() : m_hRgn(NULL), m_hwndTarget(NULL), m_nGripSize(3)
    {
    }

    virtual LPCTSTR GetWndClassNameDx() const
    {
        return TEXT("katahiromz's Rubber Band Class");
    }

    virtual void ModifyWndClassDx(WNDCLASSEX& wcx)
    {
        WindowBase::ModifyWndClassDx(wcx);
        wcx.hIcon = NULL;
        wcx.hbrBackground = GetStockBrush(NULL_BRUSH);
        wcx.lpszMenuName = MAKEINTRESOURCE(1);
        wcx.hIconSm = NULL;
    }

    virtual LRESULT CALLBACK
    WindowProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
            return DefaultProcDx(hwnd, uMsg, wParam, lParam);
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

    void FitToBand(HWND hwnd)
    {
        HWND hwndParent = GetParent(hwnd);

        RECT rc;
        GetWindowRect(hwnd, &rc);
        MapWindowRect(NULL, hwndParent, &rc);

        InflateRect(&rc, -2 * m_nGripSize, -2 * m_nGripSize);

        MoveWindow(m_hwndTarget, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top, TRUE);
    }

    void FitToTarget(HWND hwnd)
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
            FitToBand(hwnd);
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

#endif  // ndef RUBBER_BAND_HPP_

//////////////////////////////////////////////////////////////////////////////
