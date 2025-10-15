#include "StdAfx.h"
#include <d2d1_1.h>

namespace ui {

using namespace Gdiplus;

Brush_Gdiplus::Brush_Gdiplus(DWORD color)
	: IBrush(color)
{
	brush_.reset(new SolidBrush(color));
}

Brush_Gdiplus::Brush_Gdiplus(HBITMAP bitmap)
	: IBrush(bitmap)
{
	Gdiplus::Bitmap image(bitmap, NULL);
	bitmap_brush_.reset(new TextureBrush(&image));
}

Brush_Gdiplus::Brush_Gdiplus(const Brush_Gdiplus& r)
	: IBrush(r)
{
	if (r.brush_)
		brush_.reset(r.brush_->Clone());

	if (r.bitmap_brush_)
		bitmap_brush_.reset(r.bitmap_brush_->Clone());
}

ui::IBrush* Brush_Gdiplus::Clone()
{
	return new Brush_Gdiplus(*this);
}




Brush_Direct2D::Brush_Direct2D(DWORD color, ID2D1RenderTarget* pRT)
	: IBrush(color)
{
	int alpha = color >> 24;
	ID2D1SolidColorBrush *pColorBrush = nullptr;
	HRESULT hr = pRT->CreateSolidColorBrush(D2D1::ColorF(color & 0x00FFFFFF, alpha / 255.0f),&pColorBrush);
	ASSERT(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		brush_ = pColorBrush;
		SafeRelease(pColorBrush);
	}
}

Brush_Direct2D::Brush_Direct2D(HBITMAP bitmap, ID2D1RenderTarget* pRT)
	: IBrush(bitmap)
{
	BITMAP info;
	if (0 == ::GetObject(bitmap, sizeof(info), &info))
	{
		ASSERT(0);
		return;
	}
	LPBYTE pPixel = LPBYTE(info.bmBits);
	WORD bytesPerPixel = info.bmBitsPixel / 8;
	FLOAT opacity = 1.0f;
	D2D1_BITMAP_PROPERTIES bitmapProperties = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
	ID2D1Bitmap *pImageBitmap = nullptr;
	HRESULT hr = pRT->CreateBitmap(D2D1::SizeU(info.bmWidth, info.bmHeight), bitmapProperties, &pImageBitmap);
	ASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		DeleteObject(bitmap);
		return;
	}

	auto imageRt = D2D1::RectU(0, 0, info.bmWidth, info.bmHeight);
	hr = pImageBitmap->CopyFromMemory(&imageRt, pPixel, info.bmWidthBytes);
	ASSERT(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		ID2D1BitmapBrush *pBitmapBrush = nullptr;
		hr = pRT->CreateBitmapBrush(pImageBitmap, &pBitmapBrush);
		ASSERT(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			brush_ = pBitmapBrush;
			SafeRelease(pBitmapBrush);
		}
	}
	SafeRelease(pImageBitmap);
	DeleteObject(bitmap);
}

Brush_Direct2D::Brush_Direct2D(const Brush_Direct2D& r)
	: IBrush(r)
{
	brush_ = r.brush_;
}

ui::IBrush* Brush_Direct2D::Clone()
{
	return new Brush_Direct2D(*this);
}

} // namespace ui
