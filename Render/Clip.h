#ifndef UI_RENDER_CLIP_H_
#define UI_RENDER_CLIP_H_

#pragma once
#include <stack>

namespace ui 
{

class UILIB_API GdiClip : public IClip
{
public:
	GdiClip();
    ~GdiClip();

	virtual void CreateClip(HDC hDC, UiRect rc) override;
	virtual void CreateRoundClip(HDC hDC, UiRect rc, int width, int height) override;
	virtual void ClearClip(HDC hDC) override;
};

class UILIB_API D2DClip : public IClip2
{
public:
	D2DClip();
	~D2DClip();

	virtual void CreateClip2(HDC hdc, ID2D1RenderTarget *pRender, UiRect rc) override;
	virtual void CreateRoundClip2(HDC hdc, ID2D1RenderTarget *pRender, UiRect rc, int width, int height) override;
	virtual void ClearClip2(ID2D1RenderTarget *pRender) override;
private:
	std::stack<int> m_layers;
	ID2D1RenderTarget *m_pClipRender;
};


} // namespace ui

#endif // UI_RENDER_CLIP_H_
