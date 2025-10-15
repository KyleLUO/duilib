#include "StdAfx.h"
#include <d2d1helper.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwrite_1.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "dxguid.lib")
#include "DirectXHelp.h"
#include "base/util/string_util.h"
#include "third_party/tinyxml/tinyxml.h"
#include "./Effects/ClearAlphaEffect.h"
#include "./Effects/RichTextFixAlphaEffect.h"
#include "TextRender.h"

#define D2D_USE_MEMDC 

static int g_iFontID = 30000;

namespace ui {

	Microsoft::WRL::ComPtr<IDWriteFactory1>	RenderContext_D3D::m_d2dWriteFactory;
	Microsoft::WRL::ComPtr<IWICImagingFactory> RenderContext_D3D::m_widImagingFactory;

static inline void DrawFunction(HDC hDC, bool bTransparent, UiRect rcDest, HDC hdcSrc, UiRect rcSrc, bool bAlphaChannel, int uFade)
{
	if (bTransparent || bAlphaChannel || uFade < 255
		|| (rcSrc.GetWidth() == rcDest.GetWidth() && rcSrc.GetHeight() == rcDest.GetHeight())) {
		BLENDFUNCTION ftn = { AC_SRC_OVER, 0, uFade, AC_SRC_ALPHA };
		::AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.GetWidth(), rcDest.GetHeight(),
			hdcSrc, rcSrc.left, rcSrc.top, rcSrc.GetWidth(), rcSrc.GetHeight(), ftn);
	}
	else 
	{
		::SetStretchBltMode(hDC, STRETCH_HALFTONE);
		::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.GetWidth(), rcDest.GetHeight(),
			hdcSrc, rcSrc.left, rcSrc.top, rcSrc.GetWidth(), rcSrc.GetHeight(), SRCCOPY);
	}
}

RenderContext_GdiPlus::RenderContext_GdiPlus()
	: m_hDC(NULL)
	, m_hOldBitmap(NULL)
	, m_bTransparent(false)
{
	HDC hDC = ::GetDC(NULL);
	m_hDC = ::CreateCompatibleDC(hDC);
	::ReleaseDC(NULL, hDC);
	assert(m_hDC);
}

RenderContext_GdiPlus::~RenderContext_GdiPlus()
{
	if (m_hOldBitmap != NULL)
	{
		::SelectObject(m_hDC, m_hOldBitmap);
		m_hOldBitmap = NULL;
	}

	if (m_hDC != NULL)
	{
		::DeleteDC(m_hDC);
		m_hDC = NULL;
	}
}

HDC RenderContext_GdiPlus::GetDC()
{
	return m_hDC;
}

bool RenderContext_GdiPlus::Resize(int width, int height, bool flipBItmap)
{
	assert(m_hDC);
	if (m_bitmap.GetWidth() == width && m_bitmap.GetHeight() == height)
		return false;

	if (m_hOldBitmap != NULL)
	{
		::SelectObject(m_hDC, m_hOldBitmap);
	}

	bool ret = m_bitmap.Init(m_hDC, width, height, flipBItmap);
	m_hOldBitmap = (HBITMAP)::SelectObject(m_hDC, m_bitmap.GetBitmap());
	return ret;
}

void RenderContext_GdiPlus::Clear()
{
	assert(m_hDC);
	m_bitmap.Clear();
}

std::unique_ptr<ui::IRenderContext> RenderContext_GdiPlus::Clone()
{
	std::unique_ptr<ui::IRenderContext> pClone = std::make_unique<ui::RenderContext_GdiPlus>();
	pClone->Resize(GetWidth(), GetHeight());
	pClone->BitBlt(0, 0, GetWidth(), GetHeight(), m_hDC);
	return pClone;
}

HBITMAP RenderContext_GdiPlus::DetachBitmap()
{
	assert(m_hDC && m_hOldBitmap);
	assert(m_bitmap.GetHeight() != 0 && m_bitmap.GetWidth() != 0);
	if (m_hOldBitmap == NULL)
		return NULL;

	::SelectObject(m_hDC, m_hOldBitmap);
	return m_bitmap.DetachBitmap();
}

BYTE* RenderContext_GdiPlus::GetBits()
{
	return m_bitmap.GetBits();
}

int RenderContext_GdiPlus::GetWidth()
{
	return m_bitmap.GetWidth();
}

int RenderContext_GdiPlus::GetHeight()
{
	return m_bitmap.GetHeight();
}

void RenderContext_GdiPlus::ClearAlpha(const UiRect& rcDirty, int alpha)
{
	m_bitmap.ClearAlpha(rcDirty, alpha);
}

void RenderContext_GdiPlus::RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding, int alpha)
{
	m_bitmap.RestoreAlpha(rcDirty, rcShadowPadding, alpha);
}

bool RenderContext_GdiPlus::IsRenderTransparent() const
{
	return m_bTransparent;
}

bool RenderContext_GdiPlus::SetRenderTransparent(bool bTransparent)
{
	bool oldValue = m_bTransparent;
	m_bTransparent = bTransparent;
	return oldValue;
}

void RenderContext_GdiPlus::Save()
{
	m_saveDC = SaveDC(m_hDC);
}

void RenderContext_GdiPlus::Restore()
{
	RestoreDC(m_hDC, m_saveDC);
}

CPoint RenderContext_GdiPlus::OffsetWindowOrg(CPoint ptOffset)
{
	CPoint ptOldWindowOrg;
	GetWindowOrgEx(m_hDC, &ptOldWindowOrg);
	ptOffset.Offset(ptOldWindowOrg.x, ptOldWindowOrg.y);
	::SetWindowOrgEx(m_hDC, ptOffset.x, ptOffset.y, NULL);
	return ptOldWindowOrg;
}

CPoint RenderContext_GdiPlus::SetWindowOrg(CPoint ptOffset)
{
	CPoint ptOldWindowOrg;
	GetWindowOrgEx(m_hDC, &ptOldWindowOrg);
	::SetWindowOrgEx(m_hDC, ptOffset.x, ptOffset.y, NULL);
	return ptOldWindowOrg;
}

CPoint RenderContext_GdiPlus::GetWindowOrg() const
{
	CPoint ptWindowOrg;
	GetWindowOrgEx(m_hDC, &ptWindowOrg);
	return ptWindowOrg;
}

void RenderContext_GdiPlus::SetClip(const UiRect& rc)
{
	m_clip.CreateClip(m_hDC, rc);
}

void RenderContext_GdiPlus::SetRoundClip(const UiRect& rc, int width, int height)
{
	m_clip.CreateRoundClip(m_hDC, rc, width, height);
}

void RenderContext_GdiPlus::ClearClip()
{
	m_clip.ClearClip(m_hDC);
}

HRESULT RenderContext_GdiPlus::BitBlt(int x, int y, int cx, int cy, HDC hdcSrc, int xSrc /*= 0*/, int yScr /*= 0*/, DWORD rop /*= SRCCOPY*/)
{
	return ::BitBlt(m_hDC, x, y, cx, cy, hdcSrc, xSrc, yScr, rop);
}

bool RenderContext_GdiPlus::AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, HDC hdcSrc, int xSrc, int yScr, int widthSrc, int heightSrc, BYTE uFade /*= 255*/)
{
	BLENDFUNCTION bf = { AC_SRC_OVER, 0, uFade, AC_SRC_ALPHA };
	return (TRUE == ::AlphaBlend(m_hDC, xDest, yDest, widthDest, heightDest, hdcSrc, xSrc, yScr, widthSrc, heightSrc, bf));
}

HRESULT RenderContext_GdiPlus::BitBlt(int x, int y, int cx, int cy, IRenderContext *srcCtx, int xSrc /*= 0*/, int yScr /*= 0*/, DWORD rop /*= SRCCOPY*/)
{
	return ::BitBlt(m_hDC, x, y, cx, cy, srcCtx->GetDC(), xSrc, yScr, rop);
}

void RenderContext_GdiPlus::SetTransform()
{
	;
}

void RenderContext_GdiPlus::ResetTransform()
{
	;
}
void RenderContext_GdiPlus::TranslateTransform(int dx, int dy)
{

}
void RenderContext_GdiPlus::RotateTransform(float angle)
{

}

bool RenderContext_GdiPlus::AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, IRenderContext *srcCtx, int xSrc, int yScr, int widthSrc, int heightSrc, BYTE uFade /*= 255*/)
{
	BLENDFUNCTION bf = { AC_SRC_OVER, 0, uFade, AC_SRC_ALPHA };
	return (TRUE == ::AlphaBlend(m_hDC, xDest, yDest, widthDest, heightDest, srcCtx->GetDC(), xSrc, yScr, widthSrc, heightSrc, bf));
}

void RenderContext_GdiPlus::SetResourceRoot(const std::wstring &path)
{
	;
}

void RenderContext_GdiPlus::BeginPaint(const UiRect* rcPaint)
{
	;
}

void RenderContext_GdiPlus::EndPaint()
{
	;
}

bool RenderContext_GdiPlus::UseDirectX()
{
	return false;
}

ID2D1RenderTarget* RenderContext_GdiPlus::GetRender()
{
	throw RenderNotImplemented();
}

