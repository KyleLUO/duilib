#include "StdAfx.h"

namespace ui {

ui::IPen* RenderFactory_GdiPlus::CreatePen(DWORD color, int width /*= 1*/, ui::IRenderContext* ctx/*=nullptr*/)
{
	return new Pen_GdiPlus(color, width);
}

ui::IBrush* RenderFactory_GdiPlus::CreateBrush(DWORD color, ui::IRenderContext* ctx/*=nullptr*/)
{
	return new Brush_Gdiplus(color);
}

ui::IBrush* RenderFactory_GdiPlus::CreateBrush(HBITMAP bitmap, ui::IRenderContext* ctx/*=nullptr*/)
{
	return new Brush_Gdiplus(bitmap);
}

ui::IMatrix* RenderFactory_GdiPlus::CreateMatrix()
{
	return new Matrix_Gdiplus();
}

ui::IPath* RenderFactory_GdiPlus::CreatePath(ui::IRenderContext* ctx/*=nullptr*/)
{
	return new Path_Gdiplus();
}

ui::IBitmap* RenderFactory_GdiPlus::CreateBitmap(ui::IRenderContext* ctx/*=nullptr*/)
{
	return new GdiBitmap();
}

ui::IRenderContext* RenderFactory_GdiPlus::CreateRenderContext(HWND wnd)
{
	return new RenderContext_GdiPlus();
}

ui::IRenderContext* RenderFactory_GdiPlus::CreateRenderContext(ui::IRenderContext *pSourceRender)
{
	return new RenderContext_GdiPlus();
}





ui::IPen* RenderFactory_D3D::CreatePen(DWORD color, int width /*= 1*/, ui::IRenderContext* ctx/*=nullptr*/)
{
	return new Pen_GdiPlus(color, width);
}

ui::IBrush* RenderFactory_D3D::CreateBrush(DWORD color, ui::IRenderContext* ctx/*=nullptr*/)
{
	return new Brush_Direct2D(color, ctx->GetRender());
}

ui::IBrush* RenderFactory_D3D::CreateBrush(HBITMAP bitmap, ui::IRenderContext* ctx/*=nullptr*/)
{
	return new Brush_Direct2D(bitmap, ctx->GetRender());
}

ui::IMatrix* RenderFactory_D3D::CreateMatrix()
{
	return new Matrix_Direct2D();
}

ui::IPath* RenderFactory_D3D::CreatePath(ui::IRenderContext* ctx/*=nullptr*/)
{
	return new Path_Direct2D();
}

ui::IBitmap* RenderFactory_D3D::CreateBitmap(ui::IRenderContext* ctx/*=nullptr*/)
{
	return new GdiBitmap();
}

ui::IRenderContext* RenderFactory_D3D::CreateRenderContext(HWND wnd)
{
	return new RenderContext_D3D(wnd);
}

ui::IRenderContext* RenderFactory_D3D::CreateRenderContext(ui::IRenderContext *pSourceRender)
{
	return new RenderContext_D3D(reinterpret_cast<RenderContext_D3D*>(pSourceRender));
}

} // namespace ui
