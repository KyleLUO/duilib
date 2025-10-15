#ifndef UI_CORE_RENDER_PEN_H_
#define UI_CORE_RENDER_PEN_H_

#pragma once
#include <GdiPlus.h>

namespace ui 
{

class UILIB_API Pen_GdiPlus : public IPen
{
public:
	Pen_GdiPlus(DWORD color, int width = 1);
	Pen_GdiPlus(const Pen_GdiPlus& r);
	Pen_GdiPlus& operator=(const Pen_GdiPlus& r) = delete;

	virtual IPen* Clone() override;

	virtual void SetWidth(int width) override;
	virtual int GetWidth()const override;
	virtual void SetColor(DWORD color) override;

	virtual void SetStartCap(LineCap cap) override;
	virtual void SetEndCap(LineCap cap) override;
	virtual void SetDashCap(LineCap cap) override;
	virtual LineCap GetStartCap()const override;
	virtual LineCap GetEndCap()const override;
	virtual LineCap GetDashCap()const override;

	virtual void SetLineJoin(LineJoin join) override;
	virtual LineJoin GetLineJoin()const override;

	virtual void SetDashStyle(DashStyle style) override;
	virtual DashStyle GetDashStyle()const override;

	Gdiplus::Pen* GetPen() { return pen_.get(); };
private:
	std::unique_ptr<Gdiplus::Pen> pen_;
};

} // namespace ui

#endif // UI_CORE_RENDER_PEN_H_
