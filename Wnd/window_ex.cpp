#include "stdafx.h"
#include "window_ex.h"
#include "base/thread/thread_manager.h"
#include "threads_define.h"
#include "base/file/file_util.h"
#include <shlwapi.h >


namespace ui
{

ShadowWnd::ShadowWnd()
{

}

void ShadowWnd::SetShadowCorner(const ui::UiRect &rect)
{
	m_shadow.SetShadowCorner(rect);
}

void ShadowWnd::SetShadowImage(const std::wstring &image)
{
	m_shadow.SetShadowImage(image);
}

void ShadowWnd::SetRoundCorner(int cx, int cy)
{
	m_shadow.SetRoundCorner(cx, cy);
}

void ShadowWnd::ResetShadowBox()
{
	m_shadow.ResetShadowBox();
}

std::wstring ShadowWnd::GetSkinFolder()
{
	return L"shadow";
}

std::wstring ShadowWnd::GetSkinFile()
{
	return L"shadow.xml";
}

std::wstring ShadowWnd::GetWindowClassName() const
{
	return L"ShadowWindow";
}

HWND ShadowWnd::Create(Window* window)
{
	first_show_window_ = true;
	window_ = window;
	return Window::Create(NULL, L"ShadowWindow", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, true);
}

LRESULT ShadowWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (window_ == NULL || window_->GetHWND() == NULL)
	{
		return FALSE;
	}

	switch (uMsg)
	{
	case WM_ERASEBKGND:
	case WM_PAINT:
	case WM_MOVE:
	case WM_ACTIVATE:
	case WM_NCACTIVATE:
	{
		if (::IsWindowVisible(window_->GetHWND()))
		{
			RECT rc;
			::GetWindowRect(window_->GetHWND(), &rc);
			SetPos(rc, false, /*SWP_SHOWWINDOW | */SWP_NOACTIVATE, window_->GetHWND());
		}
		break;
	}
	case WM_CLOSE:
	{
		if (delay_timer_.HasUsed())
		{
			delay_timer_.Cancel();
		}
		ShowWindow(false, false);
		Close(0);
		break;
	}
	case WM_SHOWWINDOW:
	{
		if (first_show_window_)
		{
			if (delay_timer_.HasUsed())
			{
				delay_timer_.Cancel();
			}
			auto ret_cb = delay_timer_.ToWeakCallback([=]()
			{
				first_show_window_ = false;
				ShowWindow(wParam == 0 ? false : true, false);
			});
			nbase::ThreadManager::PostDelayedTask(ThreadIdDefine::kThreadUI, ret_cb, nbase::TimeDelta::FromMilliseconds(1500));
		}
		else
		{
			ShowWindow(wParam == 0 ? false : true, false);
		}

		break;
	}
	default:
	{
		break;
	}

	}

	bHandled = FALSE;
	return FALSE;
}


WindowEx::WindowEx(const wchar_t *bk_path):WindowImplBase(bk_path)
{

}

WindowEx::~WindowEx()
{
}

HWND WindowEx::Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, bool isLayeredWindow, const ui::UiRect& rc)
{
	if (!RegisterWnd())
	{
		return NULL;
	}

	if (!isLayeredWindow)
	{
// 		shadow_wnd_ = new ShadowWnd;
// 		this->AddMessageFilter(shadow_wnd_);
		//在创建窗口
		__super::Create(hwndParent, pstrName, dwStyle, dwExStyle, false, rc);

		m_shadow.ShowShadow(true);
		m_shadow.SetSize(25);
		m_shadow.SetShadowCorner(ui::UiRect(25,25,25,25));

		std::wstring imageFullPath = ui::GlobalManager::GetResourcePath();
		imageFullPath += L"../res/bk/bk_shadow.png";
		m_shadow.SetImage(imageFullPath.c_str());
		m_shadow.Create(GetHWND());

// 		shadow_wnd_->Create(this);
// 		if (::IsWindowVisible(GetHWND()))
// 			shadow_wnd_->ShowWindow();
		return GetHWND();
	}

	HWND hwnd = __super::Create(hwndParent, pstrName, dwStyle, dwExStyle, isLayeredWindow, rc);
	ASSERT(hwnd);
	return hwnd;
}

LRESULT WindowEx::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UnRegisterWnd();
	return __super::OnDestroy(uMsg, wParam, lParam, bHandled);
}

void WindowEx::OnEsc( BOOL &bHandled )
{
	bHandled = FALSE;
}

bool WindowEx::RegisterWnd()
{
	std::wstring wnd_class_name = GetWindowClassName();
	std::wstring wnd_id = GetWindowId();
	if (!WindowsManager::GetInstance()->RegisterWindow(wnd_class_name, wnd_id, this))
	{
		return false;
	}
	return true;
}

void WindowEx::UnRegisterWnd()
{
	std::wstring wnd_class_name = GetWindowClassName();
	std::wstring wnd_id = GetWindowId();
	WindowsManager::GetInstance()->UnRegisterWindow(wnd_class_name, wnd_id, this);
}

LRESULT WindowEx::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_CLOSE)
	{
		if(!::IsWindowEnabled(m_hWnd))
		{
			::SetForegroundWindow(m_hWnd);
			return FALSE;
		}
	}
	else if(uMsg == WM_KILLFOCUS)
	{
		KillFocus();
	}
	else if(uMsg == WM_KEYDOWN)
	{
		if(wParam == VK_ESCAPE)
		{
			BOOL bHandled = FALSE;
			OnEsc(bHandled);
			if( !bHandled )
				this->Close();
		}
	}
	return __super::HandleMessage(uMsg,wParam,lParam);
}

ui::UiRect WindowEx::CalculateWindowRBPos()
{
	ui::UiRect rcwnd = GetPos(true);
	ui::UiRect rccs = GetAlphaFixCorner();
	RECT rcSysbar;
	SystemBar_Pos pos;
	GetSystemBarPos(rcSysbar, pos);
	//int x = GetSystemMetrics(SM_CXSCREEN);
	//int y = GetSystemMetrics(SM_CYSCREEN);
	int wnd_height = rcwnd.GetHeight();
	int wnd_width = rcwnd.GetWidth();
	ui::UiRect rc2;
	if (pos == SystemBar_Pos::TOP)
	{
		rc2.top = rcSysbar.bottom + 4 - rccs.top;
		rc2.bottom = rc2.top + wnd_height;
		rc2.right = rcSysbar.right - 4 + rccs.right;
		rc2.left = rc2.right - wnd_width;
	}
	else if (pos == SystemBar_Pos::BOTTOM)
	{
		rc2.bottom = rcSysbar.top - 4 + rccs.bottom;
		rc2.top = rc2.bottom - wnd_height;
		rc2.right = rcSysbar.right - 4 + rccs.right;
		rc2.left = rc2.right - wnd_width;
	}
	else if (pos == SystemBar_Pos::LEFT)
	{
		rc2.left = rcSysbar.right + 4 - rccs.left;
		rc2.right = rc2.left + wnd_width;
		rc2.bottom = rcSysbar.bottom - 4 + rccs.bottom;
		rc2.top = rc2.bottom - wnd_height;
	}
	else if (pos == SystemBar_Pos::RIGHT)
	{
		rc2.right = rcSysbar.left - 4 + rccs.right;
		rc2.left = rc2.right - wnd_width;
		rc2.bottom = rcSysbar.bottom - 4 + rccs.bottom;
		rc2.top = rc2.bottom - wnd_height;
	}
	return rc2;
}