void RenderContext_GdiPlus::DrawImage(const UiRect& rcPaint, HBITMAP hBitmap, bool bAlphaChannel,
	const UiRect& rcImageDest, const UiRect& rcImageSource, const UiRect& rcCorners, BYTE uFade /*= 255*/, 
	bool xtiled /*= false*/, bool ytiled /*= false*/)
{
	UiRect rcTestTemp;
	if (!::IntersectRect(&rcTestTemp, &rcImageDest, &rcPaint)) return;

	assert(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);

	if (hBitmap == NULL) return;

	HDC hCloneDC = ::CreateCompatibleDC(m_hDC);
	HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
	int stretchBltMode = ::SetStretchBltMode(m_hDC, HALFTONE);

	UiRect rcTemp;
	UiRect rcSource;
	UiRect rcDest;
	UiRect rcDpiCorner = rcCorners;
	//DpiManager::GetInstance()->ScaleRect(rcDpiCorner);

	// middle
	rcDest.left = rcImageDest.left + rcDpiCorner.left;
	rcDest.top = rcImageDest.top + rcDpiCorner.top;
	rcDest.right = rcImageDest.right - rcDpiCorner.right;
	rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;
	rcSource.left = rcImageSource.left + rcDpiCorner.left;
	rcSource.top = rcImageSource.top + rcDpiCorner.top;
	rcSource.right = rcImageSource.right - rcDpiCorner.right;
	rcSource.bottom = rcImageSource.bottom - rcDpiCorner.bottom;
	if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
		if (!xtiled && !ytiled) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
		else if (xtiled && ytiled) {
			LONG lWidth = rcImageSource.right - rcImageSource.left - rcDpiCorner.left - rcDpiCorner.right;
			LONG lHeight = rcImageSource.bottom - rcImageSource.top - rcDpiCorner.top - rcDpiCorner.bottom;
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
					rcDest.left = rcDest.left + lWidth * i;
					rcDest.top = rcDest.top + lHeight * j;
					rcDest.right = rcDest.left + lDestRight - lDestLeft;
					rcDest.bottom = rcDest.top + lDestBottom - lDestTop;
					rcSource.left = rcImageSource.left + rcDpiCorner.left;
					rcSource.top = rcImageSource.top + rcDpiCorner.top;
					rcSource.right = rcSource.left + lDrawWidth;
					rcSource.bottom = rcSource.top + lDrawHeight;
					DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
				}
			}
		}
		else if (xtiled) {
			LONG lWidth = rcImageSource.right - rcImageSource.left - rcDpiCorner.left - rcDpiCorner.right;
			int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
			for (int i = 0; i < iTimes; ++i) {
				LONG lDestLeft = rcDest.left + lWidth * i;
				LONG lDestRight = rcDest.left + lWidth * (i + 1);
				LONG lDrawWidth = lWidth;
				if (lDestRight > rcDest.right) {
					lDrawWidth -= lDestRight - rcDest.right;
					lDestRight = rcDest.right;
				}
				rcDest.left = lDestLeft;
				rcDest.top = rcDest.top;
				rcDest.right = lDestRight;
				rcDest.bottom = rcDest.top + rcDest.bottom;
				rcSource.left = rcImageSource.left + rcDpiCorner.left;
				rcSource.top = rcImageSource.top + rcDpiCorner.top;
				rcSource.right = rcSource.left + lDrawWidth;
				rcSource.bottom = rcImageSource.bottom - rcDpiCorner.bottom;
				DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
			}
		}
		else { // ytiled
			LONG lHeight = rcImageSource.bottom - rcImageSource.top - rcDpiCorner.top - rcDpiCorner.bottom;
			int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
			for (int i = 0; i < iTimes; ++i) {
				LONG lDestTop = rcDest.top + lHeight * i;
				LONG lDestBottom = rcDest.top + lHeight * (i + 1);
				LONG lDrawHeight = lHeight;
				if (lDestBottom > rcDest.bottom) {
					lDrawHeight -= lDestBottom - rcDest.bottom;
					lDestBottom = rcDest.bottom;
				}
				rcDest.left = rcDest.left;
				rcDest.top = rcDest.top + lHeight * i;
				rcDest.right = rcDest.left + rcDest.right;
				rcDest.bottom = rcDest.top + lDestBottom - lDestTop;
				rcSource.left = rcImageSource.left + rcDpiCorner.left;
				rcSource.top = rcImageSource.top + rcDpiCorner.top;
				rcSource.right = rcImageSource.right - rcDpiCorner.right;
				rcSource.bottom = rcSource.top + lDrawHeight;
				DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
			}
		}
	}

	// left-top
	if (rcDpiCorner.left > 0 && rcDpiCorner.top > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.left + rcDpiCorner.left;
		rcSource.bottom = rcImageSource.top + rcDpiCorner.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// top
	if (rcDpiCorner.top > 0) {
		rcDest.left = rcImageDest.left + rcDpiCorner.left;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.right - rcDpiCorner.right;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.left + rcDpiCorner.left;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.right - rcDpiCorner.right;
		rcSource.bottom = rcImageSource.top + rcDpiCorner.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// right-top
	if (rcDpiCorner.right > 0 && rcDpiCorner.top > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.right - rcDpiCorner.right;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.top + rcDpiCorner.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// left
	if (rcDpiCorner.left > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.top + rcDpiCorner.top;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.top + rcDpiCorner.top;
		rcSource.right = rcImageSource.left + rcDpiCorner.left;
		rcSource.bottom = rcImageSource.bottom - rcDpiCorner.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// right
	if (rcDpiCorner.right > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.top + rcDpiCorner.top;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;
		rcSource.left = rcImageSource.right - rcDpiCorner.right;
		rcSource.top = rcImageSource.top + rcDpiCorner.top;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.bottom - rcDpiCorner.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// left-bottom
	if (rcDpiCorner.left > 0 && rcDpiCorner.bottom > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.bottom - rcDpiCorner.bottom;
		rcSource.right = rcImageSource.left + rcDpiCorner.left;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// bottom
	if (rcDpiCorner.bottom > 0) {
		rcDest.left = rcImageDest.left + rcDpiCorner.left;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.right - rcDpiCorner.right;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.left + rcDpiCorner.left;
		rcSource.top = rcImageSource.bottom - rcDpiCorner.bottom;
		rcSource.right = rcImageSource.right - rcDpiCorner.right;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// right-bottom
	if (rcDpiCorner.right > 0 && rcDpiCorner.bottom > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.right - rcDpiCorner.right;
		rcSource.top = rcImageSource.bottom - rcDpiCorner.bottom;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}

	::SetStretchBltMode(m_hDC, stretchBltMode);
	::SelectObject(hCloneDC, hOldBitmap);
	::DeleteDC(hCloneDC);
}

void RenderContext_GdiPlus::DrawColor(const UiRect& rc, DWORD dwColor, BYTE uFade)
{
	DWORD dwNewColor = dwColor;
	if (uFade < 255) {
		int alpha = dwColor >> 24;
		dwNewColor = dwColor % 0xffffff;
		alpha *= double(uFade) / 255;
		dwNewColor += alpha << 24;
	}

	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Color color(dwNewColor);
	Gdiplus::SolidBrush brush(color);
	Gdiplus::RectF rcFill(rc.left, rc.top, rc.GetWidth(), rc.GetHeight());
	graphics.FillRectangle(&brush, rcFill);
}

void RenderContext_GdiPlus::DrawColor(const UiRect& rc, const std::wstring& colorStr, BYTE uFade)
{
	if (colorStr.empty()) {
		return;
	}

	DWORD dwColor = GlobalManager::GetTextColor(colorStr);
	DrawColor(rc, dwColor, uFade);
}

void RenderContext_GdiPlus::DrawGradientColor(const UiRect& rc, const std::wstring& colorStr, const std::wstring& colorStr1, const int mode)
{
	DWORD dwColor = GlobalManager::GetTextColor(colorStr);
	DWORD dwColor1 = GlobalManager::GetTextColor(colorStr1);

	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Color color(dwColor);
	Gdiplus::Color color1(dwColor1);
	Gdiplus::RectF rcFill(rc.left, rc.top, rc.GetWidth(), rc.GetHeight());
	Gdiplus::LinearGradientBrush brush(rcFill, color, color1, (Gdiplus::LinearGradientMode)mode);
	graphics.FillRectangle(&brush, rcFill);
}

void RenderContext_GdiPlus::DrawLine(const UiRect& rc, int nSize, DWORD dwPenColor)
{
	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Pen pen(Gdiplus::Color(dwPenColor), (Gdiplus::REAL)nSize);
	graphics.DrawLine(&pen, Gdiplus::Point(rc.left, rc.top), Gdiplus::Point(rc.right, rc.bottom));
}

void RenderContext_GdiPlus::DrawLine(const IPen* pen, int x1, int y1, int x2, int y2)
{
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	graphics.DrawLine(((Pen_GdiPlus*)pen)->GetPen(), x1, y1, x2, y2);
}

void RenderContext_GdiPlus::DrawBezier(const IPen* pen, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	graphics.DrawBezier(((Pen_GdiPlus*)pen)->GetPen(), x1, y1, x2, y2, x3, y3, x4, y4);
}

void RenderContext_GdiPlus::DrawRect(const UiRect& rc, int nSize, DWORD dwPenColor)
{
	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Pen pen(Gdiplus::Color(dwPenColor), (Gdiplus::REAL)nSize);
	graphics.DrawRectangle(&pen, rc.left, rc.top, rc.GetWidth(), rc.GetHeight());
}


void RenderContext_GdiPlus::DrawRoundRect(const UiRect& rc, const CSize& roundSize, int nSize, DWORD dwPenColor)
{
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	Gdiplus::Pen pen(Gdiplus::Color(dwPenColor), (Gdiplus::REAL)nSize);

	// 裁剪区域不能作画，导致边框有时不全，往里收缩一个像素
	// UiRect rcInflate = rc;
	// rcInflate.Inflate({ -1, -1, -1, -1 });

	Gdiplus::GraphicsPath pPath;
	pPath.AddArc(rc.left, rc.top, roundSize.cx, roundSize.cy, 180, 90);
	pPath.AddLine(rc.left + roundSize.cx, rc.top, rc.right - roundSize.cx, rc.top);
	pPath.AddArc(rc.right - roundSize.cx, rc.top, roundSize.cx, roundSize.cy, 270, 90);
	pPath.AddLine(rc.right, rc.top + roundSize.cy, rc.right, rc.bottom - roundSize.cy);
	pPath.AddArc(rc.right - roundSize.cx, rc.bottom - roundSize.cy, roundSize.cx, roundSize.cy, 0, 90);
	pPath.AddLine(rc.right - roundSize.cx, rc.bottom, rc.left + roundSize.cx, rc.bottom);
	pPath.AddArc(rc.left, rc.bottom - roundSize.cy, roundSize.cx, roundSize.cy, 90, 90);
	pPath.AddLine(rc.left, rc.bottom - roundSize.cy, rc.left, rc.top + roundSize.cy);
	pPath.CloseFigure();

	graphics.DrawPath(&pen, &pPath);
}

void RenderContext_GdiPlus::DrawText(const UiRect& rc, const std::wstring& strText, DWORD dwTextColor, const std::wstring& strFontId, UINT uStyle, BYTE uFade /*= 255*/, bool bLineLimit /*= false*/)
{
	assert(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);
	if (strText.empty()) return;

	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Font font(m_hDC, GlobalManager::GetFont(strFontId));
	Gdiplus::RectF rcPaint((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)(rc.right - rc.left), (Gdiplus::REAL)(rc.bottom - rc.top));
	int alpha = dwTextColor >> 24;
	uFade *= double(alpha) / 255;
	if (uFade == 255) {
		uFade = 254;
	}
	Gdiplus::SolidBrush tBrush(Gdiplus::Color(uFade, GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));

	Gdiplus::StringFormat stringFormat = Gdiplus::StringFormat::GenericTypographic();
	if ((uStyle & DT_END_ELLIPSIS) != 0) {
		stringFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
	}
	if ((uStyle & DT_PATH_ELLIPSIS) != 0) {
		stringFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisPath);
	}

	int formatFlags = 0;
	if ((uStyle & DT_NOCLIP) != 0) {
		formatFlags |= Gdiplus::StringFormatFlagsNoClip;
	}
	if ((uStyle & DT_SINGLELINE) != 0) {
		formatFlags |= Gdiplus::StringFormatFlagsNoWrap;
	}
	if (bLineLimit) {
		formatFlags |= Gdiplus::StringFormatFlagsLineLimit;
	}
	stringFormat.SetFormatFlags(formatFlags);

	if ((uStyle & DT_LEFT) != 0) {
		stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
	}
	else if ((uStyle & DT_CENTER) != 0) {
		stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
	}
	else if ((uStyle & DT_RIGHT) != 0) {
		stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
	}
	else {
		stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
	}

	if ((uStyle & DT_TOP) != 0) {
		stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
	}
	else if ((uStyle & DT_VCENTER) != 0) {
		TFontInfo* fontInfo = GlobalManager::GetTFontInfo(strFontId);
		if (fontInfo->sFontName == L"新宋体") {
			if (rcPaint.Height >= fontInfo->iSize + 2) {
				rcPaint.Offset(0, 1);
			}
		}
		stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	}
	else if ((uStyle & DT_BOTTOM) != 0) {
		stringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
	}
	else {
		stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
	}

	graphics.DrawString(strText.c_str(), (int)strText.length(), &font, rcPaint, &stringFormat, &tBrush);
}


// 考虑到在xml编辑器中使用<>符号不方便，可以使用{}符号代替
// 支持标签嵌套（如<l><b>text</b></l>），但是交叉嵌套是应该避免的（如<l><b>text</l></b>）
// The string formatter supports a kind of "mini-html" that consists of various short tags:
//
//   Bold:             <b>text</b>
//   Color:            <c #xxxxxx>text</c>  where x = RGB in hex
//   Font:             <f x>text</f>        where x = font id
//   Italic:           <i>text</i>
//   Image:            <i x y z>            where x = image name and y = imagelist num and z(optional) = imagelist id
//   Link:             <a x>text</a>        where x(optional) = link content, normal like app:notepad or http:www.xxx.com
//   NewLine           <n>                  
//   Paragraph:        <p x>text</p>        where x = extra pixels indent in p
//   Raw Text:         <r>text</r>
//   Selected:         <s>text</s>
//   Underline:        <u>text</u>
//   X Indent:         <x i>                where i = hor indent in pixels
//   Y Indent:         <y i>                where i = ver indent in pixels 
void RenderContext_GdiPlus::DrawHtmlText(const UiRect& rc_item, const std::wstring& strText, DWORD dwTextColor, const std::wstring& strFontId, UINT uStyle, RECT* prcLinks)
{
	UiRect rc = rc_item;
	
	assert(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);
	if (strText.empty()) return;

	if (::IsRectEmpty(&rc)) return;

	bool bDraw = (uStyle & DT_CALCRECT) == 0;

	ui::CStdPtrArray aFontArray(10);
	ui::CStdPtrArray aColorArray(10);
	ui::CStdPtrArray aPIndentArray(10);

	//control父控件中已经裁剪和移动了窗口原点，这里不需要再设置
// 	RECT rcClip = { 0 };
// 	::GetClipBox(m_hDC, &rcClip);
// 	HRGN hOldRgn = ::CreateRectRgnIndirect(&rcClip);
// 	HRGN hRgn = ::CreateRectRgnIndirect(&rc);
// 	if (bDraw) ::ExtSelectClipRgn(m_hDC, hRgn, RGN_AND);

	TFontInfo* pDefFontInfo = GlobalManager::GetTFontInfo(strFontId);
	if (pDefFontInfo == NULL) {
		pDefFontInfo = GlobalManager::GetTFontInfo(L"yahei_14");
	}
	TEXTMETRIC* pTm = &pDefFontInfo->tm;
	HFONT hOldFont = (HFONT) ::SelectObject(m_hDC, pDefFontInfo->hFont);
	::SetBkMode(m_hDC, TRANSPARENT);
	::SetTextColor(m_hDC, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));
	DWORD dwBkColor = GlobalManager::GetDefaultSelectedBkColor();
	::SetBkColor(m_hDC, RGB(GetBValue(dwBkColor), GetGValue(dwBkColor), GetRValue(dwBkColor)));

	// If the drawstyle include a alignment, we'll need to first determine the text-size so
	// we can draw it at the correct position...
	if (((uStyle & DT_CENTER) != 0 || (uStyle & DT_RIGHT) != 0 || (uStyle & DT_VCENTER) != 0 || (uStyle & DT_BOTTOM) != 0) && (uStyle & DT_CALCRECT) == 0) {
		UiRect rcText = { 0, 0, 9999, 100 };
	
		DrawHtmlText(rcText, strText, dwTextColor, strFontId, uStyle | DT_CALCRECT, prcLinks);
		if ((uStyle & DT_SINGLELINE) != 0){
			if ((uStyle & DT_CENTER) != 0) {
				rc.left = rc.left + ((rc.right - rc.left) / 2) - ((rcText.right - rcText.left) / 2);
				rc.right = rc.left + (rcText.right - rcText.left);
			}
			if ((uStyle & DT_RIGHT) != 0) {
				rc.left = rc.right - (rcText.right - rcText.left);
			}
		}
		if ((uStyle & DT_VCENTER) != 0) {
			rc.top = rc.top + ((rc.bottom - rc.top) / 2) - ((rcText.bottom - rcText.top) / 2);
			rc.bottom = rc.top + (rcText.bottom - rcText.top);
		}
		if ((uStyle & DT_BOTTOM) != 0) {
			rc.top = rc.bottom - (rcText.bottom - rcText.top);
		}
	}



	POINT pt = { rc.left, rc.top };
	int iLinkIndex = 0;
	int cyLine = pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
	int cyMinHeight = 0;
	int cxMaxWidth = 0;
	POINT ptLinkStart = { 0 };
	bool bLineEnd = false;
	bool bInRaw = false;
	bool bInSelected = false;
	int iLineLinkIndex = 0;

	// 排版习惯是图文底部对齐，所以每行绘制都要分两步，先计算高度，再绘制
	ui::CStdPtrArray aLineFontArray;
	ui::CStdPtrArray aLineColorArray;
	ui::CStdPtrArray aLinePIndentArray;
	LPCTSTR pstrLineBegin = strText.c_str();

	LPCTSTR pstrText = strText.c_str();
	bool bLineInRaw = false;

	bool bLineInSelected = false;
	int cyLineHeight = 0;
	bool bLineDraw = false; // 行的第二阶段：绘制
	while (*pstrText != _T('\0'))
	{
		if (pt.x >= rc.right || *pstrText == _T('\n') || bLineEnd)
		{
			if (*pstrText == _T('\n')) pstrText++;
			if (bLineEnd) bLineEnd = false;
			if (!bLineDraw)
			{
				for (int i = iLineLinkIndex; i < iLinkIndex; i++) {
					prcLinks[i].bottom = pt.y + cyLine;
				}
				if (bDraw) {

					iLinkIndex = iLineLinkIndex;
				}
			}
			else
			{
				iLineLinkIndex = iLinkIndex;
			}
			if ((uStyle & DT_SINGLELINE) != 0 && (!bDraw || bLineDraw)) break;
			if (bDraw) bLineDraw = !bLineDraw; // !
			pt.x = rc.left;
			if (!bLineDraw) pt.y += cyLine;
			if (pt.y > rc.bottom && bDraw) break;
			ptLinkStart = pt;
			cyLine = pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
			if (pt.x >= rc.right) break;
		}
		else if (!bInRaw && (*pstrText == _T('<') || *pstrText == _T('{'))
			&& (pstrText[1] >= _T('a') && pstrText[1] <= _T('z'))
			&& (pstrText[2] == _T(' ') || pstrText[2] == _T('>') || pstrText[2] == _T('}'))) {
			pstrText++;
			LPCTSTR pstrNextStart = NULL;
			switch (*pstrText)
			{
			case _T('b'):  // Bold
			{
				pstrText++;
				TFontInfo* pFontInfo = pDefFontInfo;
				if (aFontArray.GetSize() > 0) pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
				if (pFontInfo->bBold == false) {
					HFONT hFont = GlobalManager::GetFont(pFontInfo->sFontName, pFontInfo->iSize, true, pFontInfo->bUnderline, pFontInfo->bItalic);
					if (hFont == NULL)
					{
						std::wstring font = L"font_" + std::to_wstring(g_iFontID++);
						hFont = GlobalManager::AddFont(font, pFontInfo->sFontName, pFontInfo->iSize, true, pFontInfo->bUnderline, pFontInfo->bItalic, false,0,0);
					}
					pFontInfo = GlobalManager::GetFontInfo(hFont, m_hDC);
					aFontArray.Add(pFontInfo);
					pTm = &pFontInfo->tm;
					::SelectObject(m_hDC, pFontInfo->hFont);
					cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
				}
			}
			break;
			case _T('c'):  // Color
			{
				pstrText++;
				while (*pstrText > _T('\0') && *pstrText <= _T(' ')) pstrText = ::CharNext(pstrText);
				if (*pstrText == _T('#')) pstrText++;
				DWORD clrColor = _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 16);
				aColorArray.Add((LPVOID)clrColor);
				::SetTextColor(m_hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
			}
			break;
			case _T('f'):  // Font
			{
				pstrText++;
				while (*pstrText > _T('\0') && *pstrText <= _T(' ')) pstrText = ::CharNext(pstrText);
				LPCTSTR pstrTemp = pstrText;
				int iFont = (int)_tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
				if (pstrTemp != pstrText)
				{
					std::wstring font = L"yahei_" + std::to_wstring(iFont);
					TFontInfo* pFontInfo = GlobalManager::GetFontInfo(font, m_hDC);
					aFontArray.Add(pFontInfo);
					pTm = &pFontInfo->tm;
					::SelectObject(m_hDC, pFontInfo->hFont);
				}
				else {
					std::wstring sFontName;
					int iFontSize = 10;
					std::wstring sFontAttr;
					bool bBold = false;
					bool bUnderline = false;
					bool bItalic = false;
					bool bStrikeout = false;

					while (*pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') && *pstrText != _T(' ')) {
						pstrTemp = ::CharNext(pstrText);
						while (pstrText < pstrTemp) {
							sFontName += *pstrText++;
						}
					}
					while (*pstrText > _T('\0') && *pstrText <= _T(' ')) pstrText = ::CharNext(pstrText);
					if (isdigit(*pstrText)) {
						iFontSize = (int)_tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
					}
					while (*pstrText > _T('\0') && *pstrText <= _T(' ')) pstrText = ::CharNext(pstrText);
					while (*pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}')) {
						pstrTemp = ::CharNext(pstrText);
						while (pstrText < pstrTemp) {
							sFontAttr += *pstrText++;
						}
					}
					std::transform(sFontAttr.begin(), sFontAttr.end(), sFontAttr.begin(), towlower);
					if (sFontAttr.find(_T("bold")) >= 0) bBold = true;
					if (sFontAttr.find(_T("underline")) >= 0) bUnderline = true;
					if (sFontAttr.find(_T("italic")) >= 0) bItalic = true;
					if (sFontAttr.find(_T("strikeout")) >= 0) bStrikeout = true;
					HFONT hFont = GlobalManager::GetFont(sFontName, iFontSize, bBold, bUnderline, bItalic);
					if (hFont == NULL)
					{
						std::wstring font = L"font_" + std::to_wstring(g_iFontID++);
						hFont = GlobalManager::AddFont(font, sFontName, iFontSize, bBold, bUnderline, bItalic, false, 0, 0);
					}
					TFontInfo* pFontInfo = GlobalManager::GetFontInfo(hFont, m_hDC);
					aFontArray.Add(pFontInfo);
					pTm = &pFontInfo->tm;
					::SelectObject(m_hDC, pFontInfo->hFont);
				}
				cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
			}
			break;
			case _T('i'):  // Italic or Image
			{
				pstrNextStart = pstrText - 1;
				pstrText++;

				while (*pstrText > _T('\0') && *pstrText <= _T(' ')) pstrText = ::CharNext(pstrText);

				std::wstring sName;
				while (*pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') && *pstrText != _T(' ')) {
					LPCTSTR pstrTemp = ::CharNext(pstrText);
					while (pstrText < pstrTemp) {
						sName += *pstrText++;
					}
				}
				if (sName.empty())
				{ // Italic
					pstrNextStart = NULL;
					TFontInfo* pFontInfo = pDefFontInfo;
					if (aFontArray.GetSize() > 0) pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
					if (pFontInfo->bItalic == false) {
						HFONT hFont = GlobalManager::GetFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, pFontInfo->bUnderline, true);
						if (hFont == NULL)
						{
							std::wstring font = L"font_" + std::to_wstring(g_iFontID++);
							hFont = GlobalManager::AddFont(font, pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, pFontInfo->bUnderline, true, false, 0, 0);
						}
						pFontInfo = GlobalManager::GetFontInfo(hFont, m_hDC);
						aFontArray.Add(pFontInfo);
						pTm = &pFontInfo->tm;
						::SelectObject(m_hDC, pFontInfo->hFont);
						cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
					}
				}
				pstrNextStart = NULL;
			}
			break;
			case _T('n'):  // Newline
			{
				pstrText++;
				if ((uStyle & DT_SINGLELINE) != 0) break;
				bLineEnd = true;
			}
			break;
			case _T('p'):  // Paragraph
			{
				pstrText++;
				if (pt.x > rc.left) bLineEnd = true;
				while (*pstrText > _T('\0') && *pstrText <= _T(' ')) pstrText = ::CharNext(pstrText);
				int cyLineExtra = (int)_tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
				aPIndentArray.Add((LPVOID)cyLineExtra);
				cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + cyLineExtra);
			}
			break;
			case _T('r'):  // Raw Text
			{
				pstrText++;
				bInRaw = true;
			}
			break;
			case _T('s'):  // Selected text background color
			{
				pstrText++;
				bInSelected = !bInSelected;
				if (bDraw && bLineDraw) {
					if (bInSelected) ::SetBkMode(m_hDC, OPAQUE);
					else ::SetBkMode(m_hDC, TRANSPARENT);
				}
			}
			break;
			case _T('u'):  // Underline text
			{
				pstrText++;
				TFontInfo* pFontInfo = pDefFontInfo;
				if (aFontArray.GetSize() > 0) pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
				if (pFontInfo->bUnderline == false) {
					HFONT hFont = GlobalManager::GetFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
					if (hFont == NULL)
					{
						std::wstring font = L"font_" + std::to_wstring(g_iFontID++);
						hFont = GlobalManager::AddFont(font, pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic, false, 0, 0);
					}
					pFontInfo = GlobalManager::GetFontInfo(hFont, m_hDC);
					aFontArray.Add(pFontInfo);
					pTm = &pFontInfo->tm;
					::SelectObject(m_hDC, pFontInfo->hFont);
					cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
				}
			}
			break;
			case _T('x'):  // X Indent
			{
				pstrText++;
				while (*pstrText > _T('\0') && *pstrText <= _T(' ')) pstrText = ::CharNext(pstrText);
				int iWidth = (int)_tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
				pt.x += iWidth;
				cxMaxWidth = MAX(cxMaxWidth, pt.x);
			}
			break;
			case _T('y'):  // Y Indent
			{
				pstrText++;
				while (*pstrText > _T('\0') && *pstrText <= _T(' ')) pstrText = ::CharNext(pstrText);
				cyLine = (int)_tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
			}
			break;
			}
			if (pstrNextStart != NULL) pstrText = pstrNextStart;
			else {
				while (*pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}')) pstrText = ::CharNext(pstrText);
				pstrText = ::CharNext(pstrText);
			}
		}
		else if (!bInRaw && (*pstrText == _T('<') || *pstrText == _T('{')) && pstrText[1] == _T('/'))
		{
			pstrText++;
			pstrText++;
			switch (*pstrText)
			{
			case _T('c'):
			{
				pstrText++;
				aColorArray.Remove(aColorArray.GetSize() - 1);
				DWORD clrColor = dwTextColor;
				if (aColorArray.GetSize() > 0) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
				::SetTextColor(m_hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
			}
			break;
			case _T('p'):
				pstrText++;
				if (pt.x > rc.left) bLineEnd = true;
				aPIndentArray.Remove(aPIndentArray.GetSize() - 1);
				cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
				break;
			case _T('s'):
			{
				pstrText++;
				bInSelected = !bInSelected;
				if (bDraw && bLineDraw) {
					if (bInSelected) ::SetBkMode(m_hDC, OPAQUE);
					else ::SetBkMode(m_hDC, TRANSPARENT);
				}
			}
			break;
			case _T('b'):
			case _T('f'):
			case _T('i'):
			case _T('u'):
			{
				pstrText++;
				aFontArray.Remove(aFontArray.GetSize() - 1);
				TFontInfo* pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
				if (pFontInfo == NULL) pFontInfo = pDefFontInfo;
				if (pTm->tmItalic && pFontInfo->bItalic == false) {
					ABC abc;
					::GetCharABCWidths(m_hDC, _T(' '), _T(' '), &abc);
					pt.x += abc.abcC / 2; // 简单修正一下斜体混排的问题, 正确做法应该是http://support.microsoft.com/kb/244798/en-us
				}
				pTm = &pFontInfo->tm;
				::SelectObject(m_hDC, pFontInfo->hFont);
				cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
			}
			break;
			}
			while (*pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}')) pstrText = ::CharNext(pstrText);
			pstrText = ::CharNext(pstrText);
		}
		else if (!bInRaw &&  *pstrText == _T('<') && pstrText[2] == _T('>') && (pstrText[1] == _T('{') || pstrText[1] == _T('}')))
		{
			SIZE szSpace = { 0 };
			::GetTextExtentPoint32(m_hDC, &pstrText[1], 1, &szSpace);
			if (bDraw && bLineDraw) ::TextOut(m_hDC, pt.x, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, &pstrText[1], 1);
			pt.x += szSpace.cx;
			cxMaxWidth = MAX(cxMaxWidth, pt.x);
			pstrText++; pstrText++; pstrText++;
		}
		else if (!bInRaw &&  *pstrText == _T('{') && pstrText[2] == _T('}') && (pstrText[1] == _T('<') || pstrText[1] == _T('>')))
		{
			SIZE szSpace = { 0 };
			::GetTextExtentPoint32(m_hDC, &pstrText[1], 1, &szSpace);
			if (bDraw && bLineDraw) ::TextOut(m_hDC, pt.x, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, &pstrText[1], 1);
			pt.x += szSpace.cx;
			cxMaxWidth = MAX(cxMaxWidth, pt.x);
			pstrText++; pstrText++; pstrText++;
		}
		else if (!bInRaw &&  *pstrText == _T(' '))
		{
			SIZE szSpace = { 0 };
			::GetTextExtentPoint32(m_hDC, _T(" "), 1, &szSpace);
			// Still need to paint the space because the font might have
			// underline formatting.
			if (bDraw && bLineDraw) ::TextOut(m_hDC, pt.x, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, _T(" "), 1);
			pt.x += szSpace.cx;
			cxMaxWidth = MAX(cxMaxWidth, pt.x);
			pstrText++;
		}
		else
		{
			POINT ptPos = pt;
			int cchChars = 0;
			int cchSize = 0;
			int cchLastGoodWord = 0;
			int cchLastGoodSize = 0;
			LPCTSTR p = pstrText;
			LPCTSTR pstrNext;
			SIZE szText = { 0 };
			if (!bInRaw && *p == _T('<') || *p == _T('{')) p++, cchChars++, cchSize++;
			while (*p != _T('\0') && *p != _T('\n')) {
				// This part makes sure that we're word-wrapping if needed or providing support
				// for DT_END_ELLIPSIS. Unfortunately the GetTextExtentPoint32() call is pretty
				// slow when repeated so often.
				// TODO: Rewrite and use GetTextExtentExPoint() instead!
				if (bInRaw) {
					if ((*p == _T('<') || *p == _T('{')) && p[1] == _T('/')
						&& p[2] == _T('r') && (p[3] == _T('>') || p[3] == _T('}'))) {
						p += 4;
						bInRaw = false;
						break;
					}
				}
				else {
					if (*p == _T('<') || *p == _T('{')) break;
				}
				pstrNext = ::CharNext(p);
				cchChars++;
				cchSize += (int)(pstrNext - p);
				//szText.cx = cchChars * pTm->tmMaxCharWidth;
				//if (pt.x + szText.cx >= rc.right) {
					::GetTextExtentPoint32(m_hDC, pstrText, cchSize, &szText);
				//}

				szText.cx = ui::DpiManager::GetInstance()->ScaleIntEx(szText.cx);
				if (pt.x + szText.cx  > rc.right) {
					if (pt.x + szText.cx > rc.right && pt.x != rc.left) {
						cchChars--;
						cchSize -= (int)(pstrNext - p);
					}
					if ((uStyle & DT_WORDBREAK) != 0 && cchLastGoodWord > 0) {
						cchChars = cchLastGoodWord;
						cchSize = cchLastGoodSize;
					}

					if ((uStyle & DT_END_ELLIPSIS) != 0 && cchChars > 0) {

						::GetTextExtentPoint32(m_hDC, pstrText, cchSize, &szText);

						SIZE szDot = { 0 };
						::GetTextExtentPoint32(m_hDC, L"...", 3, &szDot);
						if (pt.x + szText.cx + szDot.cx > rc.right
							&& pt.y + szText.cy >= rc.bottom) {
							LPCTSTR pstrPrev = ::CharPrev(pstrText, p);
			
							cchChars--;
							cchSize -= (int)(p - pstrPrev);	
						}
						pt.x = rc.right;
					}
					bLineEnd = true;
					cxMaxWidth = MAX(cxMaxWidth, pt.x);
					break;
				}
				if (!((p[0] >= _T('a') && p[0] <= _T('z')) || (p[0] >= _T('A') && p[0] <= _T('Z')))) {
					cchLastGoodWord = cchChars;
					cchLastGoodSize = cchSize;
				}
				if (*p == _T(' ')) {
					cchLastGoodWord = cchChars;
					cchLastGoodSize = cchSize;
				}
				p = ::CharNext(p);
			}

			::GetTextExtentPoint32(m_hDC, pstrText, cchSize, &szText);
			if (bDraw && bLineDraw) {
				if ((uStyle & DT_SINGLELINE) == 0 && (uStyle & DT_CENTER) != 0) {
					ptPos.x += (rc.right - rc.left - szText.cx) / 2;
				}
				else if ((uStyle & DT_SINGLELINE) == 0 && (uStyle & DT_RIGHT) != 0) {
					ptPos.x += (rc.right - rc.left - szText.cx);
				}
				::TextOut(m_hDC, ptPos.x, ptPos.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, pstrText, cchSize);
				
				if (pt.x >= rc.right && (uStyle & DT_SINGLELINE) != 0 && (uStyle & DT_END_ELLIPSIS) != 0)
					::TextOut(m_hDC, ptPos.x + szText.cx, ptPos.y, _T("..."), 3);
				else if (pt.x >= rc.right && (pt.y + szText.cy >= rc.bottom) && (uStyle & DT_END_ELLIPSIS) != 0)
					::TextOut(m_hDC, ptPos.x + szText.cx, ptPos.y, _T("..."), 3);
			}
			pt.x += szText.cx;
			cxMaxWidth = MAX(cxMaxWidth, pt.x);
			pstrText += cchSize;
		}

		if (pt.x >= rc.right || *pstrText == _T('\n') || *pstrText == _T('\0')) bLineEnd = true;
		if (bDraw && bLineEnd) {
			if (!bLineDraw) {
				aFontArray.Resize(aLineFontArray.GetSize());
				::CopyMemory(aFontArray.GetData(), aLineFontArray.GetData(), aLineFontArray.GetSize() * sizeof(LPVOID));
				aColorArray.Resize(aLineColorArray.GetSize());
				::CopyMemory(aColorArray.GetData(), aLineColorArray.GetData(), aLineColorArray.GetSize() * sizeof(LPVOID));
				aPIndentArray.Resize(aLinePIndentArray.GetSize());
				::CopyMemory(aPIndentArray.GetData(), aLinePIndentArray.GetData(), aLinePIndentArray.GetSize() * sizeof(LPVOID));

				cyLineHeight = cyLine;
				pstrText = pstrLineBegin;
				bInRaw = bLineInRaw;
				bInSelected = bLineInSelected;

				DWORD clrColor = dwTextColor;
				if (aColorArray.GetSize() > 0) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
				::SetTextColor(m_hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
				TFontInfo* pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
				if (pFontInfo == NULL) pFontInfo = pDefFontInfo;
				pTm = &pFontInfo->tm;
				::SelectObject(m_hDC, pFontInfo->hFont);
				if (bInSelected) ::SetBkMode(m_hDC, OPAQUE);
			}
			else {
				aLineFontArray.Resize(aFontArray.GetSize());
				::CopyMemory(aLineFontArray.GetData(), aFontArray.GetData(), aFontArray.GetSize() * sizeof(LPVOID));
				aLineColorArray.Resize(aColorArray.GetSize());
				::CopyMemory(aLineColorArray.GetData(), aColorArray.GetData(), aColorArray.GetSize() * sizeof(LPVOID));
				aLinePIndentArray.Resize(aPIndentArray.GetSize());
				::CopyMemory(aLinePIndentArray.GetData(), aPIndentArray.GetData(), aPIndentArray.GetSize() * sizeof(LPVOID));
				pstrLineBegin = pstrText;
				bLineInSelected = bInSelected;
				bLineInRaw = bInRaw;
			}
		}

	}

	// Return size of text when requested
	if ((uStyle & DT_CALCRECT) != 0) {
		rc.bottom = MAX(cyMinHeight, pt.y + cyLine);
		rc.right = MIN(rc.right, cxMaxWidth);
	}

// 	if (bDraw) ::SelectClipRgn(m_hDC, hOldRgn);
// 	::DeleteObject(hOldRgn);
// 	::DeleteObject(hRgn);

	::SelectObject(m_hDC, hOldFont);
}


void RenderContext_GdiPlus::DrawHighlightText(const UiRect& rc, const std::wstring& strText, DWORD dwTextColor, 
	const std::wstring& strHighlightText, DWORD dwHighlightColor,
	const std::wstring& strFontId, UINT uStyle, BYTE uFade /*= 255*/, bool bLineLimit /*= false*/)
{
	assert(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);
	if (strText.empty()) return;

	ui::UiRect rcStart = rc;
	UiRect rcText = MeasureText(strText, strFontId, DT_LEFT);
	if ((uStyle & DT_SINGLELINE) != 0)
	{
		if ((uStyle & DT_CENTER) != 0) {
			rcStart.left += (rc.GetWidth() - rcText.GetWidth()) / 2;
		}
		else if ((uStyle & DT_RIGHT) != 0) {
			rcStart.left += (rc.GetWidth() - rcText.GetWidth());
		}
	}

	if ((uStyle & DT_VCENTER) != 0) {
		rcStart.top = rc.top + ((rc.GetHeight() - rcText.GetHeight()) / 2);
	}
	if ((uStyle & DT_BOTTOM) != 0) {
		rcStart.top = rc.bottom - rcText.GetHeight();
	}

	POINT ptDraw = { rcStart.left, rcStart.top };
	UiRect rcDot = MeasureText(L"...", strFontId, uStyle);

	auto fn_cb = [this, rcStart, &ptDraw, rcDot, dwTextColor](const std::wstring& strText, DWORD dwColor, const std::wstring& strFontId, UINT uStyle){
		UiRect rcText = MeasureText(strText, strFontId, uStyle);
		UiRect rcDraw = rcStart;
		rcDraw.left = ptDraw.x;
		rcDraw.right = rcDraw.left + rcText.GetWidth();
		if ((uStyle & DT_END_ELLIPSIS) && rcDraw.right > rcStart.right)
		{
			if (dwTextColor == dwColor)
			{
				rcDraw.right = rcStart.right;
				DrawText(rcDraw, strText, dwColor, strFontId, uStyle);
				return false;
			}
			rcDraw.right = rcStart.right - rcDot.GetWidth();
			if (rcDraw.right > rcDraw.left)
			{
				DrawText(rcDraw, strText, dwColor, strFontId, uStyle);
				rcDraw.left = rcDraw.right;
			}
			rcDraw.right = rcStart.right;
			DrawText(rcDraw, L"...", dwTextColor, strFontId, DT_LEFT);
			return false;
		}
		DrawText(rcDraw, strText, dwColor, strFontId, uStyle);
		ptDraw.x += rcText.GetWidth();
		return true;
	};

	std::wstring strTemp = strText;
	size_t nPos = 0;
	while ((nPos = strTemp.find(strHighlightText)) != std::wstring::npos)
	{
		std::wstring strPre = strTemp.substr(0, nPos);

		if (strPre.size() > 0)
		{
			if (!fn_cb(strPre, dwTextColor, strFontId, uStyle))
			{
				return;
			}
		}
		
		if (!fn_cb(strHighlightText, dwHighlightColor, strFontId, uStyle))
		{
			return;
		}

		size_t nNextPos = nPos + strHighlightText.size();
		if (nNextPos <= strTemp.size())
		{
			strTemp = strTemp.substr(nNextPos);
		}
		else
		{
			break;
		}
	}

	if (strTemp.size() > 0)
	{
		UiRect rcText = MeasureText(strTemp, strFontId, uStyle);
		UiRect rcDraw;
		rcDraw.top = rcStart.top;
		rcDraw.bottom = rcDraw.top + rcText.GetHeight();
		rcDraw.left = ptDraw.x;
		rcDraw.right = rcDraw.left + rcText.GetWidth();
		if (rcDraw.right > rcStart.right)
		{
			rcDraw.right = rcStart.right;
		}
		DrawText(rcDraw, strTemp, dwTextColor, strFontId, uStyle);
	}
}

void RenderContext_GdiPlus::DrawEllipse(const UiRect& rc, int nSize, DWORD dwColor)
{
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	Gdiplus::Pen pen(dwColor, nSize);
	graphics.DrawEllipse(&pen, rc.left, rc.top, rc.GetWidth(), rc.GetHeight());
}

void RenderContext_GdiPlus::FillEllipse(const UiRect& rc, DWORD dwColor)
{
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	Gdiplus::SolidBrush brush(dwColor);
	graphics.FillEllipse(&brush, rc.left, rc.top, rc.GetWidth(), rc.GetHeight());
}

void RenderContext_GdiPlus::DrawPath(const IPath* path, const IPen* pen)
{
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	graphics.DrawPath(((Pen_GdiPlus*)pen)->GetPen(), ((Path_Gdiplus*)path)->GetPath());
}

void RenderContext_GdiPlus::FillPath(const IPath* path, const IBrush* brush)
{
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	graphics.FillPath(((Brush_Gdiplus*)brush)->GetBrush(), ((Path_Gdiplus*)path)->GetPath());
}

ui::UiRect RenderContext_GdiPlus::MeasureText(const std::wstring& strText, const std::wstring& strFontId, UINT uStyle, int width /*= DUI_NOSET_VALUE*/)
{
	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Font font(m_hDC, GlobalManager::GetFont(strFontId));
	Gdiplus::RectF bounds;

	Gdiplus::StringFormat stringFormat = Gdiplus::StringFormat::GenericTypographic();
	int formatFlags = 0;
	if ((uStyle & DT_SINGLELINE) != 0) {
		formatFlags |= Gdiplus::StringFormatFlagsNoWrap;
	}
	stringFormat.SetFormatFlags(formatFlags);

	if (width == DUI_NOSET_VALUE) {
		graphics.MeasureString(strText.c_str(), (int)strText.length(), &font, Gdiplus::PointF(), &stringFormat, &bounds);
	}
	else {
		Gdiplus::REAL height = 0;
		if ((uStyle & DT_SINGLELINE) != 0) {
			Gdiplus::RectF rcEmpty((Gdiplus::REAL)0, (Gdiplus::REAL)0, (Gdiplus::REAL)0, (Gdiplus::REAL)0);
			graphics.MeasureString(L"测试", 2, &font, rcEmpty, &stringFormat, &bounds);
			height = bounds.Height;
		}
		Gdiplus::RectF rcText((Gdiplus::REAL)0, (Gdiplus::REAL)0, (Gdiplus::REAL)width, height);
		graphics.MeasureString(strText.c_str(), (int)strText.length(), &font, rcText, &stringFormat, &bounds);
	}

	UiRect rc(int(bounds.GetLeft()), int(bounds.GetTop()), int(bounds.GetRight() + 1), int(bounds.GetBottom() + 1));
	return rc;
}


//////////////////////////////////////////////////////////////////////////

RenderContext_D3D::RenderContext_D3D(HWND wnd) : 
	m_wnd(wnd)
, m_bTransparent(false)
, m_isCopy(false)
, m_dpiTransform(D2D1::Matrix3x2F::Identity())
, m_renderTransform(D2D1::Matrix3x2F::Identity())
, m_worldToPixel(D2D1::Matrix3x2F::Identity())
, m_pixelToWorld(D2D1::Matrix3x2F::Identity())
, m_beginDraw(false)
, m_hOldGdiBitmap(NULL)
, m_repaintRC(0,0,0,0)
{
	assert(m_wnd != NULL);
	HDC hDC = ::GetDC(m_wnd);
	m_compatGdiDC = ::CreateCompatibleDC(hDC);

#ifdef D2D_USE_MEMDC
	m_hDC = ::CreateCompatibleDC(hDC);
	::ReleaseDC(NULL, hDC);
#else
	m_hDC = hDC;
#endif

	RECT rc;
	GetClientRect(m_wnd, &rc);

	DpiManager::GetInstance()->ScaleRect(rc);

	D2D1_SIZE_U size = D2D1::SizeU(
		rc.right - rc.left,
		rc.bottom - rc.top);

	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> ctx = CreateD2DContext();
	if (ctx == nullptr)
	{
		throw std::exception("D2D create device context failure");
	}

	ID2D1Factory *pFactoryFromCtx = nullptr;
	ctx->GetFactory(&pFactoryFromCtx);

	m_d2dTar = CreateDCRender(pFactoryFromCtx);
	if (m_d2dTar == nullptr)
	{
		throw std::exception("D2D create render failure");
	}

	hr = m_d2dTar->BindDC(m_hDC, &rc);
	if (FAILED(hr))
	{
		throw std::exception("D2D binding render failure");
	}

	ctx->GetDpi(&m_dpi.width, &m_dpi.height);
	m_dpiTransform = D2D1::Matrix3x2F::Scale(m_dpi.width / 96.0, m_dpi.height / 96.0);

	if(m_d2dWriteFactory == nullptr)
		CreateDWriteTextResource(&m_d2dWriteFactory);

	SafeRelease(pFactoryFromCtx);

	m_characterFormatter = new CharacterFormatter();

	if (m_widImagingFactory == nullptr)
	{
		IWICImagingFactory *pWicFactory = nullptr;
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&pWicFactory)
		);
		if (SUCCEEDED(hr))
		{
			m_widImagingFactory = pWicFactory;
			SafeRelease(pWicFactory);
		}
		else {
			throw std::exception("D2D create bitmap failure");
		}
	}
	
}

RenderContext_D3D::RenderContext_D3D(RenderContext_D3D *pSourceRender) : m_wnd(pSourceRender->m_wnd)
, m_bTransparent(pSourceRender->m_bTransparent)
, m_isCopy(true)
, m_dpiTransform(pSourceRender->m_dpiTransform)
, m_renderTransform(pSourceRender->m_renderTransform)
, m_worldToPixel(pSourceRender->m_worldToPixel)
, m_pixelToWorld(pSourceRender->m_pixelToWorld)
, m_hOldGdiBitmap(NULL)
, m_repaintRC(pSourceRender->m_repaintRC)
{
	if (m_wnd == NULL)
		throw std::invalid_argument("hwnd can not be none");

	// shared device resource
	m_wndGdiTar = pSourceRender->m_wndGdiTar;

	m_d2dTar = pSourceRender->m_d2dTar;
	m_gpu = pSourceRender->m_gpu;
	m_resourceRoot = pSourceRender->m_resourceRoot;
	m_characterFormatter = pSourceRender->m_characterFormatter;
	m_d2dTar->GetDpi(&m_dpi.width, &m_dpi.height);
	//构建独立bitmap 优化掉子控件的MEM DC()
	//HDC hDC = ::GetDC(m_wnd);
	//m_hDC = ::CreateCompatibleDC(hDC);
	//::ReleaseDC(NULL, hDC);

	m_hDC = NULL;
	m_saveDC = NULL;

	m_compatGdiDC = NULL;
}

RenderContext_D3D::~RenderContext_D3D()
{
	if (!m_isCopy)
	{
		if (NULL != m_hDC)
		{
#ifdef D2D_USE_MEMDC
			DeleteDC(m_hDC);
#else
			::ReleaseDC(m_wnd, m_hDC);
#endif
			m_hDC = NULL;
		}
		if (NULL != m_compatGdiDC)
		{
			DeleteDC(m_compatGdiDC);
			m_compatGdiDC = NULL;
		}
	}
}

Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> RenderContext_D3D::CreateDCRender(ID2D1Factory *pD2DFactory)
{
	/*
	建议使用 DXGI_FORMAT_B8G8R8A8_UNORM 作为像素格式，以提高性能。 这对软件呈现目标特别有用。 
	BGRA 格式目标的性能优于 RGBA 格式。
	若要强制呈现目标使用硬件呈现，请使用 D2D1_RENDER_TARGET_TYPE_HARDWARE 设置
	*/
	ID2D1Factory1 *pCtxFactory1 = nullptr;
	if (SUCCEEDED(pD2DFactory->QueryInterface(&pCtxFactory1)))
	{
		HRESULT hr = ClearAlphaEffect::Register(pCtxFactory1, GlobalManager::GetResourceAssetsPath() + L"res");
		if (FAILED(hr))
		{
			throw std::exception("ClearAlphaEffect::Register");
		}
		hr = RichTextFixAlphaEffect::Register(pCtxFactory1, GlobalManager::GetResourceAssetsPath() + L"res");
		if (FAILED(hr))
		{
			throw std::exception("RichTextFixAlphaEffect::Register");
		}
	}
	ID2D1DCRenderTarget *pDCRender = nullptr;
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_HARDWARE,
		D2D1::PixelFormat(
			DXGI_FORMAT_B8G8R8A8_UNORM,
			D2D1_ALPHA_MODE_PREMULTIPLIED), //直接让渲染DC支持透明度
		0,
		0,
		D2D1_RENDER_TARGET_USAGE_NONE,//D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE
		D2D1_FEATURE_LEVEL_DEFAULT
	);
	
	HRESULT hr = pD2DFactory->CreateDCRenderTarget(&props, &pDCRender);
	assert(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		throw std::bad_alloc();
	}

	if(!pDCRender->IsSupported(props)){
		throw std::bad_alloc();
	}

	return pDCRender;
}

