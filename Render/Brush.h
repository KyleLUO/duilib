#ifndef UI_CORE_RENDER_BRUSH_H_
#define UI_CORE_RENDER_BRUSH_H_

#pragma once
#include <GdiPlus.h>

namespace ui 
{

class UILIB_API Brush_Gdiplus : public IBrush
{
public:
	Brush_Gdiplus(DWORD color);
	Brush_Gdiplus(HBITMAP bitmap);
	Brush_Gdiplus(const Brush_Gdiplus& r);
	Brush_Gdiplus& operator=(const Brush_Gdiplus& r) = delete;

	virtual IBrush* Clone() override;

	Gdiplus::Brush* GetBrush() { return brush_ ? brush_.get() : bitmap_brush_.get(); };
private:
	std::unique_ptr<Gdiplus::Brush> brush_;
	std::unique_ptr<Gdiplus::Brush> bitmap_brush_;
};

class UILIB_API Brush_Direct2D : public IBrush
{
public:
	Brush_Direct2D(DWORD color,ID2D1RenderTarget* pRT);
	Brush_Direct2D(HBITMAP bitmap, ID2D1RenderTarget* pRT);
	Brush_Direct2D(const Brush_Direct2D& r);
	Brush_Direct2D& operator=(const Brush_Direct2D& r) = delete;

	virtual IBrush* Clone() override;

	Microsoft::WRL::ComPtr<ID2D1Brush> GetBrush() { return brush_; };
private:
	Microsoft::WRL::ComPtr<ID2D1Brush> brush_;
};

} // namespace ui

#endif // UI_CORE_RENDER_BRUSH_H_