void WindowEx::SetHideShadowUI(bool hide/* = true*/)
{
	if (!IsWindow(m_hWnd))
	{
		return;
	}
	if (!IsWindowVisible(m_hWnd))
	{
		return;
	}

	if (::IsZoomed(m_hWnd))
	{
		m_shadow.SetHideShadow(hide);
	}
}

POINT GetPopupWindowPos( WindowEx* window )
{
	ASSERT( window && IsWindow( window->GetHWND() ) );

	//屏幕大小
	MONITORINFO oMonitor = { sizeof(oMonitor) };
	::GetMonitorInfo( ::MonitorFromWindow( window->GetHWND(), MONITOR_DEFAULTTONEAREST ), &oMonitor );
	RECT screen = oMonitor.rcWork;

	ui::UiRect rect = window->GetPos(true);

	POINT pt = { 0, 0 };
	pt.x = screen.right - rect.GetWidth();
	pt.y = screen.bottom - rect.GetHeight();

	return pt;
}

void GetSystemBarPos(RECT& rc, SystemBar_Pos& pos)
{
	::GetWindowRect(FindWindow(L"Shell_TrayWnd", L""), &rc);
	unsigned int width = rc.right - rc.left;
	unsigned int height = rc.bottom - rc.top;
	if (width >= height)
	{
		if (rc.top == 0)
			pos = SystemBar_Pos::TOP;
		else
			pos = SystemBar_Pos::BOTTOM;
	}
	else
	{
		if (rc.left == 0)
			pos = SystemBar_Pos::LEFT;
		else
			pos = SystemBar_Pos::RIGHT;
	}
}

//////////////////////////////////////////////////////////////////////////
const TCHAR *strWndClassName = _T("PerryShadowWnd");
bool CShadowUI::s_bHasInit = FALSE;

CShadowUI::CShadowUI(void)
	: m_hWnd((HWND)NULL)
	, m_OriParentProc(NULL)
	, m_Status(0)
	, m_nDarkness(150)
	, m_nSharpness(5)
	, m_nSize(0)
	, m_nxOffset(0)
	, m_nyOffset(0)
	, m_Color(RGB(0, 0, 0))
	, m_WndSize(0)
	, m_bUpdate(false)
	, m_bIsImageMode(false)
	, m_bIsShowShadow(false)
	, m_bIsDisableShadow(false)
{
	::ZeroMemory(&m_rcShadowCorner, sizeof(RECT));
}

CShadowUI::~CShadowUI(void)
{
}

bool CShadowUI::Initialize(HINSTANCE hInstance)
{
	if (s_bHasInit)
		return false;

	// Register window class for shadow window
	WNDCLASSEX wcex;

	memset(&wcex, 0, sizeof(wcex));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DefWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = strWndClassName;
	wcex.hIconSm = NULL;

	RegisterClassEx(&wcex);

	s_bHasInit = true;
	return true;
}

void CShadowUI::Create(HWND hParentWnd)
{
	if (!m_bIsShowShadow)
		return;

	// Already initialized
	if (!s_bHasInit)
	{
		Initialize(GetModuleHandle(nullptr));
	}
	// Add parent window - shadow pair to the map
	_ASSERT(GetShadowMap().find(hParentWnd) == GetShadowMap().end());	// Only one shadow for each window
	GetShadowMap()[hParentWnd] = this;

	// Determine the initial show state of shadow according to parent window's state
	LONG lParentStyle = GetWindowLongPtr(hParentWnd, GWL_STYLE);

	// Create the shadow window
	LONG styleValue = lParentStyle & WS_CAPTION;
	m_hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT, strWndClassName, NULL,
		/*WS_VISIBLE | */styleValue | WS_POPUPWINDOW,
		CW_USEDEFAULT, 0, 0, 0, hParentWnd, NULL, ::GetModuleHandle(NULL), NULL);

	if (!(WS_VISIBLE & lParentStyle))	// Parent invisible
		m_Status = SS_ENABLED;
	else if ((WS_MAXIMIZE | WS_MINIMIZE) & lParentStyle)	// Parent visible but does not need shadow
		m_Status = SS_ENABLED | SS_PARENTVISIBLE;
	else	// Show the shadow
	{
		m_Status = SS_ENABLED | SS_VISABLE | SS_PARENTVISIBLE;
		::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
		Update(hParentWnd);
	}

	// Replace the original WndProc of parent window to steal messages
	m_OriParentProc = GetWindowLongPtr(hParentWnd, GWLP_WNDPROC);

#pragma warning(disable: 4311)	// temporrarily disable the type_cast warning in Win32
	SetWindowLongPtr(hParentWnd, GWLP_WNDPROC, (LONG_PTR)ParentProc);
#pragma warning(default: 4311)

}

std::map<HWND, CShadowUI *>& CShadowUI::GetShadowMap()
{
	static std::map<HWND, CShadowUI *> s_Shadowmap;
	return s_Shadowmap;
}