Microsoft::WRL::ComPtr<ID2D1DeviceContext> RenderContext_D3D::CreateD2DContext()
{
	ID2D1Factory* pD2DFactory = NULL;
#if defined(DEBUG) || defined(_DEBUG)
	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&pD2DFactory
	);
	if (!SUCCEEDED(hr))
	{
		return nullptr;
	}
#else
	D2D1_FACTORY_OPTIONS options;
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

	HRESULT hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		options,
		&pD2DFactory
	);
#endif

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D_FEATURE_LEVEL feature ;

	// Create the DX11 API device object, and get a corresponding context.
	ID3D11Device* pD3DDevice = nullptr;
	ID3D11DeviceContext* context = nullptr;
	//ID2D1DeviceContext
	hr = D3D11CreateDevice(
		nullptr,                    // specify null to use the default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		creationFlags,              // optionally set debug and Direct2D compatibility flags
		featureLevels,              // list of feature levels this app can support
		ARRAYSIZE(featureLevels),   // number of possible feature levels
		D3D11_SDK_VERSION,
		&pD3DDevice,                    // returns the Direct3D device created
		&feature,            // returns feature level of device created
		&context                    // returns the device immediate context
	);

	if (!SUCCEEDED(hr))
	{
		return nullptr;
	}

	IDXGIDevice* pDXGIDevice = nullptr;
	hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDXGIDevice));
	if (!SUCCEEDED(hr))
	{
		pD3DDevice->Release();
		return nullptr;
	}
	

	D2D1_CREATION_PROPERTIES d2d_prop;
	d2d_prop.threadingMode = D2D1_THREADING_MODE_SINGLE_THREADED;
	d2d_prop.options = D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS;
	ID2D1Device *pD2DDevice = nullptr;
	hr = D2D1CreateDevice(pDXGIDevice, d2d_prop, &pD2DDevice);
	if (!SUCCEEDED(hr))
	{
		pDXGIDevice->Release();
		pD3DDevice->Release();
		return nullptr;
	}

	switch (pD3DDevice->GetFeatureLevel())
	{
	case D3D_FEATURE_LEVEL_11_0:
	case D3D_FEATURE_LEVEL_10_1:
	case D3D_FEATURE_LEVEL_10_0:
	case D3D_FEATURE_LEVEL_9_3:
	case D3D_FEATURE_LEVEL_9_2:
	case D3D_FEATURE_LEVEL_9_1:
		m_gpu = true;
		break;
	default:
		m_gpu = false;
		break;
	}

	ID2D1DeviceContext *pD2DCtx = nullptr;
	hr = pD2DDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
		&pD2DCtx
	);
	
	//D2D1_PRIMITIVE_BLEND blendInfo = pD2DCtx->GetPrimitiveBlend();
	SafeRelease(pDXGIDevice);
	SafeRelease(pD2DDevice);
	SafeRelease(pD3DDevice);
	SafeRelease(pD2DFactory);
	return pD2DCtx;
}

