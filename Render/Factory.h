#ifndef UI_CORE_RENDER_FACTORY_H_
#define UI_CORE_RENDER_FACTORY_H_
#pragma once

namespace ui 
{

class UILIB_API RenderFactory_GdiPlus : public IRenderFactory
{
public:
	virtual ui::IPen* CreatePen(DWORD color, int width = 1, ui::IRenderContext* ctx=nullptr) override;
	virtual ui::IBrush* CreateBrush(DWORD corlor, ui::IRenderContext* ctx = nullptr) override;
	virtual ui::IBrush* CreateBrush(HBITMAP bitmap, ui::IRenderContext* ctx = nullptr) override;
	virtual ui::IMatrix* CreateMatrix() override;
	virtual ui::IPath* CreatePath(ui::IRenderContext* ctx = nullptr) override;
	virtual ui::IBitmap* CreateBitmap(ui::IRenderContext* ctx = nullptr) override;
	virtual ui::IRenderContext* CreateRenderContext(HWND wnd) override;
	virtual ui::IRenderContext* CreateRenderContext(ui::IRenderContext *pSourceRender) override;
};

class UILIB_API RenderFactory_D3D : public IRenderFactory
{
public:
	virtual ui::IPen* CreatePen(DWORD color, int width = 1, ui::IRenderContext* ctx=nullptr) override;
	virtual ui::IBrush* CreateBrush(DWORD corlor, ui::IRenderContext* ctx=nullptr) override;
	virtual ui::IBrush* CreateBrush(HBITMAP bitmap, ui::IRenderContext* ctx=nullptr) override;
	virtual ui::IMatrix* CreateMatrix() override;
	virtual ui::IPath* CreatePath(ui::IRenderContext* ctx=nullptr) override;
	virtual ui::IBitmap* CreateBitmap(ui::IRenderContext* ctx=nullptr) override;
	virtual ui::IRenderContext* CreateRenderContext(HWND wnd) override;
	virtual ui::IRenderContext* CreateRenderContext(ui::IRenderContext *pSourceRender) override;
};

} // namespace ui

#endif // UI_CORE_RENDER_FACTORY_H_