LRESULT CALLBACK CShadowUI::ParentProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_ASSERT(GetShadowMap().find(hwnd) != GetShadowMap().end());	// Shadow must have been attached

	CShadowUI *pThis = GetShadowMap()[hwnd];
	if (!pThis)
	{
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	if (pThis->m_bIsDisableShadow) {

#pragma warning(disable: 4312)	// temporrarily disable the type_cast warning in Win32
		// Call the default(original) window procedure for other messages or messages processed but not returned
		return ((WNDPROC)pThis->m_OriParentProc)(hwnd, uMsg, wParam, lParam);
#pragma warning(default: 4312)
	}
	switch (uMsg)
	{
	case WM_ACTIVATEAPP:
	case WM_NCACTIVATE:
	{
		if (::IsWindowVisible(pThis->m_hWnd))
		{
			::SetWindowPos(pThis->m_hWnd, hwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);
		}
		break;
	}
	case WM_WINDOWPOSCHANGED:

		RECT WndRect;
		LPWINDOWPOS pWndPos;
		pWndPos = (LPWINDOWPOS)lParam;
		GetWindowRect(hwnd, &WndRect);
		if (pThis->m_bIsImageMode) {
			SetWindowPos(pThis->m_hWnd, hwnd, WndRect.left - pThis->m_nSize, WndRect.top - pThis->m_nSize, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
		}
		else {
			SetWindowPos(pThis->m_hWnd, hwnd, WndRect.left + pThis->m_nxOffset - pThis->m_nSize, WndRect.top + pThis->m_nyOffset - pThis->m_nSize, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
		}

		if (pWndPos->flags & SWP_SHOWWINDOW) {
			if (pThis->m_Status & SS_ENABLED && !(pThis->m_Status & SS_PARENTVISIBLE))
			{
				if (!IsZoomed(hwnd))
				{
					pThis->m_bUpdate = true;
					::ShowWindow(pThis->m_hWnd, SW_SHOWNOACTIVATE);
					pThis->m_Status |= SS_VISABLE | SS_PARENTVISIBLE;
				}
			}
		}
		else if (pWndPos->flags & SWP_HIDEWINDOW) {
			if (pThis->m_Status & SS_ENABLED)
			{
				::ShowWindow(pThis->m_hWnd, SW_HIDE);
				pThis->m_Status &= ~(SS_VISABLE | SS_PARENTVISIBLE);
			}
		}
		break;
	case WM_MOVE:
		if (pThis->m_Status & SS_VISABLE && ::IsWindowVisible(pThis->m_hWnd)) {
			RECT WndRect;
			GetWindowRect(hwnd, &WndRect);
			if (pThis->m_bIsImageMode) {
				SetWindowPos(pThis->m_hWnd, hwnd, WndRect.left - pThis->m_nSize, WndRect.top - pThis->m_nSize, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
			}
			else {
				SetWindowPos(pThis->m_hWnd, hwnd, WndRect.left + pThis->m_nxOffset - pThis->m_nSize, WndRect.top + pThis->m_nyOffset - pThis->m_nSize, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}
		break;

	case WM_SIZE:
		if (pThis->m_Status & SS_ENABLED)
		{
			if (SIZE_MAXIMIZED == wParam || SIZE_MINIMIZED == wParam)
			{
				::ShowWindow(pThis->m_hWnd, SW_HIDE);
				::SetWindowPos(pThis->m_hWnd, hwnd, 99999, 99999, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);
				pThis->m_Status &= ~SS_VISABLE;
			}
			else if (pThis->m_Status & SS_PARENTVISIBLE)	// Parent maybe resized even if invisible
			{
				// Awful! It seems that if the window size was not decreased
				// the window region would never be updated until WM_PAINT was sent.
				// So do not Update() until next WM_PAINT is received in this case
				if (LOWORD(lParam) > LOWORD(pThis->m_WndSize) || HIWORD(lParam) > HIWORD(pThis->m_WndSize))
					pThis->m_bUpdate = true;
				else
					pThis->Update(hwnd);
				if (!(pThis->m_Status & SS_VISABLE))
				{
					if (!IsZoomed(hwnd))
					{
						::ShowWindow(pThis->m_hWnd, SW_SHOWNOACTIVATE);
						pThis->m_Status |= SS_VISABLE;
					}
				}
			}
			pThis->m_WndSize = lParam;
		}
		break;

	case WM_PAINT:
	{
		if (pThis->m_bUpdate)
		{
			pThis->Update(hwnd);
			pThis->m_bUpdate = false;
		}
		//return hr;
		break;
	}

	// In some cases of sizing, the up-right corner of the parent window region would not be properly updated
	// Update() again when sizing is finished
	case WM_EXITSIZEMOVE:
		if (pThis->m_Status & SS_VISABLE)
		{
			pThis->Update(hwnd);
		}
		break;

	case WM_SHOWWINDOW:
		if (pThis->m_Status & SS_ENABLED)
		{
			if (!wParam)	// the window is being hidden
			{
				::ShowWindow(pThis->m_hWnd, SW_HIDE);
				pThis->m_Status &= ~(SS_VISABLE | SS_PARENTVISIBLE);
			}
			else if (!(pThis->m_Status & SS_PARENTVISIBLE))
			{
				//pThis->Update(hwnd);
				pThis->m_bUpdate = true;
				::ShowWindow(pThis->m_hWnd, SW_SHOWNOACTIVATE);
				pThis->m_Status |= SS_VISABLE | SS_PARENTVISIBLE;
			}
		}
		break;

	case WM_DESTROY:
		DestroyWindow(pThis->m_hWnd);	// Destroy the shadow
		break;

	case WM_NCDESTROY:
		GetShadowMap().erase(hwnd);	// Remove this window and shadow from the map
		break;

	}


#pragma warning(disable: 4312)	// temporrarily disable the type_cast warning in Win32
	// Call the default(original) window procedure for other messages or messages processed but not returned
	return ((WNDPROC)pThis->m_OriParentProc)(hwnd, uMsg, wParam, lParam);
#pragma warning(default: 4312)

}
void GetLastErrorMessage() {          //Formats GetLastError() value.
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf, 0, NULL
		);

	// Display the string.
	//MessageBox(NULL, (const wchar_t*)lpMsgBuf, L"GetLastError", MB_OK | MB_ICONINFORMATION);

	// Free the buffer.
	LocalFree(lpMsgBuf);

}

static COLORREF PixelAlpha(COLORREF clrSrc, double src_darken, COLORREF clrDest, double dest_darken)
{
	return RGB(GetRValue(clrSrc) * src_darken + GetRValue(clrDest) * dest_darken,
		GetGValue(clrSrc) * src_darken + GetGValue(clrDest) * dest_darken,
		GetBValue(clrSrc) * src_darken + GetBValue(clrDest) * dest_darken);
}

static BOOL WINAPI AlphaBitBlt(HDC hDC, int nDestX, int nDestY, int dwWidth, int dwHeight, HDC hSrcDC, \
	int nSrcX, int nSrcY, int wSrc, int hSrc, BLENDFUNCTION ftn)
{
	HDC hTempDC = ::CreateCompatibleDC(hDC);
	if (NULL == hTempDC)
		return FALSE;

	//Creates Source DIB
	LPBITMAPINFO lpbiSrc = NULL;
	// Fill in the BITMAPINFOHEADER
	lpbiSrc = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
	if (lpbiSrc == NULL)
	{
		::DeleteDC(hTempDC);
		return FALSE;
	}
	lpbiSrc->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbiSrc->bmiHeader.biWidth = dwWidth;
	lpbiSrc->bmiHeader.biHeight = dwHeight;
	lpbiSrc->bmiHeader.biPlanes = 1;
	lpbiSrc->bmiHeader.biBitCount = 32;
	lpbiSrc->bmiHeader.biCompression = BI_RGB;
	lpbiSrc->bmiHeader.biSizeImage = dwWidth * dwHeight;
	lpbiSrc->bmiHeader.biXPelsPerMeter = 0;
	lpbiSrc->bmiHeader.biYPelsPerMeter = 0;
	lpbiSrc->bmiHeader.biClrUsed = 0;
	lpbiSrc->bmiHeader.biClrImportant = 0;

	COLORREF* pSrcBits = NULL;
	HBITMAP hSrcDib = CreateDIBSection(
		hSrcDC, lpbiSrc, DIB_RGB_COLORS, (void **)&pSrcBits,
		NULL, NULL);

	if ((NULL == hSrcDib) || (NULL == pSrcBits))
	{
		delete[] lpbiSrc;
		::DeleteDC(hTempDC);
		return FALSE;
	}

	HBITMAP hOldTempBmp = (HBITMAP)::SelectObject(hTempDC, hSrcDib);
	::StretchBlt(hTempDC, 0, 0, dwWidth, dwHeight, hSrcDC, nSrcX, nSrcY, wSrc, hSrc, SRCCOPY);
	::SelectObject(hTempDC, hOldTempBmp);

	//Creates Destination DIB
	LPBITMAPINFO lpbiDest = NULL;
	// Fill in the BITMAPINFOHEADER
	lpbiDest = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
	if (lpbiDest == NULL)
	{
		delete[] lpbiSrc;
		::DeleteObject(hSrcDib);
		::DeleteDC(hTempDC);
		return FALSE;
	}

	lpbiDest->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbiDest->bmiHeader.biWidth = dwWidth;
	lpbiDest->bmiHeader.biHeight = dwHeight;
	lpbiDest->bmiHeader.biPlanes = 1;
	lpbiDest->bmiHeader.biBitCount = 32;
	lpbiDest->bmiHeader.biCompression = BI_RGB;
	lpbiDest->bmiHeader.biSizeImage = dwWidth * dwHeight;
	lpbiDest->bmiHeader.biXPelsPerMeter = 0;
	lpbiDest->bmiHeader.biYPelsPerMeter = 0;
	lpbiDest->bmiHeader.biClrUsed = 0;
	lpbiDest->bmiHeader.biClrImportant = 0;

	COLORREF* pDestBits = NULL;
	HBITMAP hDestDib = CreateDIBSection(
		hDC, lpbiDest, DIB_RGB_COLORS, (void **)&pDestBits,
		NULL, NULL);

	if ((NULL == hDestDib) || (NULL == pDestBits))
	{
		delete[] lpbiSrc;
		::DeleteObject(hSrcDib);
		::DeleteDC(hTempDC);
		return FALSE;
	}

	::SelectObject(hTempDC, hDestDib);
	::BitBlt(hTempDC, 0, 0, dwWidth, dwHeight, hDC, nDestX, nDestY, SRCCOPY);
	::SelectObject(hTempDC, hOldTempBmp);

	double src_darken;
	BYTE nAlpha;

	for (int pixel = 0; pixel < dwWidth * dwHeight; pixel++, pSrcBits++, pDestBits++)
	{
		nAlpha = LOBYTE(*pSrcBits >> 24);
		src_darken = (double)(nAlpha * ftn.SourceConstantAlpha) / 255.0 / 255.0;
		if (src_darken < 0.0) src_darken = 0.0;
		*pDestBits = PixelAlpha(*pSrcBits, src_darken, *pDestBits, 1.0 - src_darken);
	} //for

	::SelectObject(hTempDC, hDestDib);
	::BitBlt(hDC, nDestX, nDestY, dwWidth, dwHeight, hTempDC, 0, 0, SRCCOPY);
	::SelectObject(hTempDC, hOldTempBmp);

	delete[] lpbiDest;
	::DeleteObject(hDestDib);

	delete[] lpbiSrc;
	::DeleteObject(hSrcDib);

	::DeleteDC(hTempDC);
	return TRUE;
}

void DrawImage(HDC hDC, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint,
	const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha,
	BYTE uFade, bool hole, bool xtiled, bool ytiled)
{
	ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);

	typedef BOOL(WINAPI *LPALPHABLEND)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
	static LPALPHABLEND lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "AlphaBlend");

	if (lpAlphaBlend == NULL) lpAlphaBlend = AlphaBitBlt;
	if (hBitmap == NULL) return;

	HDC hCloneDC = ::CreateCompatibleDC(hDC);
	HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
	::SetStretchBltMode(hDC, HALFTONE);

	RECT rcTemp = { 0 };
	RECT rcDest = { 0 };
	if (lpAlphaBlend && (bAlpha || uFade < 255)) {
		BLENDFUNCTION bf = { AC_SRC_OVER, 0, uFade, AC_SRC_ALPHA };
		// middle
		if (!hole) {
			rcDest.left = rc.left + rcCorners.left;
			rcDest.top = rc.top + rcCorners.top;
			rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
			rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
				if (!xtiled && !ytiled) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
						rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, \
						rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
				}
				else if (xtiled && ytiled) {
					LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
					LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
					int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
					int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
					for (int j = 0; j < iTimesY; ++j) {
						LONG lDestTop = rcDest.top + lHeight * j;
						LONG lDestBottom = rcDest.top + lHeight * (j + 1);
						LONG lDrawHeight = lHeight;
						if (lDestBottom > rcDest.bottom) {
							lDrawHeight -= lDestBottom - rcDest.bottom;
							lDestBottom = rcDest.bottom;
						}
						for (int i = 0; i < iTimesX; ++i) {
							LONG lDestLeft = rcDest.left + lWidth * i;
							LONG lDestRight = rcDest.left + lWidth * (i + 1);
							LONG lDrawWidth = lWidth;
							if (lDestRight > rcDest.right) {
								lDrawWidth -= lDestRight - rcDest.right;
								lDestRight = rcDest.right;
							}
							lpAlphaBlend(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j,
								lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC,
								rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, lDrawWidth, lDrawHeight, bf);
						}
					}
				}
				else if (xtiled) {
					LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
					int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
					for (int i = 0; i < iTimes; ++i) {
						LONG lDestLeft = rcDest.left + lWidth * i;
						LONG lDestRight = rcDest.left + lWidth * (i + 1);
						LONG lDrawWidth = lWidth;
						if (lDestRight > rcDest.right) {
							lDrawWidth -= lDestRight - rcDest.right;
							lDestRight = rcDest.right;
						}
						lpAlphaBlend(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom,
							hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
							lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
					}
				}
				else { // ytiled
					LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
					int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
					for (int i = 0; i < iTimes; ++i) {
						LONG lDestTop = rcDest.top + lHeight * i;
						LONG lDestBottom = rcDest.top + lHeight * (i + 1);
						LONG lDrawHeight = lHeight;
						if (lDestBottom > rcDest.bottom) {
							lDrawHeight -= lDestBottom - rcDest.bottom;
							lDestBottom = rcDest.bottom;
						}
						lpAlphaBlend(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop,
							hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
							rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, lDrawHeight, bf);
					}
				}
			}
		}

		// left-top
		if (rcCorners.left > 0 && rcCorners.top > 0) {
			rcDest.left = rc.left;
			rcDest.top = rc.top;
			rcDest.right = rcCorners.left;
			rcDest.bottom = rcCorners.top;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
				lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left, rcBmpPart.top, rcCorners.left, rcCorners.top, bf);
			}
		}
		// top
		if (rcCorners.top > 0) {
			rcDest.left = rc.left + rcCorners.left;
			rcDest.top = rc.top;
			rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
			rcDest.bottom = rcCorners.top;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
				lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left + rcCorners.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - \
					rcCorners.left - rcCorners.right, rcCorners.top, bf);
			}
		}
		// right-top
		if (rcCorners.right > 0 && rcCorners.top > 0) {
			rcDest.left = rc.right - rcCorners.right;
			rcDest.top = rc.top;
			rcDest.right = rcCorners.right;
			rcDest.bottom = rcCorners.top;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
				lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.right - rcCorners.right, rcBmpPart.top, rcCorners.right, rcCorners.top, bf);
			}
		}
		// left
		if (rcCorners.left > 0) {
			rcDest.left = rc.left;
			rcDest.top = rc.top + rcCorners.top;
			rcDest.right = rcCorners.left;
			rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
				lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left, rcBmpPart.top + rcCorners.top, rcCorners.left, rcBmpPart.bottom - \
					rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
			}
		}
		// right
		if (rcCorners.right > 0) {
			rcDest.left = rc.right - rcCorners.right;
			rcDest.top = rc.top + rcCorners.top;
			rcDest.right = rcCorners.right;
			rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
				lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.right - rcCorners.right, rcBmpPart.top + rcCorners.top, rcCorners.right, \
					rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, bf);
			}
		}
		// left-bottom
		if (rcCorners.left > 0 && rcCorners.bottom > 0) {
			rcDest.left = rc.left;
			rcDest.top = rc.bottom - rcCorners.bottom;
			rcDest.right = rcCorners.left;
			rcDest.bottom = rcCorners.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
				lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left, rcBmpPart.bottom - rcCorners.bottom, rcCorners.left, rcCorners.bottom, bf);
			}
		}
		// bottom
		if (rcCorners.bottom > 0) {
			rcDest.left = rc.left + rcCorners.left;
			rcDest.top = rc.bottom - rcCorners.bottom;
			rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
			rcDest.bottom = rcCorners.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
				lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left + rcCorners.left, rcBmpPart.bottom - rcCorners.bottom, \
					rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, rcCorners.bottom, bf);
			}
		}
		// right-bottom
		if (rcCorners.right > 0 && rcCorners.bottom > 0) {
			rcDest.left = rc.right - rcCorners.right;
			rcDest.top = rc.bottom - rcCorners.bottom;
			rcDest.right = rcCorners.right;
			rcDest.bottom = rcCorners.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
				lpAlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.right - rcCorners.right, rcBmpPart.bottom - rcCorners.bottom, rcCorners.right, \
					rcCorners.bottom, bf);
			}
		}
	}
	else
	{
		if (rc.right - rc.left == rcBmpPart.right - rcBmpPart.left \
			&& rc.bottom - rc.top == rcBmpPart.bottom - rcBmpPart.top \
			&& rcCorners.left == 0 && rcCorners.right == 0 && rcCorners.top == 0 && rcCorners.bottom == 0)
		{
			if (::IntersectRect(&rcTemp, &rcPaint, &rc)) {
				::BitBlt(hDC, rcTemp.left, rcTemp.top, rcTemp.right - rcTemp.left, rcTemp.bottom - rcTemp.top, \
					hCloneDC, rcBmpPart.left + rcTemp.left - rc.left, rcBmpPart.top + rcTemp.top - rc.top, SRCCOPY);
			}
		}
		else
		{
			// middle
			if (!hole) {
				rcDest.left = rc.left + rcCorners.left;
				rcDest.top = rc.top + rcCorners.top;
				rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
				rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
					if (!xtiled && !ytiled) {
						rcDest.right -= rcDest.left;
						rcDest.bottom -= rcDest.top;
						::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
							rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
							rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, \
							rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
					}
					else if (xtiled && ytiled) {
						LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
						LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
						int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
						int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
						for (int j = 0; j < iTimesY; ++j) {
							LONG lDestTop = rcDest.top + lHeight * j;
							LONG lDestBottom = rcDest.top + lHeight * (j + 1);
							LONG lDrawHeight = lHeight;
							if (lDestBottom > rcDest.bottom) {
								lDrawHeight -= lDestBottom - rcDest.bottom;
								lDestBottom = rcDest.bottom;
							}
							for (int i = 0; i < iTimesX; ++i) {
								LONG lDestLeft = rcDest.left + lWidth * i;
								LONG lDestRight = rcDest.left + lWidth * (i + 1);
								LONG lDrawWidth = lWidth;
								if (lDestRight > rcDest.right) {
									lDrawWidth -= lDestRight - rcDest.right;
									lDestRight = rcDest.right;
								}
								::BitBlt(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j, \
									lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC, \
									rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, SRCCOPY);
							}
						}
					}
					else if (xtiled) {
						LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right;
						int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
						for (int i = 0; i < iTimes; ++i) {
							LONG lDestLeft = rcDest.left + lWidth * i;
							LONG lDestRight = rcDest.left + lWidth * (i + 1);
							LONG lDrawWidth = lWidth;
							if (lDestRight > rcDest.right) {
								lDrawWidth -= lDestRight - rcDest.right;
								lDestRight = rcDest.right;
							}
							::StretchBlt(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom,
								hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
								lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
						}
					}
					else { // ytiled
						LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom;
						int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
						for (int i = 0; i < iTimes; ++i) {
							LONG lDestTop = rcDest.top + lHeight * i;
							LONG lDestBottom = rcDest.top + lHeight * (i + 1);
							LONG lDrawHeight = lHeight;
							if (lDestBottom > rcDest.bottom) {
								lDrawHeight -= lDestBottom - rcDest.bottom;
								lDestBottom = rcDest.bottom;
							}
							::StretchBlt(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop,
								hCloneDC, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, \
								rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, lDrawHeight, SRCCOPY);
						}
					}
				}
			}

			// left-top
			if (rcCorners.left > 0 && rcCorners.top > 0) {
				rcDest.left = rc.left;
				rcDest.top = rc.top;
				rcDest.right = rcCorners.left;
				rcDest.bottom = rcCorners.top;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left, rcBmpPart.top, rcCorners.left, rcCorners.top, SRCCOPY);
				}
			}
			// top
			if (rcCorners.top > 0) {
				rcDest.left = rc.left + rcCorners.left;
				rcDest.top = rc.top;
				rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
				rcDest.bottom = rcCorners.top;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left + rcCorners.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - \
						rcCorners.left - rcCorners.right, rcCorners.top, SRCCOPY);
				}
			}
			// right-top
			if (rcCorners.right > 0 && rcCorners.top > 0) {
				rcDest.left = rc.right - rcCorners.right;
				rcDest.top = rc.top;
				rcDest.right = rcCorners.right;
				rcDest.bottom = rcCorners.top;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.right - rcCorners.right, rcBmpPart.top, rcCorners.right, rcCorners.top, SRCCOPY);
				}
			}
			// left
			if (rcCorners.left > 0) {
				rcDest.left = rc.left;
				rcDest.top = rc.top + rcCorners.top;
				rcDest.right = rcCorners.left;
				rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left, rcBmpPart.top + rcCorners.top, rcCorners.left, rcBmpPart.bottom - \
						rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
				}
			}
			// right
			if (rcCorners.right > 0) {
				rcDest.left = rc.right - rcCorners.right;
				rcDest.top = rc.top + rcCorners.top;
				rcDest.right = rcCorners.right;
				rcDest.bottom = rc.bottom - rc.top - rcCorners.top - rcCorners.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.right - rcCorners.right, rcBmpPart.top + rcCorners.top, rcCorners.right, \
						rcBmpPart.bottom - rcBmpPart.top - rcCorners.top - rcCorners.bottom, SRCCOPY);
				}
			}
			// left-bottom
			if (rcCorners.left > 0 && rcCorners.bottom > 0) {
				rcDest.left = rc.left;
				rcDest.top = rc.bottom - rcCorners.bottom;
				rcDest.right = rcCorners.left;
				rcDest.bottom = rcCorners.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left, rcBmpPart.bottom - rcCorners.bottom, rcCorners.left, rcCorners.bottom, SRCCOPY);
				}
			}
			// bottom
			if (rcCorners.bottom > 0) {
				rcDest.left = rc.left + rcCorners.left;
				rcDest.top = rc.bottom - rcCorners.bottom;
				rcDest.right = rc.right - rc.left - rcCorners.left - rcCorners.right;
				rcDest.bottom = rcCorners.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left + rcCorners.left, rcBmpPart.bottom - rcCorners.bottom, \
						rcBmpPart.right - rcBmpPart.left - rcCorners.left - rcCorners.right, rcCorners.bottom, SRCCOPY);
				}
			}
			// right-bottom
			if (rcCorners.right > 0 && rcCorners.bottom > 0) {
				rcDest.left = rc.right - rcCorners.right;
				rcDest.top = rc.bottom - rcCorners.bottom;
				rcDest.right = rcCorners.right;
				rcDest.bottom = rcCorners.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.right - rcCorners.right, rcBmpPart.bottom - rcCorners.bottom, rcCorners.right, \
						rcCorners.bottom, SRCCOPY);
				}
			}
		}
	}

	::SelectObject(hCloneDC, hOldBitmap);
	::DeleteDC(hCloneDC);
}