void RenderContext_D3D::CreateDWriteTextResource(IDWriteFactory1 **ppWriteFactory)
{
	HRESULT hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory1),
		reinterpret_cast<IUnknown**>(ppWriteFactory)
	);
	if (SUCCEEDED(hr))
	{
		;
	}
}

HDC RenderContext_D3D::GetDC()
{
	return m_hDC;
}

bool RenderContext_D3D::Resize(int width, int height, bool flipBItmap)
{
	if (m_d2dBitmapTar)
	{
		D2D1_SIZE_U size = m_d2dBitmapTar->GetPixelSize();
		if (size.width == width && size.height == height)
		{
			return false;
		}
	}
	m_bFlipDraw = flipBItmap;	//后面绘图时用settransform
	return true;
}

void RenderContext_D3D::Clear()
{
	m_d2dBitmapTar->Clear();
	m_gdiBitmap.Clear();
}

std::unique_ptr<IRenderContext> RenderContext_D3D::Clone()
{
	std::unique_ptr<ui::IRenderContext> pClone = std::make_unique<ui::RenderContext_D3D>(m_wnd);
	pClone->Resize(GetWidth(), GetHeight());
	pClone->BitBlt(0, 0, GetWidth(), GetHeight(), this);
	return pClone;
}

HBITMAP RenderContext_D3D::DetachBitmap()
{
#ifdef D2D_USE_MEMDC
	assert(m_hDC && m_hOldGdiBitmap);
	assert(m_gdiBitmap.GetHeight() != 0 && m_gdiBitmap.GetWidth() != 0);
	if (m_hOldGdiBitmap == NULL)
		return NULL;

	::SelectObject(m_hDC, m_hOldGdiBitmap);
	return m_gdiBitmap.DetachBitmap();
#else
	return nullptr;
#endif
}

BYTE* RenderContext_D3D::GetBits()
{
	return nullptr;
}

int	RenderContext_D3D::GetWidth()
{
	return m_d2dBitmapTar->GetPixelSize().width;
}

int RenderContext_D3D::GetHeight()
{
	return m_d2dBitmapTar->GetPixelSize().height;
}

void RenderContext_D3D::ClearAlpha(const UiRect& rcDirty, int alpha)
{
	auto lt = m_dpiTransform.TransformPoint(D2D1::Point2F(rcDirty.left, rcDirty.top));
	auto rb = m_dpiTransform.TransformPoint(D2D1::Point2F(rcDirty.right, rcDirty.bottom));
	auto destRC = D2D1::RectF(lt.x, lt.y, rb.x, rb.y);

	m_d2dBitmapTar->PushAxisAlignedClip(
		destRC,
		D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	m_d2dBitmapTar->Clear(D2D1::ColorF(RGB(alpha, alpha, alpha), alpha / 255.0f));
	m_d2dBitmapTar->PopAxisAlignedClip();
	/*
	ID2D1Effect *pEffect = nullptr;
	do 
	{
		HRESULT hr = m_d2dBitmapTar->CreateEffect(CLSID_D2D1ColorMatrix, &pEffect);
		if (FAILED(hr)) {
			break;
		}
		
		auto cloneBitmap = compatibleBitmap(this);
		pEffect->SetInput(0, cloneBitmap.Get());
		//bgra or rgba
		D2D1_MATRIX_5X4_F matrix = D2D1::Matrix5x4F(
			1, 0, 0, 0, 
			0, 1, 0, 0, 
			0, 0, 1, 0, 
			0, 0, 0, alpha / 255.0,
			0, 0, 0, 0);
		hr = pEffect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
		assert(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			m_d2dBitmapTar->DrawImage(
				pEffect,
				lt,
				destRC,
				D2D1_INTERPOLATION_MODE_LINEAR,
				D2D1_COMPOSITE_MODE_SOURCE_COPY
			);
		}

	} while (FALSE);

	SafeRelease(pEffect); */
}

void RenderContext_D3D::RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding, int alpha)
{
	auto lt = m_dpiTransform.TransformPoint(D2D1::Point2F(rcDirty.left, rcDirty.top));
	auto rb = m_dpiTransform.TransformPoint(D2D1::Point2F(rcDirty.right, rcDirty.bottom));
	auto destRC = D2D1::RectF(lt.x, lt.y, rb.x, rb.y);

	ID2D1Effect *pEffect = nullptr;
	do
	{
		HRESULT hr = m_d2dBitmapTar->CreateEffect(CLSID_ClearAlphaEffect, &pEffect);
		if (FAILED(hr)) {
			throw std::exception("Create CLSID_ClearAlphaEffect faiulre");
		}

		Microsoft::WRL::ComPtr<ID2D1Bitmap> pCopyBitmap = compatibleBitmap(this);
		pEffect->SetInput(0, pCopyBitmap.Get());
		auto size = pCopyBitmap->GetPixelSize();

		int nTop = MAX(rcDirty.top, 0);
		int nBottom = MIN(rcDirty.bottom, size.height);
		int nLeft = MAX(rcDirty.left, 0);
		int nRight = MIN(rcDirty.right, size.width);
		pEffect->SetValue(CLEAR_ALPHA_DIRTY_RC, D2D1::RectF(nLeft, nTop, nRight, nBottom));
		pEffect->SetValue(CLEAR_ALPHA_PROP_PADDING, D2D1::RectF(rcShadowPadding.left, rcShadowPadding.top, rcShadowPadding.right, rcShadowPadding.bottom));
		pEffect->SetValue(CLEAR_ALPHA_PROP_SIZE, D2D1::SizeF(size.width, size.height));
		double testN = std::round(alpha / 255);
		pEffect->SetValue(CLEAR_ALPHA_PROP_ALPHA, std::round(alpha / 255));
		assert(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			m_d2dBitmapTar->DrawImage(
				pEffect,
				D2D1::Point2F(),
				D2D1::RectF(0,0, size.width, size.height),
				D2D1_INTERPOLATION_MODE_LINEAR,
				D2D1_COMPOSITE_MODE_SOURCE_COPY
			);
		}
	} while (FALSE);
	SafeRelease(pEffect);
}

bool RenderContext_D3D::IsRenderTransparent()const
{
	return m_bTransparent;
}

bool RenderContext_D3D::SetRenderTransparent(bool bTransparent)
{
	bool oldValue = m_bTransparent;
	m_bTransparent = bTransparent;
	return oldValue;
}

void RenderContext_D3D::Save()
{
	m_saveDC = SaveDC(m_hDC);
}

void RenderContext_D3D::Restore()
{
	RestoreDC(m_hDC, m_saveDC);
}

CPoint RenderContext_D3D::OffsetWindowOrg(CPoint ptOffset)
{
	//test
	D2D1_POINT_2F offset = m_dpiTransform.TransformPoint(D2D1::Point2F(ptOffset.x, ptOffset.y));
	D2D1::Matrix3x2F offsetWindowOrgTransform = D2D1::Matrix3x2F::Translation(-offset.x, -offset.y);
	D2D1::Matrix3x2F oldRenderTransform;
	m_d2dBitmapTar->GetTransform(&oldRenderTransform);
	D2D1::Matrix3x2F renderTransform = offsetWindowOrgTransform * m_renderTransform;
	m_d2dBitmapTar->SetTransform(renderTransform);
	m_worldToPixel = renderTransform * m_dpiTransform;
	m_pixelToWorld = m_worldToPixel;
	m_pixelToWorld.Invert();
	D2D1_POINT_2F oldWindowOrg = (oldRenderTransform * m_dpiTransform).TransformPoint(D2D1::Point2F(0.0f, 0.0f));
	return CPoint(oldWindowOrg.x, oldWindowOrg.y);
}

CPoint RenderContext_D3D::SetWindowOrg(CPoint ptOffset)
{
	D2D1_POINT_2F offset = m_dpiTransform.TransformPoint(D2D1::Point2F(ptOffset.x, ptOffset.y));
	D2D1::Matrix3x2F offsetWindowOrgTransform = D2D1::Matrix3x2F::Translation(offset.x, offset.y);
	D2D1::Matrix3x2F oldRenderTransform;
	m_d2dBitmapTar->GetTransform(&oldRenderTransform);
	m_d2dBitmapTar->SetTransform(offsetWindowOrgTransform);
	m_worldToPixel = offsetWindowOrgTransform * m_dpiTransform;
	m_pixelToWorld = m_worldToPixel;
	m_pixelToWorld.Invert();
	D2D1_POINT_2F oldWindowOrg = (oldRenderTransform * m_dpiTransform).TransformPoint(D2D1::Point2F(0.0f, 0.0f));
	return CPoint(oldWindowOrg.x, oldWindowOrg.y);
}

CPoint RenderContext_D3D::GetWindowOrg() const
{
	D2D1::Matrix3x2F oldRenderTransform;
	m_d2dBitmapTar->GetTransform(&oldRenderTransform);
	D2D1_POINT_2F oldWindowOrg = (oldRenderTransform * m_dpiTransform).TransformPoint(D2D1::Point2F(0.0f, 0.0f));
	return CPoint(oldWindowOrg.x, oldWindowOrg.y);
}

void RenderContext_D3D::SetClip(const UiRect& rc)
{
	m_clip.CreateClip2(m_hDC, m_d2dBitmapTar.Get(), rc);
}

void RenderContext_D3D::SetRoundClip(const UiRect& rc, int width, int height)
{
	m_clip.CreateRoundClip2(m_hDC, m_d2dBitmapTar.Get(), rc, width, height);
}

void RenderContext_D3D::ClearClip()
{
	m_clip.ClearClip2(m_d2dBitmapTar.Get());
}

HRESULT RenderContext_D3D::BitBlt(int x, int y, int cx, int cy, HDC hdcSrc, int xSrc, int yScr, DWORD rop)
{
	HRESULT hr = S_OK; 
	HBITMAP hBitmap = static_cast<HBITMAP>(GetCurrentObject(hdcSrc, OBJ_BITMAP));
	assert(NULL != hBitmap);
	
	Microsoft::WRL::ComPtr<ID2D1Bitmap> bitmap = CompatibleBitmapFromHBITMAP(hdcSrc,hBitmap,m_d2dBitmapTar.Get(), D2D1_ALPHA_MODE_PREMULTIPLIED);

	D2D1_POINT_2F destXY = m_dpiTransform.TransformPoint(D2D1::Point2F(x, y));
	D2D1_POINT_2F srcXY = m_dpiTransform.TransformPoint(D2D1::Point2F(xSrc, yScr));
	D2D1_POINT_2F destSize = m_dpiTransform.TransformPoint(D2D1::Point2F(cx, cy));
	m_d2dBitmapTar->DrawBitmap(
		bitmap.Get(), 
		D2D1::RectF(destXY.x, destXY.y, destXY.x + destSize.x, destXY.y + destSize.y),
		1.0,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		D2D1::RectF(srcXY.x, srcXY.y, srcXY.x + destSize.x, srcXY.y + destSize.y));
	return hr;
}

bool RenderContext_D3D::AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, HDC hdcSrc, int xSrc, int yScr, int widthSrc, int heightSrc, BYTE uFade)
{
	HRESULT hr = S_OK;
	HBITMAP hBitmap = static_cast<HBITMAP>(GetCurrentObject(hdcSrc, OBJ_BITMAP));
	assert(NULL != hBitmap);
	
	Microsoft::WRL::ComPtr<ID2D1Bitmap> srcBitmap = CompatibleBitmapFromHBITMAP(m_hDC, hBitmap, m_d2dBitmapTar.Get(), D2D1_ALPHA_MODE_PREMULTIPLIED);
	Microsoft::WRL::ComPtr<ID2D1Bitmap> copyBitmap = compatibleBitmap(this);
	assert(copyBitmap != nullptr);

	auto srcPixelSize =  srcBitmap->GetPixelSize();
	D2D1_POINT_2F destXY = m_dpiTransform.TransformPoint(D2D1::Point2F(xDest, yDest));
	D2D1_POINT_2F srcXY = m_dpiTransform.TransformPoint(D2D1::Point2F(xSrc, yScr));
	D2D1_POINT_2F destSize = m_dpiTransform.TransformPoint(D2D1::Point2F(widthDest, heightDest));

	Microsoft::WRL::ComPtr<ID2D1Effect> blendEffect;
	
	hr = m_d2dBitmapTar->CreateEffect(CLSID_D2D1Blend, &blendEffect);
	assert(SUCCEEDED(hr));

	blendEffect->SetInput(0, copyBitmap.Get());
	blendEffect->SetInput(1, srcBitmap.Get());
	hr = blendEffect->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_SCREEN);
	assert(SUCCEEDED(hr));

	m_d2dBitmapTar->DrawImage(blendEffect.Get(),
		&destXY,
		&D2D1::RectF(srcXY.x, srcXY.y, srcXY.x+ destSize.x, srcXY.y + destSize.y),
		D2D1_INTERPOLATION_MODE_LINEAR, 
		D2D1_COMPOSITE_MODE_SOURCE_COPY);

	return hr;
}

Microsoft::WRL::ComPtr<ID2D1Bitmap> RenderContext_D3D::compatibleBitmap(RenderContext_D3D *tarRenderContext)
{
	Microsoft::WRL::ComPtr<ID2D1Bitmap> bitmap;
	assert(tarRenderContext != nullptr);

	{
		m_d2dBitmapTar->Flush();
	}

	if (this == tarRenderContext) {
		//自己复制自己的位图
		Microsoft::WRL::ComPtr<ID2D1Bitmap> backingBitmap = m_curBitmap;
		D2D1_BITMAP_PROPERTIES bitmapProperties = D2D1::BitmapProperties(backingBitmap->GetPixelFormat());
		Microsoft::WRL::ComPtr<ID2D1Bitmap> copiedBitmap;
		D2D1_SIZE_U bitmapSize = backingBitmap->GetPixelSize();
		auto targetPos = D2D1::Point2U();
		D2D1_RECT_U dataRect = D2D1::RectU(0, 0, bitmapSize.width, bitmapSize.height);
		if (SUCCEEDED(tarRenderContext->m_d2dBitmapTar->CreateBitmap(bitmapSize, bitmapProperties, &copiedBitmap))) {
			if (SUCCEEDED(copiedBitmap->CopyFromBitmap(&targetPos, backingBitmap.Get(), &dataRect)))
				return copiedBitmap;
		}
	}

	auto size = m_curBitmap->GetPixelSize();
	assert(size.height && size.width);

	Microsoft::WRL::ComPtr<ID2D1DeviceContext> sourceDeviceCtx = m_d2dBitmapTar;
	
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> sourceCPUBitmap;
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1((D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW), D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
	HRESULT hr = sourceDeviceCtx->CreateBitmap(size, nullptr, size.width * 4, bitmapProperties, &sourceCPUBitmap);
	if (!SUCCEEDED(hr))
		return bitmap;

	if (!sourceCPUBitmap)
		return bitmap;

	hr = sourceCPUBitmap->CopyFromBitmap(nullptr, m_curBitmap.Get(), nullptr);
	if (!SUCCEEDED(hr))
		return bitmap;

	D2D1_MAPPED_RECT mappedSourceData;
	hr = sourceCPUBitmap->Map(D2D1_MAP_OPTIONS_READ, &mappedSourceData);
	if (!SUCCEEDED(hr))
		return bitmap;

	Microsoft::WRL::ComPtr<ID2D1Bitmap> compatibleBitmap;
	D2D1_BITMAP_PROPERTIES tarBitmapProperties = D2D1::BitmapProperties(m_curBitmap->GetPixelFormat());
	hr = tarRenderContext->m_d2dBitmapTar->CreateBitmap(size, mappedSourceData.bits, mappedSourceData.pitch, tarBitmapProperties, &compatibleBitmap);
	if (!SUCCEEDED(hr))
		return bitmap;

	hr = sourceCPUBitmap->Unmap();
	assert(SUCCEEDED(hr));

	return compatibleBitmap;
}

void RenderContext_D3D::saveRepaint(const UiRect& destRt) {
	D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(destRt.left, destRt.top));
	D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(destRt.right, destRt.bottom));
	
	D2D1_RECT_F destRc = D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y);
	HRESULT hr = S_OK;
	m_d2dBitmapTar->SetTarget(m_paintBitmap.Get());
	m_d2dBitmapTar->BeginDraw();
	m_d2dBitmapTar->Clear();
	/*
	m_d2dBitmapTar->DrawBitmap(
		m_curBitmap.Get(),
		destRc,
		1.0f,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		destRc);*/
	hr = m_d2dBitmapTar->EndDraw();
	if (FAILED(hr)) {
		throw std::exception("saveRepaint failure");
	}
	m_d2dBitmapTar->SetTarget(m_curBitmap.Get());
}

HRESULT RenderContext_D3D::BitBlt(int x, int y, int cx, int cy, IRenderContext *srcCtx, int xSrc, int yScr, DWORD rop)
{
	HRESULT hr = S_OK;
	RenderContext_D3D *d3dCtx = dynamic_cast<RenderContext_D3D*>(srcCtx);
	if (nullptr != d3dCtx) {
		Microsoft::WRL::ComPtr<ID2D1Bitmap> pCloneBitmap = compatibleBitmap(d3dCtx);
		if (pCloneBitmap != nullptr) {
			D2D1_POINT_2F destXY = m_dpiTransform.TransformPoint(D2D1::Point2F(x, y));
			D2D1_POINT_2F srcXY = m_dpiTransform.TransformPoint(D2D1::Point2F(xSrc, yScr));
			D2D1_POINT_2F destSize = m_dpiTransform.TransformPoint(D2D1::Point2F(cx, cy));
			m_d2dBitmapTar->DrawBitmap(
				pCloneBitmap.Get(),
				D2D1::RectF(destXY.x, destXY.y, destXY.x + destSize.x, destXY.y + destSize.y),
				1.0,
				D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
				D2D1::RectF(srcXY.x, srcXY.y, srcXY.x + destSize.x, srcXY.y + destSize.y));
		}
	}
	else {
		return BitBlt(x, y, cx, cy, srcCtx->GetDC(), xSrc, yScr, rop);
	}
	return hr;
}

bool RenderContext_D3D::AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, IRenderContext *srcCtx, int xSrc, int yScr, int widthSrc, int heightSrc, BYTE uFade)
{
	HRESULT hr = S_OK;
	//do nothing
	RenderContext_D3D *d3dCtx = dynamic_cast<RenderContext_D3D*>(srcCtx);
	if (nullptr != d3dCtx)
	{
		Microsoft::WRL::ComPtr<ID2D1Bitmap> pCopyBitmap = compatibleBitmap(this);
		assert(pCopyBitmap != nullptr);
		Microsoft::WRL::ComPtr<ID2D1Bitmap> pTarCloneBitmap = compatibleBitmap(d3dCtx);
		assert(pTarCloneBitmap != nullptr);

		if (pTarCloneBitmap != nullptr && pCopyBitmap != nullptr) {
			D2D1_POINT_2F destXY = m_dpiTransform.TransformPoint(D2D1::Point2F(xDest, yDest));
			D2D1_POINT_2F srcXY = m_dpiTransform.TransformPoint(D2D1::Point2F(xSrc, yScr));
			D2D1_POINT_2F destSize = m_dpiTransform.TransformPoint(D2D1::Point2F(widthDest, heightDest));
			D2D1_SIZE_F srcSize = pTarCloneBitmap->GetSize();

			Microsoft::WRL::ComPtr<ID2D1Effect> blendEffect;

			hr = m_d2dBitmapTar->CreateEffect(CLSID_D2D1Blend, &blendEffect);
			assert(SUCCEEDED(hr));

			blendEffect->SetInput(0, pCopyBitmap.Get());
			blendEffect->SetInput(1, pTarCloneBitmap.Get());
			hr = blendEffect->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_SCREEN);
			assert(SUCCEEDED(hr));

			m_d2dBitmapTar->DrawImage(blendEffect.Get(),
				D2D1_INTERPOLATION_MODE_LINEAR,
				D2D1_COMPOSITE_MODE_SOURCE_COPY);
		}
	}
	else {
		return AlphaBlend(xDest, yDest, widthDest, heightDest, srcCtx->GetDC(), xSrc, yScr, widthSrc, heightSrc, uFade);
	}
	return true;
}

void RenderContext_D3D::DrawImageFunction(ID2D1Bitmap *pImage, const UiRect& rcDest, const UiRect& rcSource, bool bTransparent, bool bAlphaChannel, FLOAT opacity)
{
	//see https://blog.csdn.net/lvdepeng123/article/details/79322823
	//see https://learn.microsoft.com/en-us/windows/win32/api/d2d1_1/ne-d2d1_1-d2d1_composite_mode
	//see https://learn.microsoft.com/zh-cn/windows/win32/direct2d/opacity-masks-overview
	/*支持两张位图alpha混合 263537890*/

	D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(rcDest.left, rcDest.top));
	D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(rcDest.right, rcDest.bottom));
	D2D1_POINT_2F srcLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(rcSource.left, rcSource.top));
	D2D1_POINT_2F srcRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(rcSource.right, rcSource.bottom));

	bool isSrcDstFit = (rcSource.GetWidth() == rcDest.GetWidth() && rcSource.GetHeight() == rcDest.GetHeight());
	if ((bTransparent || bAlphaChannel || opacity < 0.99f) || isSrcDstFit) {
		D2D1_PRIMITIVE_BLEND oldBlend = m_d2dBitmapTar->GetPrimitiveBlend();
		m_d2dBitmapTar->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
		m_d2dBitmapTar->DrawBitmap(
			pImage,
			D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y),
			opacity,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
			D2D1::RectF(srcLT.x, srcLT.y, srcRB.x, srcRB.y));
		m_d2dBitmapTar->SetPrimitiveBlend(oldBlend);
	}
	else {
		m_d2dBitmapTar->DrawBitmap(
			pImage,
			D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y), 
			opacity,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, 
			D2D1::RectF(srcLT.x, srcLT.y, srcRB.x, srcRB.y));
	}
}

Microsoft::WRL::ComPtr<ID2D1Bitmap> RenderContext_D3D::CompatibleBitmapFromHBITMAP(HDC hDC, HBITMAP hBitmap, ID2D1RenderTarget *pRenderTar, D2D1_ALPHA_MODE alphaMode, bool flip/*=true*/, bool needDrawBk /*= false*/)
{
	/*
	For uncompressed RGB bitmaps,
	if biHeight is positive, the bitmap is a bottom-up DIB with the origin at the lower left corner.
	If biHeight is negative, the bitmap is a top-down DIB with the origin at the upper left corner.
	*/

	BITMAP info;
	if (0 == ::GetObject(hBitmap, sizeof(info), &info))
	{
		return nullptr;
	}
	if (info.bmBits == nullptr) {
		return nullptr;
	}
	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	::GetDIBits(hDC, hBitmap, 0, 0, NULL, &bmi, DIB_PAL_COLORS);

	if (!(bmi.bmiHeader.biCompression == BI_RGB || bmi.bmiHeader.biCompression == BI_BITFIELDS))
	{
		return nullptr;
	}

	LPBYTE pPixel = LPBYTE(info.bmBits);
	WORD bytesPerPixel = info.bmBitsPixel / 8;

	D2D1_BITMAP_PROPERTIES bitmapProperties = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, alphaMode));
	Microsoft::WRL::ComPtr<ID2D1Bitmap> pImageBitmap;
	HRESULT hr = pRenderTar->CreateBitmap(D2D1::SizeU(info.bmWidth, info.bmHeight), bitmapProperties, &pImageBitmap);
	assert(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		return nullptr;
	}

	auto imageRt = D2D1::RectU(0, 0, info.bmWidth, info.bmHeight);
	int size = info.bmWidth * info.bmHeight * sizeof(DWORD);
	if (flip && bmi.bmiHeader.biHeight > 0) {
		// 位图默认是倒序，需延X轴反转图片，后期用GPU换算
		int lines = size / info.bmWidthBytes;
		LPBYTE pixelBuf = (LPBYTE)malloc(size);
		for (int line = lines - 1; line >= 0; line--)
		{
			for (int n = 0; n < info.bmWidthBytes; ++n) {
				pixelBuf[(lines - 1 - line)*info.bmWidthBytes + n] = pPixel[(line*info.bmWidthBytes) + n];
			}
		}
		hr = pImageBitmap->CopyFromMemory(&imageRt, pixelBuf, info.bmWidthBytes);
		assert(SUCCEEDED(hr));
		free(pixelBuf);
	}
	else {
		hr = pImageBitmap->CopyFromMemory(&imageRt, pPixel, info.bmWidthBytes);
		assert(SUCCEEDED(hr));
	}

	if (needDrawBk)
	{
		HRESULT hr = pRenderTar->CreateBitmap(D2D1::SizeU(info.bmWidth, info.bmHeight), bitmapProperties, &pImageBitmap);
		if (FAILED(hr))
		{
			throw std::exception("");
		}
	}

	return pImageBitmap;
}

void RenderContext_D3D::DrawImage(const UiRect& rcPaint, HBITMAP hBitmap, bool bAlphaChannel,
	const UiRect& rcImageDest, const UiRect& rcImageSource, const UiRect& rcCorners, BYTE uFade, 
	bool xtiled, bool ytiled)
{
	UiRect rcTestTemp;
	if (!::IntersectRect(&rcTestTemp, &rcImageDest, &rcPaint)) return;

	assert(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);

	assert(hBitmap != NULL);
	if (hBitmap == NULL) return;

	D2D1_ALPHA_MODE alphaMode = D2D1_ALPHA_MODE_IGNORE;
	FLOAT opacity = uFade / 255;
	if ((m_bTransparent || bAlphaChannel || opacity < 0.99f)) {
		alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	}

	auto pImageBitmap = CompatibleBitmapFromHBITMAP(m_hDC, hBitmap, m_d2dBitmapTar.Get(), alphaMode);

	if (pImageBitmap == nullptr)
	{
		return;
	}

	UiRect rcTemp;
	UiRect rcSource;
	UiRect rcDest;
	UiRect rcDpiCorner = rcCorners;
	//DpiManager::GetInstance()->ScaleRect(rcDpiCorner);

	// middle
	rcDest.left = rcImageDest.left + rcDpiCorner.left;
	rcDest.top = rcImageDest.top + rcDpiCorner.top;
	rcDest.right = rcImageDest.right - rcDpiCorner.right;
	rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;
	rcSource.left = rcImageSource.left + rcDpiCorner.left;
	rcSource.top = rcImageSource.top + rcDpiCorner.top;
	rcSource.right = rcImageSource.right - rcDpiCorner.right;
	rcSource.bottom = rcImageSource.bottom - rcDpiCorner.bottom;
	if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
		if (!xtiled && !ytiled) {
			DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
		}
		else if (xtiled && ytiled) {
			LONG lWidth = rcImageSource.right - rcImageSource.left - rcDpiCorner.left - rcDpiCorner.right;
			LONG lHeight = rcImageSource.bottom - rcImageSource.top - rcDpiCorner.top - rcDpiCorner.bottom;
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
					rcDest.left = rcDest.left + lWidth * i;
					rcDest.top = rcDest.top + lHeight * j;
					rcDest.right = rcDest.left + lDestRight - lDestLeft;
					rcDest.bottom = rcDest.top + lDestBottom - lDestTop;
					rcSource.left = rcImageSource.left + rcDpiCorner.left;
					rcSource.top = rcImageSource.top + rcDpiCorner.top;
					rcSource.right = rcSource.left + lDrawWidth;
					rcSource.bottom = rcSource.top + lDrawHeight;
					DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
				}
			}
		}
		else if (xtiled) {
			LONG lWidth = rcImageSource.right - rcImageSource.left - rcDpiCorner.left - rcDpiCorner.right;
			int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
			for (int i = 0; i < iTimes; ++i) {
				LONG lDestLeft = rcDest.left + lWidth * i;
				LONG lDestRight = rcDest.left + lWidth * (i + 1);
				LONG lDrawWidth = lWidth;
				if (lDestRight > rcDest.right) {
					lDrawWidth -= lDestRight - rcDest.right;
					lDestRight = rcDest.right;
				}
				rcDest.left = lDestLeft;
				rcDest.top = rcDest.top;
				rcDest.right = lDestRight;
				rcDest.bottom = rcDest.top + rcDest.bottom;
				rcSource.left = rcImageSource.left + rcDpiCorner.left;
				rcSource.top = rcImageSource.top + rcDpiCorner.top;
				rcSource.right = rcSource.left + lDrawWidth;
				rcSource.bottom = rcImageSource.bottom - rcDpiCorner.bottom;
				D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(rcDest.left, rcDest.top));
				D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(rcDest.right, rcDest.bottom));
				D2D1_POINT_2F srcLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(rcSource.left, rcSource.top));
				D2D1_POINT_2F srcRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(rcSource.right, rcSource.bottom));
				
				DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
			}
		}
		else { // ytiled
			LONG lHeight = rcImageSource.bottom - rcImageSource.top - rcDpiCorner.top - rcDpiCorner.bottom;
			int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
			for (int i = 0; i < iTimes; ++i) {
				LONG lDestTop = rcDest.top + lHeight * i;
				LONG lDestBottom = rcDest.top + lHeight * (i + 1);
				LONG lDrawHeight = lHeight;
				if (lDestBottom > rcDest.bottom) {
					lDrawHeight -= lDestBottom - rcDest.bottom;
					lDestBottom = rcDest.bottom;
				}
				rcDest.left = rcDest.left;
				rcDest.top = rcDest.top + lHeight * i;
				rcDest.right = rcDest.left + rcDest.right;
				rcDest.bottom = rcDest.top + lDestBottom - lDestTop;
				rcSource.left = rcImageSource.left + rcDpiCorner.left;
				rcSource.top = rcImageSource.top + rcDpiCorner.top;
				rcSource.right = rcImageSource.right - rcDpiCorner.right;
				rcSource.bottom = rcSource.top + lDrawHeight;
				DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
			}
		}
	}

	// left-top
	if (rcDpiCorner.left > 0 && rcDpiCorner.top > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.left + rcDpiCorner.left;
		rcSource.bottom = rcImageSource.top + rcDpiCorner.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
		}
	}
	// top
	if (rcDpiCorner.top > 0) {
		rcDest.left = rcImageDest.left + rcDpiCorner.left;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.right - rcDpiCorner.right;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.left + rcDpiCorner.left;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.right - rcDpiCorner.right;
		rcSource.bottom = rcImageSource.top + rcDpiCorner.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
		}
	}
	// right-top
	if (rcDpiCorner.right > 0 && rcDpiCorner.top > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.right - rcDpiCorner.right;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.top + rcDpiCorner.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
		}
	}
	// left
	if (rcDpiCorner.left > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.top + rcDpiCorner.top;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.top + rcDpiCorner.top;
		rcSource.right = rcImageSource.left + rcDpiCorner.left;
		rcSource.bottom = rcImageSource.bottom - rcDpiCorner.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
		}
	}
	// right
	if (rcDpiCorner.right > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.top + rcDpiCorner.top;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;
		rcSource.left = rcImageSource.right - rcDpiCorner.right;
		rcSource.top = rcImageSource.top + rcDpiCorner.top;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.bottom - rcDpiCorner.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
		}
	}
	// left-bottom
	if (rcDpiCorner.left > 0 && rcDpiCorner.bottom > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.bottom - rcDpiCorner.bottom;
		rcSource.right = rcImageSource.left + rcDpiCorner.left;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
		}
	}
	// bottom
	if (rcDpiCorner.bottom > 0) {
		rcDest.left = rcImageDest.left + rcDpiCorner.left;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.right - rcDpiCorner.right;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.left + rcDpiCorner.left;
		rcSource.top = rcImageSource.bottom - rcDpiCorner.bottom;
		rcSource.right = rcImageSource.right - rcDpiCorner.right;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
		}
	}
	// right-bottom
	if (rcDpiCorner.right > 0 && rcDpiCorner.bottom > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.right - rcDpiCorner.right;
		rcSource.top = rcImageSource.bottom - rcDpiCorner.bottom;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawImageFunction(pImageBitmap.Get(), rcDest, rcSource, m_bTransparent, bAlphaChannel, opacity);
		}
	}
}

void RenderContext_D3D::DrawColor(const UiRect& rc, DWORD dwColor, BYTE uFade)
{
	DWORD dwNewColor = dwColor;
	int alpha = dwColor >> 24;
	if (alpha == 0)
		alpha = alpha + 1;
	dwNewColor = dwColor & 0x00ffffff;
	if (uFade < 255) {
		alpha *= double(uFade) / 255;
	}

	ID2D1SolidColorBrush *pColorBrush = nullptr;
	HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwNewColor, alpha / 255.0f), &pColorBrush);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.left, rc.top));
		D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.right, rc.bottom));
		D2D1_RECT_F rect = D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y);
		m_d2dBitmapTar->FillRectangle(rect, pColorBrush);
	}
	SafeRelease(pColorBrush);
}

void RenderContext_D3D::DrawColor(const UiRect& rc, const std::wstring& colorStr, BYTE uFade)
{
	if (colorStr.empty()) {
		return;
	}

	DWORD dwColor = GlobalManager::GetTextColor(colorStr);
	DrawColor(rc, dwColor, uFade);
}