void CShadowUI::Update(HWND hParent)
{
	if (!m_bIsShowShadow || !(m_Status & SS_VISABLE)) return;
	RECT WndRect;
	GetWindowRect(hParent, &WndRect);
	int nShadWndWid;
	int nShadWndHei;
	if (m_bIsImageMode) {
		if (m_sShadowImage.empty()) return;
		nShadWndWid = WndRect.right - WndRect.left + m_nSize * 2;
		nShadWndHei = WndRect.bottom - WndRect.top + m_nSize * 2;
	}
	else {
		if (m_nSize == 0) return;
		nShadWndWid = WndRect.right - WndRect.left + m_nSize * 2;
		nShadWndHei = WndRect.bottom - WndRect.top + m_nSize * 2;
	}

	// Create the alpha blending bitmap
	BITMAPINFO bmi;        // bitmap header
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = nShadWndWid;
	bmi.bmiHeader.biHeight = nShadWndHei;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = nShadWndWid * nShadWndHei * 4;
	BYTE *pvBits;          // pointer to DIB section
	HBITMAP hbitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void **)&pvBits, NULL, 0);
	if (hbitmap == NULL) {
		GetLastErrorMessage();
	}

	HDC hMemDC = CreateCompatibleDC(NULL);
	if (hMemDC == NULL) {
		GetLastErrorMessage();
	}
	HBITMAP hOriBmp = (HBITMAP)SelectObject(hMemDC, hbitmap);
	if (GetLastError() != 0) {
		GetLastErrorMessage();
	}
	if (m_bIsImageMode)
	{
		RECT rcPaint = { 0, 0, nShadWndWid, nShadWndHei };
		std::shared_ptr<ui::ImageInfo> image_info = ui::GlobalManager::GetImage(m_sShadowImage);
		if (!image_info.get()) return;

		RECT rcBmpPart = { 0 };
		rcBmpPart.right = image_info->GetBitmap()->GetWidth();
		rcBmpPart.bottom = image_info->GetBitmap()->GetHeight();

		RECT corner = m_rcShadowCorner;
		DrawImage(hMemDC, image_info->GetHBitmap(0), rcPaint, rcPaint, rcBmpPart, corner, image_info->IsAlpha(), 0xFF, true, false, false);
	}
	else
	{
		ZeroMemory(pvBits, bmi.bmiHeader.biSizeImage);
		MakeShadow((UINT32 *)pvBits, hParent, &WndRect);
	}

	POINT ptDst;
	if (m_bIsImageMode)
	{
		ptDst.x = WndRect.left - m_nSize;
		ptDst.y = WndRect.top - m_nSize;
	}
	else
	{
		ptDst.x = WndRect.left + m_nxOffset - m_nSize;
		ptDst.y = WndRect.top + m_nyOffset - m_nSize;
	}

	POINT ptSrc = { 0, 0 };
	SIZE WndSize = { nShadWndWid, nShadWndHei };
	BLENDFUNCTION blendPixelFunction = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	MoveWindow(m_hWnd, ptDst.x, ptDst.y, nShadWndWid, nShadWndHei, FALSE);
	BOOL bRet = ::UpdateLayeredWindow(m_hWnd, NULL, &ptDst, &WndSize, hMemDC, &ptSrc, 0, &blendPixelFunction, ULW_ALPHA);
	_ASSERT(bRet); // something was wrong....
	// Delete used resources
	SelectObject(hMemDC, hOriBmp);
	DeleteObject(hbitmap);
	DeleteDC(hMemDC);
}