void RenderContext_D3D::DrawGradientColor(const UiRect& rc, const std::wstring& colorStr, const std::wstring& colorStr1, const int mode)
{
	// see https://learn.microsoft.com/en-us/windows/win32/direct2d/direct2d-brushes-overview
	DWORD dwColor = GlobalManager::GetTextColor(colorStr);
	DWORD dwColor1 = GlobalManager::GetTextColor(colorStr1);

	ID2D1GradientStopCollection *pGradientStops = NULL;

	D2D1_GRADIENT_STOP gradientStops[2];
	gradientStops[0].color = D2D1::ColorF(dwColor & 0x00ffffff, dwColor >> 24);
	gradientStops[0].position = 0.0f;
	gradientStops[1].color = D2D1::ColorF(dwColor1 & 0x00ffffff, dwColor1 >> 24);
	gradientStops[1].position = 1.0f;

	HRESULT hr = m_d2dBitmapTar->CreateGradientStopCollection(
		gradientStops,
		ARRAYSIZE(gradientStops),
		D2D1_GAMMA_2_2,
		D2D1_EXTEND_MODE_CLAMP,
		&pGradientStops
	);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		ID2D1LinearGradientBrush *m_pLinearGradientBrush = NULL;
		D2D1_POINT_2F ps;
		D2D1_POINT_2F pe;
		switch ((Gdiplus::LinearGradientMode)mode)
		{
		case Gdiplus::LinearGradientMode::LinearGradientModeHorizontal:
			ps = m_pixelToWorld.TransformPoint(D2D1::Point2F(0, rc.GetHeight() >> 1));
			pe = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.right, rc.GetHeight() >> 1));
			break;
		case Gdiplus::LinearGradientMode::LinearGradientModeVertical:
			ps = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.GetWidth() >> 1, 0));
			pe = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.GetWidth() >> 1, rc.bottom));
			break;
		case Gdiplus::LinearGradientMode::LinearGradientModeForwardDiagonal:
			ps = m_pixelToWorld.TransformPoint(D2D1::Point2F(0, 0));
			pe = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.right, rc.bottom));
			break;
		case Gdiplus::LinearGradientMode::LinearGradientModeBackwardDiagonal:
			ps = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.right, 0));
			pe = m_pixelToWorld.TransformPoint(D2D1::Point2F(0, rc.bottom));
			break;
		}
		hr = m_d2dBitmapTar->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(
				ps,
				pe),
			pGradientStops,
			&m_pLinearGradientBrush
		);
		assert(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.left, rc.top));
			D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.right, rc.bottom));
			D2D1_RECT_F rect = D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y);
			m_d2dBitmapTar->FillRectangle(rect, m_pLinearGradientBrush);
			SafeRelease(m_pLinearGradientBrush);
		}
		SafeRelease(pGradientStops);
	}
}

void RenderContext_D3D::DrawLine(const UiRect& rc, int nSize, DWORD dwPenColor)
{
	ID2D1Factory *pD2DFactory = nullptr;
	m_d2dBitmapTar->GetFactory(&pD2DFactory);
	ID2D1SolidColorBrush *pColorBrush = nullptr;
	HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwPenColor, 1.0f), &pColorBrush);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		m_d2dBitmapTar->DrawLine(m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.left, rc.top)), m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.right, rc.bottom)), pColorBrush, DX::ConvertPixelsToDips(nSize,m_dpi.width));
		SafeRelease(pColorBrush);
	}
	SafeRelease(pD2DFactory);
}

D2D1_CAP_STYLE RenderContext_D3D::Translate2D2DPenCapStyle(int style)
{
	D2D1_CAP_STYLE capStyle;
	switch (style)
	{
	case IPen::LineCap::LineCapFlat:
		capStyle = D2D1_CAP_STYLE_FLAT;
		break;
	case IPen::LineCap::LineCapRound:
		capStyle = D2D1_CAP_STYLE_ROUND;
		break;
	case IPen::LineCap::LineCapSquare:
		capStyle = D2D1_CAP_STYLE_SQUARE;
		break;
	case IPen::LineCap::LineCapTriangle:
		capStyle = D2D1_CAP_STYLE_TRIANGLE;
		break;
	default:
		capStyle = D2D1_CAP_STYLE_ROUND;
		break;
	}
	return capStyle;
}

D2D1_LINE_JOIN RenderContext_D3D::Translate2D2DPenLineJoin(int style)
{
	D2D1_LINE_JOIN lineJoin;
	switch (style)
	{
	case IPen::LineJoin::LineJoinBevel:
		lineJoin = D2D1_LINE_JOIN_BEVEL;
		break;
	case IPen::LineJoin::LineJoinMiter:
		lineJoin = D2D1_LINE_JOIN_MITER;
		break;
	case IPen::LineJoin::LineJoinRound:
		lineJoin = D2D1_LINE_JOIN_ROUND;
		break;
	case IPen::LineJoin::LineJoinMiterClipped:
		lineJoin = D2D1_LINE_JOIN_MITER_OR_BEVEL;
		break;
	default:
		lineJoin = D2D1_LINE_JOIN_MITER;
		break;
	}
	return lineJoin;
}

D2D1_DASH_STYLE RenderContext_D3D::Translate2D2DPenDashStyle(int style)
{
	D2D1_DASH_STYLE dashStyle;
	switch (style)
	{
	case IPen::DashStyle::DashStyleSolid:
		dashStyle = D2D1_DASH_STYLE_SOLID;
		break;
	case IPen::DashStyle::DashStyleDash:
		dashStyle = D2D1_DASH_STYLE_DASH;
		break;
	case IPen::DashStyle::DashStyleDot:
		dashStyle = D2D1_DASH_STYLE_DOT;
		break;
	case IPen::DashStyle::DashStyleDashDot:
		dashStyle = D2D1_DASH_STYLE_DASH_DOT;
		break;
	case IPen::DashStyle::DashStyleDashDotDot:
		dashStyle = D2D1_DASH_STYLE_DASH_DOT_DOT;
		break;
	case IPen::DashStyle::DashStyleCustom:
		dashStyle = D2D1_DASH_STYLE_CUSTOM;
		break;
	default:
		dashStyle = D2D1_DASH_STYLE_CUSTOM;
		break;
	}
	return dashStyle;
}

ID2D1StrokeStyle* RenderContext_D3D::CreateStrokeFromPen(const ui::IPen* pen)
{
	Microsoft::WRL::ComPtr<ID2D1Factory> pD2DFactory = nullptr;
	DX::GetD2D1Factory(&pD2DFactory);
	ID2D1StrokeStyle *pStrokeStyle = nullptr;
	D2D1_CAP_STYLE startCap = Translate2D2DPenCapStyle(pen->GetStartCap());
	D2D1_CAP_STYLE endCap = Translate2D2DPenCapStyle(pen->GetEndCap());
	D2D1_CAP_STYLE dashCap = Translate2D2DPenCapStyle(pen->GetDashCap());
	D2D1_LINE_JOIN joinStyle = Translate2D2DPenLineJoin(pen->GetLineJoin());
	D2D1_DASH_STYLE dashStyle = Translate2D2DPenDashStyle(pen->GetDashStyle());
	HRESULT hr = pD2DFactory->CreateStrokeStyle(
		D2D1::StrokeStyleProperties(
			startCap,
			endCap,
			dashCap,
			joinStyle,
			10.0f,
			dashStyle,
			0.0f),
		nullptr,
		0,
		&pStrokeStyle
	);
	assert(SUCCEEDED(hr));
	return pStrokeStyle;
}

void RenderContext_D3D::DrawLine(const IPen* pen, int x1, int y1, int x2, int y2)
{
	ID2D1StrokeStyle *pStrokeStyle = CreateStrokeFromPen(pen);
	ID2D1SolidColorBrush *pColorBrush = nullptr;
	HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(pen->GetColor(), 1.0f), &pColorBrush);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		m_d2dBitmapTar->DrawLine(
			m_pixelToWorld.TransformPoint(D2D1::Point2F(x1, y1)), 
			m_pixelToWorld.TransformPoint(D2D1::Point2F(x2, y2)), 
			pColorBrush,
			DX::ConvertPixelsToDips(pen->GetWidth(),m_dpi.width), 
			pStrokeStyle);
		SafeRelease(pColorBrush);
	}
	SafeRelease(pStrokeStyle);
}


void RenderContext_D3D::DrawBezier(const IPen* pen, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	ID2D1Factory *pFactory = NULL;
	m_d2dBitmapTar->GetFactory(&pFactory);
	ID2D1PathGeometry *pPathGeometry = NULL;
	HRESULT hr = pFactory->CreatePathGeometry(&pPathGeometry);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		ID2D1GeometrySink *pSink = NULL;
		hr = pPathGeometry->Open(&pSink);

		if (SUCCEEDED(hr))
		{
			pSink->BeginFigure(
				m_pixelToWorld.TransformPoint(D2D1::Point2F(x1, y1)),
				D2D1_FIGURE_BEGIN_FILLED
			);
			pSink->AddBezier(D2D1::BezierSegment(
				m_pixelToWorld.TransformPoint(D2D1::Point2F(x2, y2)),
				m_pixelToWorld.TransformPoint(D2D1::Point2F(x3, y3)),
				m_pixelToWorld.TransformPoint(D2D1::Point2F(x4, y4))));

			pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

			hr = pSink->Close();
			assert(SUCCEEDED(hr));
			SafeRelease(pSink);

			ID2D1SolidColorBrush *pColorBrush = nullptr;
			HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(pen->GetColor(), 1.0f), &pColorBrush);
			assert(SUCCEEDED(hr));
			if (SUCCEEDED(hr))
			{
				ID2D1StrokeStyle *pStrokeStyle = CreateStrokeFromPen(pen);
				m_d2dBitmapTar->DrawGeometry(pPathGeometry,pColorBrush, DX::ConvertPixelsToDips(pen->GetWidth(), m_dpi.width), pStrokeStyle);
				SafeRelease(pColorBrush);
				SafeRelease(pStrokeStyle);
			}
		}
		SafeRelease(pPathGeometry);
	}
	SafeRelease(pFactory);
}

void RenderContext_D3D::DrawRect(const UiRect& rc, int nSize, DWORD dwPenColor)
{
	DWORD dwNewColor = dwPenColor;
	int alpha = dwPenColor >> 24;

	ID2D1SolidColorBrush *pColorBrush = nullptr;
	HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwNewColor, alpha / 255.0f), &pColorBrush);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.left, rc.top));
		D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.right, rc.bottom));
		D2D1_RECT_F rect = D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y);
		m_d2dBitmapTar->DrawRectangle(rect, pColorBrush, DX::ConvertPixelsToDips(nSize,m_dpi.width));
		SafeRelease(pColorBrush);
	}
}

void RenderContext_D3D::DrawRoundRect(const UiRect& rc, const CSize& roundSize, int nSize, DWORD dwPenColor)
{
	DWORD dwNewColor = dwPenColor;
	int alpha = dwPenColor >> 24;

	ID2D1SolidColorBrush *pColorBrush = nullptr;
	HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwNewColor, alpha / 255.0f), &pColorBrush);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.left, rc.top));
		D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.right, rc.bottom));
		D2D1_RECT_F rect = D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y);
		D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
			rect, 
			DX::ConvertPixelsToDips(roundSize.cx, m_dpi.width), 
			DX::ConvertPixelsToDips(roundSize.cy, m_dpi.height)
		);
		m_d2dBitmapTar->DrawRoundedRectangle(roundedRect, pColorBrush, DX::ConvertPixelsToDips(nSize,m_dpi.width));
		SafeRelease(pColorBrush);
	}
}

DWRITE_FONT_WEIGHT RenderContext_D3D::Translate2D2DTextWeight(tagTFontInfo* pFontInf)
{
	DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_MEDIUM;
	switch (pFontInf->tm.tmWeight)
	{
	case FW_THIN:
		fontWeight = DWRITE_FONT_WEIGHT_THIN;
		break;
	case FW_EXTRALIGHT:
		fontWeight = DWRITE_FONT_WEIGHT_EXTRA_LIGHT;
		break;
	case FW_LIGHT:
		fontWeight = DWRITE_FONT_WEIGHT_LIGHT;
		break;
	case FW_NORMAL:
		fontWeight = DWRITE_FONT_WEIGHT_REGULAR;
		break;
	case FW_MEDIUM:
		fontWeight = DWRITE_FONT_WEIGHT_MEDIUM;
		break;
	case FW_SEMIBOLD:
		fontWeight = DWRITE_FONT_WEIGHT_DEMI_BOLD;
		break;
	case FW_BOLD:
		fontWeight = DWRITE_FONT_WEIGHT_BOLD;
		break;
	case FW_EXTRABOLD:
		fontWeight = DWRITE_FONT_WEIGHT_EXTRA_BOLD;
		break;
	case FW_HEAVY:
		fontWeight = DWRITE_FONT_WEIGHT_HEAVY;
		break;
	default:
		fontWeight = DWRITE_FONT_WEIGHT_MEDIUM;
		break;
	}
	return fontWeight;
}
DWRITE_FONT_STYLE RenderContext_D3D::Translate2D2DTextStyle(tagTFontInfo* pFontInf)
{
	if (pFontInf->bItalic)
	{
		return DWRITE_FONT_STYLE_ITALIC;
	}
	else {
		return DWRITE_FONT_STYLE_NORMAL;
	}
}

FLOAT RenderContext_D3D::_GetFontHeight(tagTFontInfo* pFontInf)
{
	//获取字体高度
	DWRITE_FONT_METRICS fontMetrics = {};
	(pFontInf->dWriteFontFace)->GetMetrics(&fontMetrics);

	float fontHeight = (fontMetrics.ascent + fontMetrics.descent + fontMetrics.lineGap)
		* pFontInf->dWriteFmt->GetFontSize() / fontMetrics.designUnitsPerEm;
	return fontHeight;
}

tagTFontInfo* RenderContext_D3D::_GetAndGenD2DTextResource(const std::wstring& strFontId)
{
	ui::TFontInfo* pFontInf = GlobalManager::GetFontInfo(strFontId, m_hDC);
	assert(pFontInf != nullptr);
	Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat = pFontInf->dWriteFmt;
	if (nullptr == pTextFormat) {
		//延迟生成并保存
		HRESULT hr = m_d2dWriteFactory->CreateTextFormat(
			pFontInf->sFontName.c_str(),
			nullptr,
			Translate2D2DTextWeight(pFontInf),
			Translate2D2DTextStyle(pFontInf),
			DWRITE_FONT_STRETCH_NORMAL,
			pFontInf->iSize,
			L"zh-CN",
			&pTextFormat);
		if (SUCCEEDED(hr))
		{
			pFontInf->dWriteFmt = pTextFormat;
			hr = m_d2dWriteFactory->CreateEllipsisTrimmingSign(pTextFormat.Get(), &pFontInf->dWriteTrimObj);
			assert(SUCCEEDED(hr));
			{
				IDWriteFontCollection*  fontCollection = NULL;
				IDWriteFontFamily*      fontFamily = NULL;
				IDWriteFont*            font = NULL;

				wchar_t fontFamilyName[100];

				////////////////////
				// Map font and style to fontFace.

				if (SUCCEEDED(hr))
				{
					// Need the font collection to map from font name to actual font.
					pTextFormat->GetFontCollection(&fontCollection);
					if (fontCollection == NULL)
					{
						// No font collection was set in the format, so use the system default.
						hr = m_d2dWriteFactory->GetSystemFontCollection(&fontCollection);
					}
				}

				// Find matching family name in collection.
				if (SUCCEEDED(hr))
				{
					hr = pTextFormat->GetFontFamilyName(fontFamilyName, ARRAYSIZE(fontFamilyName));
				}

				UINT32 fontIndex = 0;
				if (SUCCEEDED(hr))
				{
					BOOL fontExists = false;
					hr = fontCollection->FindFamilyName(fontFamilyName, &fontIndex, &fontExists);
					if (!fontExists)
					{
						// If the given font does not exist, take what we can get
						// (displaying something instead nothing), choosing the foremost
						// font in the collection.
						fontIndex = 0;
					}
				}

				if (SUCCEEDED(hr))
				{
					hr = fontCollection->GetFontFamily(fontIndex, &fontFamily);
				}

				if (SUCCEEDED(hr))
				{
					hr = fontFamily->GetFirstMatchingFont(
						pTextFormat->GetFontWeight(),
						pTextFormat->GetFontStretch(),
						pTextFormat->GetFontStyle(),
						&font
					);
				}

				if (SUCCEEDED(hr))
				{
					pFontInf->dWriteFontFace = nullptr;
					hr = font->CreateFontFace(&pFontInf->dWriteFontFace);
				}

				SafeRelease(font);
				SafeRelease(fontFamily);
				SafeRelease(fontCollection);
			}
		}
	}
	return pFontInf;
}

Microsoft::WRL::ComPtr<IDWriteTextLayout> RenderContext_D3D::_BuildTextLayout(
	const std::wstring &strText, 
	const D2D1_RECT_F& rc,
	tagTFontInfo* pFontInf,
	UINT uStyle, 
	bool bLineLimit,
	D2D1_DRAW_TEXT_OPTIONS *pTextOptions)
{
	Microsoft::WRL::ComPtr<IDWriteTextLayout> pTextLayout;
	HRESULT hr = m_d2dWriteFactory->CreateTextLayout(
		strText.c_str(),
		strText.length(),
		pFontInf->dWriteFmt.Get(),
		rc.right - rc.left,
		rc.bottom - rc.top,
		&pTextLayout);
	assert(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		return pTextLayout;
	}

	DWRITE_TRIMMING trimming;
	trimming.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
	if ((uStyle & DT_END_ELLIPSIS) != 0) {
		trimming.delimiter = 0;
		trimming.delimiterCount = 0;
	}
	if ((uStyle & DT_PATH_ELLIPSIS) != 0) {
		trimming.delimiter = '\\';
		trimming.delimiterCount = 2;
	}
	hr = pTextLayout->SetTrimming(&trimming, pFontInf->dWriteTrimObj.Get());
	assert(SUCCEEDED(hr));

	D2D1_DRAW_TEXT_OPTIONS drawTextOpt;
	if ((uStyle & DT_NOCLIP) != 0) {
		drawTextOpt = D2D1_DRAW_TEXT_OPTIONS_NONE;
	}
	else {
		drawTextOpt = D2D1_DRAW_TEXT_OPTIONS_CLIP;
	}

	if (pTextOptions != nullptr)
	{
		*pTextOptions = drawTextOpt;
	}

	if ((uStyle & DT_SINGLELINE) != 0) {
		hr = pTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	}
	else if (bLineLimit) {
		hr = pTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
	}
	if (FAILED(hr))
	{
		assert(SUCCEEDED(hr));
	}

	if ((uStyle & DT_LEFT) != 0) {
		hr = pTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	}
	else if ((uStyle & DT_CENTER) != 0) {
		hr = pTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}
	else if ((uStyle & DT_RIGHT) != 0) {
		hr = pTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	}
	else {
		hr = pTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	}
	if (FAILED(hr))
	{
		assert(SUCCEEDED(hr));
	}
	if ((uStyle & DT_TOP) != 0) {
		hr = pTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	}
	else if ((uStyle & DT_VCENTER) != 0) {
		hr = pTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}
	else if ((uStyle & DT_BOTTOM) != 0) {
		hr = pTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
	}
	else {
		hr = pTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	}
	if (FAILED(hr))
	{
		assert(SUCCEEDED(hr));
	}

	return pTextLayout;
}