void CShadowUI::MakeShadow(UINT32 *pShadBits, HWND hParent, RECT *rcParent)
{
	// The shadow algorithm:
	// Get the region of parent window,
	// Apply morphologic erosion to shrink it into the size (ShadowWndSize - Sharpness)
	// Apply modified (with blur effect) morphologic dilation to make the blurred border
	// The algorithm is optimized by assuming parent window is just "one piece" and without "wholes" on it

	// Get the region of parent window,
	HRGN hParentRgn = CreateRectRgn(0, 0, abs(rcParent->right - rcParent->left), abs(rcParent->bottom - rcParent->top));
	GetWindowRgn(hParent, hParentRgn);

	// Determine the Start and end point of each horizontal scan line
	SIZE szParent = { rcParent->right - rcParent->left, rcParent->bottom - rcParent->top };
	SIZE szShadow = { szParent.cx + 2 * m_nSize, szParent.cy + 2 * m_nSize };
	// Extra 2 lines (set to be empty) in ptAnchors are used in dilation
	int nAnchors = max(szParent.cy, szShadow.cy);	// # of anchor points pares
	int(*ptAnchors)[2] = new int[nAnchors + 2][2];
	int(*ptAnchorsOri)[2] = new int[szParent.cy][2];	// anchor points, will not modify during erosion
	ptAnchors[0][0] = szParent.cx;
	ptAnchors[0][1] = 0;
	ptAnchors[nAnchors + 1][0] = szParent.cx;
	ptAnchors[nAnchors + 1][1] = 0;
	if (m_nSize > 0)
	{
		// Put the parent window anchors at the center
		for (int i = 0; i < m_nSize; i++)
		{
			ptAnchors[i + 1][0] = szParent.cx;
			ptAnchors[i + 1][1] = 0;
			ptAnchors[szShadow.cy - i][0] = szParent.cx;
			ptAnchors[szShadow.cy - i][1] = 0;
		}
		ptAnchors += m_nSize;
	}
	for (int i = 0; i < szParent.cy; i++)
	{
		// find start point
		int j;
		for (j = 0; j < szParent.cx; j++)
		{
			if (PtInRegion(hParentRgn, j, i))
			{
				ptAnchors[i + 1][0] = j + m_nSize;
				ptAnchorsOri[i][0] = j;
				break;
			}
		}

		if (j >= szParent.cx)	// Start point not found
		{
			ptAnchors[i + 1][0] = szParent.cx;
			ptAnchorsOri[i][1] = 0;
			ptAnchors[i + 1][0] = szParent.cx;
			ptAnchorsOri[i][1] = 0;
		}
		else
		{
			// find end point
			for (j = szParent.cx - 1; j >= ptAnchors[i + 1][0]; j--)
			{
				if (PtInRegion(hParentRgn, j, i))
				{
					ptAnchors[i + 1][1] = j + 1 + m_nSize;
					ptAnchorsOri[i][1] = j + 1;
					break;
				}
			}
		}
	}

	if (m_nSize > 0)
		ptAnchors -= m_nSize;	// Restore pos of ptAnchors for erosion
	int(*ptAnchorsTmp)[2] = new int[nAnchors + 2][2];	// Store the result of erosion
	// First and last line should be empty
	ptAnchorsTmp[0][0] = szParent.cx;
	ptAnchorsTmp[0][1] = 0;
	ptAnchorsTmp[nAnchors + 1][0] = szParent.cx;
	ptAnchorsTmp[nAnchors + 1][1] = 0;
	int nEroTimes = 0;
	// morphologic erosion
	for (int i = 0; i < m_nSharpness - m_nSize; i++)
	{
		nEroTimes++;
		//ptAnchorsTmp[1][0] = szParent.cx;
		//ptAnchorsTmp[1][1] = 0;
		//ptAnchorsTmp[szParent.cy + 1][0] = szParent.cx;
		//ptAnchorsTmp[szParent.cy + 1][1] = 0;
		for (int j = 1; j < nAnchors + 1; j++)
		{
			ptAnchorsTmp[j][0] = max(ptAnchors[j - 1][0], max(ptAnchors[j][0], ptAnchors[j + 1][0])) + 1;
			ptAnchorsTmp[j][1] = min(ptAnchors[j - 1][1], min(ptAnchors[j][1], ptAnchors[j + 1][1])) - 1;
		}
		// Exchange ptAnchors and ptAnchorsTmp;
		int(*ptAnchorsXange)[2] = ptAnchorsTmp;
		ptAnchorsTmp = ptAnchors;
		ptAnchors = ptAnchorsXange;
	}

	// morphologic dilation
	ptAnchors += (m_nSize < 0 ? -m_nSize : 0) + 1;	// now coordinates in ptAnchors are same as in shadow window
	// Generate the kernel
	int nKernelSize = m_nSize > m_nSharpness ? m_nSize : m_nSharpness;
	int nCenterSize = m_nSize > m_nSharpness ? (m_nSize - m_nSharpness) : 0;
	UINT32 *pKernel = new UINT32[(2 * nKernelSize + 1) * (2 * nKernelSize + 1)];
	UINT32 *pKernelIter = pKernel;
	for (int i = 0; i <= 2 * nKernelSize; i++)
	{
		for (int j = 0; j <= 2 * nKernelSize; j++)
		{
			double dLength = sqrt((i - nKernelSize) * (i - nKernelSize) + (j - nKernelSize) * (double)(j - nKernelSize));
			if (dLength < nCenterSize)
				*pKernelIter = m_nDarkness << 24 | PreMultiply(m_Color, m_nDarkness);
			else if (dLength <= nKernelSize)
			{
				UINT32 nFactor = ((UINT32)((1 - (dLength - nCenterSize) / (m_nSharpness + 1)) * m_nDarkness));
				*pKernelIter = nFactor << 24 | PreMultiply(m_Color, nFactor);
			}
			else
				*pKernelIter = 0;
			//TRACE("%d ", *pKernelIter >> 24);
			pKernelIter++;
		}
		//TRACE("\n");
	}
	// Generate blurred border
	for (int i = nKernelSize; i < szShadow.cy - nKernelSize; i++)
	{
		int j;
		if (ptAnchors[i][0] < ptAnchors[i][1])
		{

			// Start of line
			for (j = ptAnchors[i][0];
				j < min(max(ptAnchors[i - 1][0], ptAnchors[i + 1][0]) + 1, ptAnchors[i][1]);
				j++)
			{
				for (int k = 0; k <= 2 * nKernelSize; k++)
				{
					UINT32 *pPixel = pShadBits +
						(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
					UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
					for (int l = 0; l <= 2 * nKernelSize; l++)
					{
						if (*pPixel < *pKernelPixel)
							*pPixel = *pKernelPixel;
						pPixel++;
						pKernelPixel++;
					}
				}
			}	// for() start of line

			// End of line
			for (j = max(j, min(ptAnchors[i - 1][1], ptAnchors[i + 1][1]) - 1);
				j < ptAnchors[i][1];
				j++)
			{
				for (int k = 0; k <= 2 * nKernelSize; k++)
				{
					UINT32 *pPixel = pShadBits +
						(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
					UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
					for (int l = 0; l <= 2 * nKernelSize; l++)
					{
						if (*pPixel < *pKernelPixel)
							*pPixel = *pKernelPixel;
						pPixel++;
						pKernelPixel++;
					}
				}
			}	// for() end of line

		}
	}	// for() Generate blurred border

	// Erase unwanted parts and complement missing
	UINT32 clCenter = m_nDarkness << 24 | PreMultiply(m_Color, m_nDarkness);
	for (int i = min(nKernelSize, max(m_nSize - m_nyOffset, 0));
		i < max(szShadow.cy - nKernelSize, min(szParent.cy + m_nSize - m_nyOffset, szParent.cy + 2 * m_nSize));
		i++)
	{
		UINT32 *pLine = pShadBits + (szShadow.cy - i - 1) * szShadow.cx;
		if (i - m_nSize + m_nyOffset < 0 || i - m_nSize + m_nyOffset >= szParent.cy)	// Line is not covered by parent window
		{
			for (int j = ptAnchors[i][0]; j < ptAnchors[i][1]; j++)
			{
				*(pLine + j) = clCenter;
			}
		}
		else
		{
			for (int j = ptAnchors[i][0];
				j < min(ptAnchorsOri[i - m_nSize + m_nyOffset][0] + m_nSize - m_nxOffset, ptAnchors[i][1]);
				j++)
				*(pLine + j) = clCenter;
			for (int j = max(ptAnchorsOri[i - m_nSize + m_nyOffset][0] + m_nSize - m_nxOffset, 0);
				j < min(ptAnchorsOri[i - m_nSize + m_nyOffset][1] + m_nSize - m_nxOffset, szShadow.cx);
				j++)
				*(pLine + j) = 0;
			for (int j = max(ptAnchorsOri[i - m_nSize + m_nyOffset][1] + m_nSize - m_nxOffset, ptAnchors[i][0]);
				j < ptAnchors[i][1];
				j++)
				*(pLine + j) = clCenter;
		}
	}

	// Delete used resources
	delete[](ptAnchors - (m_nSize < 0 ? -m_nSize : 0) - 1);
	delete[] ptAnchorsTmp;
	delete[] ptAnchorsOri;
	delete[] pKernel;
	DeleteObject(hParentRgn);
}

void CShadowUI::ShowShadow(bool bShow)
{
	m_bIsShowShadow = bShow;
}

bool CShadowUI::IsShowShadow() const
{
	return m_bIsShowShadow;
}


void CShadowUI::DisableShadow(bool bDisable) {


	m_bIsDisableShadow = bDisable;
	if (m_hWnd != NULL) {

		if (m_bIsDisableShadow) {
			::ShowWindow(m_hWnd, SW_HIDE);
		}
		else {
			// Determine the initial show state of shadow according to parent window's state
			LONG lParentStyle = GetWindowLongPtr(GetParent(m_hWnd), GWL_STYLE);


			if (!(WS_VISIBLE & lParentStyle))	// Parent invisible
				m_Status = SS_ENABLED;
			else if ((WS_MAXIMIZE | WS_MINIMIZE) & lParentStyle)	// Parent visible but does not need shadow
				m_Status = SS_ENABLED | SS_PARENTVISIBLE;
			else	// Show the shadow
			{
				m_Status = SS_ENABLED | SS_VISABLE | SS_PARENTVISIBLE;

			}


			if ((WS_VISIBLE & lParentStyle) && !((WS_MAXIMIZE | WS_MINIMIZE) & lParentStyle))// Parent visible && no maxsize or min size
			{
				::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
				Update(GetParent(m_hWnd));
			}



		}


	}

}
////TODO shadow disnable fix////
bool CShadowUI::IsDisableShadow() const {

	return m_bIsDisableShadow;
}

bool CShadowUI::SetSize(int NewSize)
{
	if (NewSize > 35 || NewSize < -35)
		return false;

	ui::DpiManager::GetInstance()->ScaleInt(NewSize);
	m_nSize = (signed char)NewSize;
	if (m_hWnd != NULL && (SS_VISABLE & m_Status))
		Update(GetParent(m_hWnd));
	return true;
}

bool CShadowUI::SetSharpness(unsigned int NewSharpness)
{
	if (NewSharpness > 35)
		return false;

	m_nSharpness = (unsigned char)NewSharpness;
	if (m_hWnd != NULL && (SS_VISABLE & m_Status))
		Update(GetParent(m_hWnd));
	return true;
}

bool CShadowUI::SetDarkness(unsigned int NewDarkness)
{
	if (NewDarkness > 255)
		return false;

	m_nDarkness = (unsigned char)NewDarkness;
	if (m_hWnd != NULL && (SS_VISABLE & m_Status))
		Update(GetParent(m_hWnd));
	return true;
}

bool CShadowUI::SetPosition(int NewXOffset, int NewYOffset)
{
	if (NewXOffset > 35 || NewXOffset < -35 ||
		NewYOffset > 35 || NewYOffset < -35)
		return false;

	m_nxOffset = (signed char)NewXOffset;
	m_nyOffset = (signed char)NewYOffset;
	if (m_hWnd != NULL && (SS_VISABLE & m_Status))
		Update(GetParent(m_hWnd));
	return true;
}

bool CShadowUI::SetColor(COLORREF NewColor)
{
	m_Color = NewColor;
	if (m_hWnd != NULL && (SS_VISABLE & m_Status))
		Update(GetParent(m_hWnd));
	return true;
}

bool CShadowUI::SetImage(LPCTSTR szImage)
{
	if (szImage == NULL)
		return false;

	m_bIsImageMode = true;
	m_sShadowImage = szImage;
	int dpi = ui::DpiManager::GetInstance()->GetScale();
	if (dpi != 100)
	{
		std::wstring tempImageFullPath = m_sShadowImage;
		std::wstring scale_tail = L"@" + std::to_wstring(dpi);
		int nFind = tempImageFullPath.rfind(L".");
		if (nFind != -1)
		{
			tempImageFullPath.insert(nFind, scale_tail);
			if (::PathFileExists(tempImageFullPath.c_str()))
			{
				m_sShadowImage = tempImageFullPath;
			}
			else
			{
				//这边到时要断言  本地 125和150的图片没有
			}
		}
	}
	if (m_hWnd != NULL && (SS_VISABLE & m_Status))
		Update(GetParent(m_hWnd));

	return true;
}

bool CShadowUI::SetShadowCorner(RECT rcCorner)
{
	if (rcCorner.left < 0 || rcCorner.top < 0 || rcCorner.right < 0 || rcCorner.bottom < 0) return false;

	ui::DpiManager::GetInstance()->ScaleRect(rcCorner);
	m_rcShadowCorner = rcCorner;
	if (m_hWnd != NULL && (SS_VISABLE & m_Status)) {
		Update(GetParent(m_hWnd));
	}

	return true;
}

bool CShadowUI::CopyShadow(CShadowUI* pShadow)
{
	if (m_bIsImageMode) {
		pShadow->SetImage(m_sShadowImage.c_str());
		pShadow->SetShadowCorner(m_rcShadowCorner);
		pShadow->SetSize((int)m_nSize);
	}
	else {
		pShadow->SetSize((int)m_nSize);
		pShadow->SetSharpness((unsigned int)m_nSharpness);
		pShadow->SetDarkness((unsigned int)m_nDarkness);
		pShadow->SetColor(m_Color);
		pShadow->SetPosition((int)m_nxOffset, (int)m_nyOffset);
	}

	pShadow->ShowShadow(m_bIsShowShadow);
	return true;
}


void CShadowUI::SetHideShadow(bool hide/* = true*/)
{
	if (::IsWindow(m_hWnd))
	{
		if (hide)
		{
			::ShowWindow(m_hWnd, SW_HIDE);
		}
		else
		{
			::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
		}
	}
}

}