void RenderContext_D3D::DrawText(const UiRect& rc, const std::wstring& strText, DWORD dwTextColor, const std::wstring& strFontId, UINT uStyle, BYTE uFade, bool bLineLimit)
{
	//see https://learn.microsoft.com/en-us/windows/win32/direct2d/compatible-a8-rendertargets  
	//see https://learn.microsoft.com/en-us/windows/win32/direct2d/opacity-masks-overview
	assert(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);
	if (strText.empty()) return;

	ui::TFontInfo* pFontInf = _GetAndGenD2DTextResource(strFontId);
	Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat = pFontInf->dWriteFmt;

	//see https://learn.microsoft.com/en-us/windows/win32/directwrite/introducing-directwrite
	D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.left, rc.top));
	D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.right, rc.bottom));
	D2D1_RECT_F rcPaint = D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y);
	
	int alpha = dwTextColor >> 24;
	uFade *= double(alpha) / 255;
	if (uFade == 255) {
		uFade = 254;
	}
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pColorBrush = nullptr;
	HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwTextColor & 0x00ffffff, uFade / 255.0f), &pColorBrush);
	SUCCEEDED(hr);
	if (FAILED(hr))
	{
		return;
	}

	D2D1_DRAW_TEXT_OPTIONS drawTextOpt;
	Microsoft::WRL::ComPtr<IDWriteTextLayout> pTextLayout = _BuildTextLayout(strText, rcPaint, pFontInf, uStyle,bLineLimit, &drawTextOpt);
	assert(pTextLayout != nullptr);
	if (pTextLayout)
	{
		m_d2dBitmapTar->DrawTextLayout(destLT, pTextLayout.Get(), pColorBrush.Get(), drawTextOpt);
	}
}

void RenderContext_D3D::DrawHighlightText(const UiRect& rc, const std::wstring& strText, DWORD dwTextColor,
	const std::wstring& strHighlightText, DWORD dwHighlightColor,
	const std::wstring& strFontId, UINT uStyle, BYTE uFade, bool bLineLimit)
{
	assert(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);
	if (strText.empty()) return;

	ui::TFontInfo* pFontInf = _GetAndGenD2DTextResource(strFontId);
	Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat = pFontInf->dWriteFmt;
	D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.left, rc.top));
	D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.right, rc.bottom));
	D2D1_RECT_F rcPaint = D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y);

	D2D1_DRAW_TEXT_OPTIONS drawTextOpt;
	Microsoft::WRL::ComPtr<IDWriteTextLayout> pTextLayout = _BuildTextLayout(strText, rcPaint, pFontInf, uStyle, bLineLimit, &drawTextOpt);
	assert(pTextLayout != nullptr);
	if (pTextLayout == nullptr)
	{
		return;
	}

	int alphaNormal = dwTextColor >> 24;
	int alphaHightLight = dwHighlightColor >> 24;
	int uFadeNormal = uFade * double(alphaNormal) / 255;
	if (uFadeNormal == 255) {
		uFadeNormal = 254;
	}
	
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pColorBrush = nullptr;
	HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwTextColor & 0x00ffffff, uFadeNormal / 255.0f), &pColorBrush);
	assert(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		return;
	}

	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pHightColorBrush = nullptr;
	if (alphaNormal == alphaHightLight)
	{
		pHightColorBrush = pColorBrush;
	}
	else {
		//see https://www.charlespetzold.com/blog/2014/01/Character-Formatting-Extensions-with-DirectWrite.html
		int uFadeHightLight = uFade * double(alphaHightLight) / 255;
		if (uFadeHightLight == 255) {
			uFadeHightLight = 254;
		}
		HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwHighlightColor & 0x00ffffff, uFadeHightLight / 255.0f), &pHightColorBrush);
		assert(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			size_t nPos = 0;
			size_t nextPos = 0;
			while ((nPos = strText.find(strHighlightText, nextPos)) != std::wstring::npos)
			{
				DWRITE_TEXT_RANGE textRange{ nextPos,nPos };
				pTextLayout->SetDrawingEffect(pHightColorBrush.Get(), textRange);
				nextPos = nPos + strHighlightText.size();
			}
		}
	}

	if (pTextLayout)
	{
		m_d2dBitmapTar->DrawTextLayout(destLT, pTextLayout.Get(), pColorBrush.Get(), drawTextOpt);
	}
}



void RenderContext_D3D::_ReleaseHtmlTextEntryList(_HtmlTextEntry *pEntry)
{
	_HtmlTextEntry *next = pEntry;
	while (next) {
		if (next->childs)
		{
			_ReleaseHtmlTextEntryList(next->childs);
			next->childs = nullptr;
		}
		_HtmlTextEntry *pEntry = next;
		next = next->next;
		delete pEntry;
	}
}

size_t RenderContext_D3D::_BuildHtmlTextSpecifier(_HtmlTextEntry* root, IDWriteTextLayout *pTextLayout, ID2D1Brush *pDefautBrush)
{
	HRESULT hr = S_OK;
	size_t around = 0;
	_HtmlTextEntry* next = root;
	while (next)
	{
		if (next->childs != nullptr) {
			_BuildHtmlTextSpecifier(next->childs, pTextLayout, pDefautBrush);
		}
		if (next->len == 0)
		{
			next = next->next;
			continue;
		}
		DWRITE_TEXT_RANGE range{ next->start, next->len };
		if (strcmp(next->name, "b") == 0) { //bold
			hr = pTextLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
		}
		else if (strcmp(next->name, "c") == 0) //foreground color 
		{
			char color[16] = { 0 };
			if (memcmp(next->color, color, ARRAYSIZE(color)) == 0) { //检查是否有效
				hr = CharacterFormatSpecifier::SetForegroundBrush(pTextLayout, pDefautBrush, range);
			}
			else {
				ID2D1SolidColorBrush *pColorBrush = nullptr;
				DWORD dwTextColor = 0;
				if (next->color[0] == '#') {
					dwTextColor = std::stoul(&next->color[1]);
				}
				else {
					dwTextColor = std::stoul(next->color);
				}
				int alpha = dwTextColor >> 24;
				int uFade = double(alpha) / 255;
				hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwTextColor & 0x00ffffff, uFade / 255.0f), &pColorBrush);
				assert(SUCCEEDED(hr));
				if (SUCCEEDED(hr))
				{
					hr = CharacterFormatSpecifier::SetForegroundBrush(pTextLayout, pColorBrush, range);
					SafeRelease(pColorBrush);
				}
			}
			assert(SUCCEEDED(hr));
		}
		else if (strcmp(next->name, "f") == 0) // spec font
		{
			// 如果font_id不存在，则获取默认
			ui::TFontInfo* pFontInf = _GetAndGenD2DTextResource(std::to_wstring(next->font_id));
			hr = CharacterFormatSpecifier::SetFont(pTextLayout, pFontInf->dWriteFmt.Get(),range);
			assert(SUCCEEDED(hr));
		}
		else if (strcmp(next->name, "i") == 0) // Italic
		{
			if (next->text.empty()) {
				//image
				InlineImage *pInlineImage = new InlineImage(m_d2dBitmapTar.Get(), m_widImagingFactory.Get());
				hr = pInlineImage->SetImage(m_resourceRoot, next->path, next->img_attr.count, next->img_attr.index);
				if(SUCCEEDED(hr))
					hr = pTextLayout->SetInlineObject(pInlineImage, range);
				assert(SUCCEEDED(hr));
			}
			else {
				hr = pTextLayout->SetFontStyle(DWRITE_FONT_STYLE_ITALIC, range);
				assert(SUCCEEDED(hr));
			}
		}
		else if (strcmp(next->name, "a") == 0) // link
		{
			;
		}
		else if (strcmp(next->name, "s") == 0) // selected
		{
			ID2D1SolidColorBrush *pColorBrush = nullptr;
			DWORD dwBkColor = GlobalManager::GetDefaultSelectedBkColor();
			int alpha = dwBkColor >> 24;
			int uFade = double(alpha) / 255;
			HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwBkColor & 0x00ffffff, uFade / 255.0f), &pColorBrush);
			if (SUCCEEDED(hr))
			{
				hr = CharacterFormatSpecifier::SetBackgroundBrush(pTextLayout, BackgroundMode::LineHeight, pColorBrush, range);
				SafeRelease(pColorBrush);
			}
			assert(SUCCEEDED(hr));
		}
		else if (strcmp(next->name, "u") == 0) // underline
		{
			hr = CharacterFormatSpecifier::SetUnderline(pTextLayout, UnderlineType::Single, pDefautBrush, range);
			assert(SUCCEEDED(hr));
		}

		if (next->childs != nullptr)
		{
			_BuildHtmlTextSpecifier(next->childs, pTextLayout, pDefautBrush);
		}
		next = next->next;
	}
	return around;
}

std::wstring _GetHtmlRawTextString(TiXmlElement* root, RenderContext_D3D::_HtmlTextEntry** ppEntry)
{
	std::wstring whole;
	if (root) {
		size_t pos = 0;
		RenderContext_D3D::_HtmlTextEntry *first_entry = nullptr;
		RenderContext_D3D::_HtmlTextEntry *next_entry = nullptr;
		for (TiXmlElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement()) {
			const char* name = element->Value();
			const char* text = element->GetText();
			std::wstring rawText;
			if (text != nullptr) {
				rawText = std::move(nbase::StdStringToStdWstring(text));
			}
			RenderContext_D3D::_HtmlTextEntry *entry = new RenderContext_D3D::_HtmlTextEntry;
			if (first_entry == nullptr) {
				first_entry = entry;
				next_entry = first_entry;
			}
			else {
				next_entry->next = entry;
				next_entry = entry;
			}
			int line = 0; //换行
			int spacing = 0; //字符空格
			if (strcmp(name, "p") == 0) { //段落
				if (NULL == element->Attribute("x_ch", &spacing))
					spacing = 0;
				if (NULL == element->Attribute("y_ch", &line))
					line = 0;

				if (line == 0) {
					line++;
				}
			}
			else if (strcmp(name, "n") == 0) { //换行
				++line;
			}
			if (line > 0) {
				for (int i = 0; i < line; ++i)
					whole.push_back(L'\n');
			}
			if (spacing > 0)
			{
				for(int i = 0; i < spacing; i++)
					whole.push_back(L' ');
			}
			if (text != nullptr)
			{
				whole += rawText;
			}
			//构建entry
			{
				if (text != nullptr)
				{
					entry->text = rawText;
				}
				strcpy_s(entry->name,name);
				entry->start = pos;
				entry->len = rawText.length();
				if (strcmp(name, "c") == 0)
				{
					const char* color = element->Attribute("id");
					assert(color != nullptr);
					if (color != nullptr)
					{
						strcpy_s(entry->color, color);
					}
					else {
						memset(entry->color, '\0', ARRAYSIZE(entry->color));
					}
				}
				else if (strcmp(name, "f") == 0) {
					int font_id = -1;
					element->Attribute("id", &font_id);
					assert(font_id > 0);
					entry->font_id = font_id;
				}
				else if (strcmp(name, "i") == 0) {
					if (!entry->text.empty())
					{
						const char* src = element->Attribute("src");
						assert(src != nullptr);
						if (NULL == element->Attribute("count", &entry->img_attr.count))
							entry->img_attr.count = 1;
						if (NULL == element->Attribute("index", &entry->img_attr.index))
							entry->img_attr.index = 0;
						if (src != nullptr) {
							entry->path = std::move(nbase::StdStringToStdWstring(src));
						}
					}
				}
				else if (strcmp(name, "a") == 0) {
					const char* href = element->Attribute("href");
					assert(href != nullptr);
					if (nullptr != href) {
						entry->path = std::move(nbase::StdStringToStdWstring(href));
					}
				}
			}
			
			if (!element->NoChildren())
			{
				//html嵌套，深度优先
				std::wstring from_child = std::move(_GetHtmlRawTextString(element, &entry->childs));
				pos += from_child.length();
				//更新父标签
				entry->len = from_child.length();
				whole += from_child;
			}
		}
		if (nullptr != ppEntry)
		{
			*ppEntry = first_entry;
		}
	}
	return whole;
}

// 改用DWRITE方式，XML格式升级到标准XML
// 支持标签嵌套（如<l><b>text</b></l>），但是交叉嵌套是应该避免的（如<l><b>text</l></b>）
// The string formatter supports a kind of "mini-html" that consists of various short tags:
//
//   Bold:             <b>text</b>
//   Color:            <c id="#xxxxxx">text</c>  where x = RGB in hex
//   Font:             <f id=x>text</f>        where x = font id
//   Italic:           <i>text</i>
//   Image:            <i src="file://" count=y index=z>            where src = image path and y = image split count and z(optional) = image id
//   Link:             <a href="x">text</a>        where x(optional) = link content, normal like app:notepad or http:www.xxx.com
//   NewLine           <n/>                  
//   Paragraph:        <p x_ch=x y_ch=y>text</p>			   where x = hor extra spaceing indent in p
//   Raw Text:         <r>text</r>
//   Selected:         <s>text</s>
//   Underline:        <u>text</u>
void RenderContext_D3D::DrawHtmlText(const UiRect& rc_item, const std::wstring& strText, DWORD dwTextColor, const std::wstring& strFontId, UINT uStyle, RECT* prcLinks)
{
	UiRect rc = rc_item;
	
	assert(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);
	if (strText.empty()) return;

	if (::IsRectEmpty(&rc)) return;

	ui::TFontInfo* pFontInf = _GetAndGenD2DTextResource(strFontId);
	Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat = pFontInf->dWriteFmt;

	D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.left, rc.top));
	D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.right, rc.bottom));
	D2D1_RECT_F rcPaint = D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y);

	int alpha = dwTextColor >> 24;
	int uFade = double(alpha) / 255;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pColorBrush;
	HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwTextColor & 0x00ffffff, uFade / 255.0f), &pColorBrush);
	assert(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		return;
	}

	std::wstring realText;
	Microsoft::WRL::ComPtr<IDWriteTextLayout> pTextLayout;
	TiXmlDocument doc;
	doc.Parse(nbase::StdWStringToStdString(strText).c_str());
	if (!doc.Error())
	{
		TiXmlElement* root = doc.RootElement();
		_HtmlTextEntry *pEntry = nullptr;
		realText = std::move(_GetHtmlRawTextString(root, &pEntry));

		D2D1_DRAW_TEXT_OPTIONS drawTextOpt;
		pTextLayout = _BuildTextLayout(realText, rcPaint, pFontInf, uStyle, true, &drawTextOpt);
		assert(pTextLayout != nullptr);
		if (pTextLayout)
		{
			_BuildHtmlTextSpecifier(pEntry, pTextLayout.Get(), pColorBrush.Get());
			_ReleaseHtmlTextEntryList(pEntry);
		}
	}
	else {
		realText = std::move(nbase::StdStringToStdWstring(doc.ErrorDesc()));
		D2D1_DRAW_TEXT_OPTIONS drawTextOpt;
		pTextLayout = _BuildTextLayout(realText, rcPaint, pFontInf, uStyle, true, &drawTextOpt);
	}
	
	if (pTextLayout)
	{
		HRESULT hr = m_characterFormatter->Draw(m_d2dBitmapTar.Get(), pTextLayout.Get(), destLT, pColorBrush.Get());
		assert(SUCCEEDED(hr));
	}
}

void RenderContext_D3D::DrawEllipse(const UiRect& rc, int nSize, DWORD dwColor)
{
	DWORD dwNewColor = dwColor;
	int alpha = dwColor >> 24;
	
	ID2D1SolidColorBrush *pColorBrush = nullptr;
	HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwNewColor, alpha / 255.0f), &pColorBrush);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		D2D1_POINT_2F center = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.left + (rc.GetWidth() >> 1), rc.top + (rc.GetHeight() >> 1)));
		D2D1_POINT_2F sz = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.GetWidth() >> 1, rc.GetHeight() >> 1));
		D2D1_ELLIPSE ell = D2D1::Ellipse(center, sz.x, sz.y);
		m_d2dBitmapTar->DrawEllipse(ell, pColorBrush);
		SafeRelease(pColorBrush);
	}
}

void RenderContext_D3D::FillEllipse(const UiRect& rc, DWORD dwColor)
{
	DWORD dwNewColor = dwColor;
	int alpha = dwColor >> 24;

	ID2D1SolidColorBrush *pColorBrush = nullptr;
	HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwNewColor, alpha / 255.0f), &pColorBrush);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		D2D1_POINT_2F center = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.left + (rc.GetWidth() >> 1), rc.top + (rc.GetHeight() >> 1)));
		D2D1_POINT_2F sz = m_pixelToWorld.TransformPoint(D2D1::Point2F(rc.GetWidth() >> 1, rc.GetHeight() >> 1));
		D2D1_ELLIPSE ell = D2D1::Ellipse(center, sz.x, sz.y);
		m_d2dBitmapTar->FillEllipse(ell, pColorBrush);
		SafeRelease(pColorBrush);
	}
}

UiRect RenderContext_D3D::MeasureText(const std::wstring& strText, const std::wstring& strFontId, UINT uStyle, int width)
{
	ui::TFontInfo* pFontInf = _GetAndGenD2DTextResource(strFontId);
	Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat = pFontInf->dWriteFmt;
	Microsoft::WRL::ComPtr<IDWriteTextLayout> pTextLayout;
	D2D1_SIZE_U pixelSize = m_d2dTar->GetPixelSize();
	HRESULT hr = m_d2dWriteFactory->CreateTextLayout(
		strText.c_str(),
		strText.length(),
		pTextFormat.Get(),
		//std::numeric_limits<float>::infinity()
		DX::ConvertPixelsToDips(width == DUI_NOSET_VALUE ? pixelSize.width : width, m_dpi.width),
		DX::ConvertPixelsToDips(pixelSize.height, m_dpi.height),
		&pTextLayout);
	assert(SUCCEEDED(hr));
	UiRect rc(0, 0, 0, 0);
	if (SUCCEEDED(hr))
	{
		if ((width == 0 || width == DUI_NOSET_VALUE) || (uStyle & DT_SINGLELINE) != 0) {
			hr = pTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		}
		else {
			hr = pTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
		}
		if (FAILED(hr))
		{
			assert(SUCCEEDED(hr));
		}
		
		DWRITE_TEXT_METRICS textMetrics;
		hr = pTextLayout->GetMetrics(&textMetrics);
		assert(SUCCEEDED(hr));
		// 文本的宽度
		float textWidth = textMetrics.width;
		float textHeight = textMetrics.height;
		width = int(DX::ConvertDipsToPixels(textWidth, m_dpi.width));
		int height = int(DX::ConvertDipsToPixels(textHeight, m_dpi.height));
		rc.right = width;
		rc.bottom = height;
	}

	return rc;
}

void RenderContext_D3D::DrawPath(const IPath* path, const IPen* pen)
{
	Path_Direct2D *d2dPath = reinterpret_cast<Path_Direct2D*>(const_cast<IPath*>(path));
	assert(d2dPath != nullptr);
	ID2D1Geometry* geo = d2dPath->GetPath();
	assert(geo != nullptr);
	DWORD dwColor = pen->GetColor();
	int alpha = dwColor >> 24;

	ID2D1SolidColorBrush *pColorBrush = nullptr;
	HRESULT hr = m_d2dBitmapTar->CreateSolidColorBrush(D2D1::ColorF(dwColor & 0x00ffffff, alpha / 255.0f), &pColorBrush);
	assert(SUCCEEDED(hr));

	ID2D1StrokeStyle *pStroke = CreateStrokeFromPen(pen);
	assert(pStroke != nullptr);
	ID2D1Factory *pD2d1Factory = nullptr;
	DX::GetD2D1Factory(&pD2d1Factory);
	Microsoft::WRL::ComPtr<ID2D1TransformedGeometry> pTransformGeo;
	hr = pD2d1Factory->CreateTransformedGeometry(geo, m_dpiTransform, &pTransformGeo);
	assert(SUCCEEDED(hr));
	m_d2dBitmapTar->DrawGeometry(pTransformGeo.Get(), pColorBrush, DX::ConvertPixelsToDips(pen->GetWidth(),m_dpi.width),pStroke);

	SafeRelease(pStroke);
	SafeRelease(pColorBrush);
}

void RenderContext_D3D::FillPath(const IPath* path, const IBrush* brush)
{
	Path_Direct2D *d2dPath = reinterpret_cast<Path_Direct2D*>(const_cast<IPath*>(path));
	assert(d2dPath != nullptr);
	ID2D1Geometry* geo = d2dPath->GetPath();
	assert(geo != nullptr);
	ID2D1Factory *pD2d1Factory = nullptr;
	DX::GetD2D1Factory(&pD2d1Factory);
	Microsoft::WRL::ComPtr<ID2D1TransformedGeometry> pTransformGeo;
	HRESULT hr = pD2d1Factory->CreateTransformedGeometry(geo, m_dpiTransform, &pTransformGeo);
	assert(SUCCEEDED(hr));

	Brush_Direct2D *pBrush = reinterpret_cast<Brush_Direct2D*>(const_cast<IBrush*>(brush));

	m_d2dBitmapTar->FillGeometry(pTransformGeo.Get(), pBrush->GetBrush().Get());
}

void RenderContext_D3D::SetResourceRoot(const std::wstring &path)
{
	m_resourceRoot = path;
}

void RenderContext_D3D::SetTransform()
{
	m_d2dBitmapTar->SetTransform(m_preTransform);
	m_d2dTar->SetTransform(m_preTransform);
}

void RenderContext_D3D::ResetTransform()
{
	m_d2dBitmapTar->SetTransform(m_renderTransform);
	m_d2dTar->SetTransform(m_renderTransform);
}

void RenderContext_D3D::TranslateTransform(int dx, int dy)
{
	D2D1::Matrix3x2F trans = D2D1::Matrix3x2F::Translation(dx, dy);
	m_preTransform = m_preTransform * trans;
}

void RenderContext_D3D::RotateTransform(float angle)
{
	D2D1::Matrix3x2F rot = D2D1::Matrix3x2F::Rotation(angle);
	m_preTransform = m_preTransform * rot;
}

bool RenderContext_D3D::UseDirectX()
{
	return true;
}

ID2D1RenderTarget* RenderContext_D3D::GetRender()
{
	return m_d2dBitmapTar.Get();
}

ID2D1DeviceContext* RenderContext_D3D::GetDeviceContext()
{
	return m_d2dBitmapTar.Get();
}

ID2D1RenderTarget* RenderContext_D3D::BeginD2DDraw()
{
	m_compatD2DBitmap->BeginDraw();
	m_compatD2DBitmap->Clear(D2D1::ColorF(0xffffff,1.0f));
	HRESULT hr = m_compatD2DBitmap->EndDraw();
	if (FAILED(hr)) {
		throw std::exception("EndD2DDraw failure");
	}
	return m_compatD2DBitmap.Get();
}

void RenderContext_D3D::EndD2DDraw(const ui::UiRect& drawRc)
{
	HRESULT hr = S_OK;

	hr = m_compatD2DBitmap->EndDraw();
	if (FAILED(hr)) {
		throw std::exception("EndD2DDraw failure");
	}

	D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(drawRc.left, drawRc.top));
	D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(drawRc.right, drawRc.bottom));

	//m_compatGdiBitmap.RestoreAlpha(drawRc, ui::UiRect(), 0xff);
	D2D1_RECT_F destRc = D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y);

	ID2D1Bitmap *pTarBitmap = nullptr;
	hr = m_compatD2DBitmap->GetBitmap(&pTarBitmap);
	if (FAILED(hr))
	{
		throw std::exception("EndD2DDraw failure");
	}

	ID2D1Effect *pEffect = nullptr;
	do
	{
		HRESULT hr = m_d2dBitmapTar->CreateEffect(CLSID_D2D1ColorMatrix, &pEffect);
		if (FAILED(hr)) {
			break;
		}

		pEffect->SetInput(0, pTarBitmap);
		//bgra or rgba
		D2D1_MATRIX_5X4_F matrix = D2D1::Matrix5x4F(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
			0, 0, 0, 0);
		hr = pEffect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
		assert(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			m_d2dBitmapTar->DrawImage(
				pEffect,
				destLT,
				destRc,
				D2D1_INTERPOLATION_MODE_LINEAR,
				D2D1_COMPOSITE_MODE_SOURCE_OVER
			);
		}

	} while (FALSE);

	SafeRelease(pEffect);

	//m_d2dBitmapTar->DrawBitmap(pTarBitmap, &destRc, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &destRc);

	SafeRelease(pTarBitmap);
}

HDC RenderContext_D3D::BeginGdiDraw(const ui::UiRect& drawRc)
{
	HDC hDC;
	HRESULT hr = m_wndGdiTar->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
	if (FAILED(hr)) {
		//m_compatGdiBitmap.ClearAlpha(drawRc,0x00);
		m_compatGdiBitmap.Clear();
		return m_compatGdiDC;
	}
	else {
	}
	return hDC;
}

void RenderContext_D3D::EndGdiDraw(HDC hDC, const ui::UiRect& drawRc, DWORD fixAlphaColor)
{
	HRESULT hr = m_wndGdiTar->ReleaseDC(&drawRc);
	if (FAILED(hr))
	{
		D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(drawRc.left, drawRc.top));
		D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(drawRc.right, drawRc.bottom));

		D2D1_RECT_F destRc = D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y);
		auto gdiBitmap = CompatibleBitmapFromHBITMAP(hDC, m_compatGdiBitmap.GetBitmap(), m_d2dBitmapTar.Get(), D2D1_ALPHA_MODE_PREMULTIPLIED, false);

		ID2D1Effect *pEffect = nullptr;
		do
		{
			HRESULT hr = m_d2dBitmapTar->CreateEffect(CLSID_RichTextFixAlphaEffect, &pEffect);
			if (FAILED(hr)) {
				break;
			}

			auto textColor = D2D1::RectF(
				(((fixAlphaColor & 0xff000000) >> 24) / 255.0f),
				(((fixAlphaColor & 0x00ff0000) >> 16) / 255.0f),
				(((fixAlphaColor & 0x0000ff00) >> 8) / 255.0f),
				((fixAlphaColor & 0x000000ff) / 255.0f)
			);
			pEffect->SetInput(0, gdiBitmap.Get());
			hr = pEffect->SetValue(
				FIX_ALPHA_PROP_ALPHA,
				textColor
			);
			assert(SUCCEEDED(hr));
			if (SUCCEEDED(hr))
			{
				m_d2dBitmapTar->DrawImage(
					pEffect,
					destLT,
					destRc,
					D2D1_INTERPOLATION_MODE_LINEAR,
					D2D1_COMPOSITE_MODE_SOURCE_OVER
				);
			}

		} while (FALSE);

		SafeRelease(pEffect);

		//m_d2dBitmapTar->DrawBitmap(pTarBitmap, &destRc, 1.0, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &destRc);
	}
	else {
		/*
		D2D1_POINT_2F destLT = m_pixelToWorld.TransformPoint(D2D1::Point2F(drawRc.left, drawRc.top));
		D2D1_POINT_2F destRB = m_pixelToWorld.TransformPoint(D2D1::Point2F(drawRc.right, drawRc.bottom));

		D2D1_RECT_F destRc = D2D1::RectF(destLT.x, destLT.y, destRB.x, destRB.y);
		auto gdiBitmap = compatibleBitmap(this);

		ID2D1Effect *pEffect = nullptr;
		do
		{
			HRESULT hr = m_d2dBitmapTar->CreateEffect(CLSID_RichTextFixAlphaEffect, &pEffect);
			if (FAILED(hr)) {
				break;
			}

			auto textColor = D2D1::RectF(
				(((fixAlphaColor & 0xff000000) >> 24) / 255.0f),
				(((fixAlphaColor & 0x00ff0000) >> 16) / 255.0f),
				(((fixAlphaColor & 0x0000ff00) >> 8) / 255.0f),
				((fixAlphaColor & 0x000000ff) / 255.0f)
			);
			pEffect->SetInput(0, gdiBitmap.Get());
			hr = pEffect->SetValue(
				FIX_ALPHA_PROP_ALPHA,
				textColor
			);
			assert(SUCCEEDED(hr));
			if (SUCCEEDED(hr))
			{
				m_d2dBitmapTar->DrawImage(
					pEffect,
					destLT,
					destRc,
					D2D1_INTERPOLATION_MODE_LINEAR,
					D2D1_COMPOSITE_MODE_SOURCE_OVER
				);
			}

		} while (FALSE);

		SafeRelease(pEffect);*/
	}
}

void RenderContext_D3D::BeginPaint(const UiRect* rcPaint)
{
	HRESULT hr = S_OK;
	static UiRect s_maxPaintRc(0,0,0,0);
	if (rcPaint != nullptr)
	{
		m_repaintRC = *rcPaint;
		if (s_maxPaintRc.GetWidth() < rcPaint->GetWidth()) {
			s_maxPaintRc.left = rcPaint->left;
			s_maxPaintRc.right = rcPaint->right;
		}
		if (s_maxPaintRc.GetHeight() < rcPaint->GetHeight()) {
			s_maxPaintRc.top = rcPaint->top;
			s_maxPaintRc.bottom = rcPaint->bottom;
		}
		hr = m_d2dTar->BindDC(m_hDC, &s_maxPaintRc);
		if (SUCCEEDED(hr))
		{
			HRESULT hr = m_d2dTar.Get()->QueryInterface(IID_ID2D1DeviceContext, &m_d2dBitmapTar);
			if (!SUCCEEDED(hr))
			{
				throw std::exception("D2D create bitmap failure");
			}

			hr = m_d2dBitmapTar.Get()->QueryInterface(__uuidof(ID2D1GdiInteropRenderTarget), (void**)&m_wndGdiTar);
			if (FAILED(hr))
			{
				throw std::exception("ID2D1GdiInteropRenderTarget get failure");
			}

			D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_TARGET /*| D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE*/,  //GDI 兼容，用于富文本内容绘制
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
			Microsoft::WRL::ComPtr<ID2D1Bitmap1> pTarBitmap;
			auto bitmapSize = m_d2dBitmapTar->GetPixelSize();
			hr = m_d2dBitmapTar->CreateBitmap(bitmapSize, NULL, bitmapSize.width * 4, bitmapProperties, &pTarBitmap);
			if (FAILED(hr)) {
				throw std::exception("Device Context CreateBitmap");
			}
			m_curBitmap = pTarBitmap;  //用于alphablend

			m_d2dBitmapTar->GetTarget(&m_paintBitmap);
			//m_d2dBitmapTar->SetAntialiasMode(D2D1_ANTIALIAS_MODE)
			m_d2dBitmapTar->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
			auto am = m_d2dBitmapTar->GetAntialiasMode();
			auto tam = m_d2dBitmapTar->GetTextAntialiasMode();
			m_d2dBitmapTar->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
			//select bitmap to memory dc
#ifdef D2D_USE_MEMDC
			{
				if (m_hOldGdiBitmap != NULL)
				{
					::SelectObject(m_hDC, m_hOldGdiBitmap);
				}

				bool ret = m_gdiBitmap.Init(m_hDC, m_d2dBitmapTar->GetPixelSize().width, m_d2dBitmapTar->GetPixelSize().height, m_bFlipDraw);
				m_hOldGdiBitmap = (HBITMAP)::SelectObject(m_hDC, m_gdiBitmap.GetBitmap());
			}
#endif
			{
				m_compatGdiBitmap.Init(m_compatGdiDC, m_d2dBitmapTar->GetPixelSize().width, m_d2dBitmapTar->GetPixelSize().height, m_bFlipDraw);
				m_compatGdiBitmap.Clear();
				::SelectObject(m_compatGdiDC, m_compatGdiBitmap.GetBitmap());
				hr = m_d2dTar->CreateCompatibleRenderTarget(&m_compatD2DBitmap);
				if (FAILED(hr))
				{
					throw std::exception("CreateCompatibleRenderTarget failure");
				}
			}
		}
		
	}
	m_d2dTar->GetTransform(&m_renderTransform);
	m_d2dBitmapTar->SetTransform(m_renderTransform);
	m_worldToPixel = m_renderTransform * m_dpiTransform;
	m_pixelToWorld = m_worldToPixel;
	m_pixelToWorld.Invert();
	if (!m_beginDraw && rcPaint)
	{
		saveRepaint(s_maxPaintRc);
		m_d2dBitmapTar->SetTarget(m_curBitmap.Get());
		m_d2dBitmapTar->BeginDraw();
		m_beginDraw = true;
	}
}

void RenderContext_D3D::EndPaint()
{
	//Bitmap draw into memDC
	HRESULT hr = m_d2dBitmapTar->EndDraw();
	if (FAILED(hr))
	{
		throw std::exception("direct2d EndPaint step1 Fault");
	}
	m_beginDraw = false;
	
	D2D1::Matrix3x2F tmpM;
	m_d2dTar->GetTransform(&tmpM);
	m_d2dBitmapTar->GetTransform(&tmpM);
	m_d2dBitmapTar->SetTarget(m_paintBitmap.Get());
	m_d2dTar->BeginDraw();
	D2D1_SIZE_F sSize = m_curBitmap->GetSize();
	D2D1_SIZE_F tSize = m_d2dTar->GetSize();
	m_d2dTar->DrawBitmap(m_curBitmap.Get(), D2D1::RectF(0,0, tSize.width, tSize.height),1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0,0, sSize.width, sSize.height));
	hr = m_d2dTar->EndDraw();
	m_d2dBitmapTar->SetTarget(m_curBitmap.Get());
	if (FAILED(hr))
	{
		if (D2DERR_RECREATE_TARGET == hr)
		{
			auto size = m_d2dBitmapTar->GetSize();
			auto pixSize = m_d2dBitmapTar->GetPixelSize();
			
			Microsoft::WRL::ComPtr<ID2D1DeviceContext> ctx = CreateD2DContext();

			ID2D1Factory *pFactoryFromCtx = nullptr;
			ctx->GetFactory(&pFactoryFromCtx);

			m_d2dTar = CreateDCRender(pFactoryFromCtx);

			SafeRelease(pFactoryFromCtx);
		}
		else {
			assert(SUCCEEDED(hr));
			throw std::exception("D2D end draw fail");
		}
	}
	else {
		;
	}
}

HRESULT createWicBitmap(
	IWICImagingFactory *pWicFactory,
	ID2D1RenderTarget *pRender,
	LPCTSTR szFileName,
	ID2D1Bitmap ***pppBitmap,
	UINT *pWidth,
	UINT *pHeight,
	UINT **ppDuration,
	UINT *pFrameCount)
{
	if (pppBitmap == nullptr)
		return E_INVALIDARG;
	if (pFrameCount == nullptr)
		return E_INVALIDARG;
	if (ppDuration == nullptr)
		return E_INVALIDARG;

	IWICBitmapDecoder *pDecoder = NULL;
	HRESULT hr = pWicFactory->CreateDecoderFromFilename(
		szFileName,                      // Image to be decoded
		NULL,                            // Do not prefer a particular vendor
		GENERIC_READ,                    // Desired read access to the file
		WICDecodeMetadataCacheOnDemand,  // Cache metadata when needed
		&pDecoder                        // Pointer to the decoder
	);
	if (FAILED(hr))
	{
		return hr;
	}
	// Retrieve the first frame of the image from the decoder
	IWICFormatConverter *pConverter = NULL;
	UINT frameCount = 0;
	pDecoder->GetFrameCount(&frameCount);
	if (pFrameCount != NULL)
		*pFrameCount = frameCount;

	*pppBitmap = new ID2D1Bitmap *[frameCount];
	*ppDuration = new UINT[frameCount];
	UINT maxWidth = 0;
	UINT maxHeight = 0;
	for (UINT ind = 0; ind < frameCount; ++ind)
	{
		IWICBitmapFrameDecode *pFrame = NULL;
		hr = pDecoder->GetFrame(ind, &pFrame);
		if (SUCCEEDED(hr))
		{
			UINT w = 0, h = 0;
			HRESULT subHr = pFrame->GetSize(&w, &h);
			if (pWidth != nullptr)
				*pWidth = w;
			if (pHeight != nullptr)
				*pHeight = h;

			(*ppDuration)[ind] = 100; //先设置默认
									  // 通过帧的属性检索器获取帧的时长
			IWICMetadataQueryReader* pFrameMetadata = nullptr;
			subHr = pFrame->GetMetadataQueryReader(&pFrameMetadata);
			assert(SUCCEEDED(subHr));
			if (SUCCEEDED(subHr))
			{
				PROPVARIANT propDuration;
				PropVariantInit(&propDuration);

				subHr = pFrameMetadata->GetMetadataByName(L"/grctlext/Delay", &propDuration);
				assert(SUCCEEDED(subHr));
				if (SUCCEEDED(subHr))
				{
					if (propDuration.vt == VT_UI2) {
						WORD duration = propDuration.uiVal;
						//std::cout << "Frame " << i << " duration: " << duration * 10 << " ms" << std::endl;
						(*ppDuration)[ind] = duration * 10;
					}

					PropVariantClear(&propDuration);
				}

			}


			SafeRelease(pFrameMetadata);
		}
		else {
			(*ppDuration)[ind] = 100;
		}

		hr = pWicFactory->CreateFormatConverter(&pConverter);
		if (SUCCEEDED(hr))
		{
			hr = pConverter->Initialize(pFrame,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,         // Specified dither pattern
				NULL,                            // Specify a particular palette 
				0.f,                             // Alpha threshold
				WICBitmapPaletteTypeCustom       // Palette translation type
			);
			if (SUCCEEDED(hr))
			{
				//分配一张BITMAP
				auto compateBitmapProp = D2D1::BitmapProperties(
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
				);
				ID2D1Bitmap *pBitmap = nullptr;
				hr = pRender->CreateBitmapFromWicBitmap(pConverter, &compateBitmapProp, &pBitmap);
				if (SUCCEEDED(hr))
				{
					(*pppBitmap)[ind] = pBitmap;
				}
				else {
					(*pppBitmap)[ind] = nullptr;
				}
				SafeRelease(pConverter);
			}
		}
		SafeRelease(pFrame);
	}

	SafeRelease(pDecoder);
	return hr;
}
} // namespace ui
