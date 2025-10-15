#include "StdAfx.h"
#include "DirectXHelp.h"

namespace ui {

GdiClip::GdiClip()
{

}

GdiClip::~GdiClip()
{

}

void GdiClip::CreateClip(HDC hDC, UiRect rcItem)
{
	if (hDC != NULL) {
		CPoint ptWinOrg;
		GetWindowOrgEx(hDC, &ptWinOrg);
		rcItem.Offset(-ptWinOrg.x, -ptWinOrg.y);

		HRGN hRgn = ::CreateRectRgnIndirect(&rcItem);
		::SaveDC(hDC);
		::ExtSelectClipRgn(hDC, hRgn, RGN_AND);
		::DeleteObject(hRgn);
	}
}

void GdiClip::CreateRoundClip(HDC hDC, UiRect rcItem, int width, int height)
{
	if (hDC != NULL) {
		CPoint ptWinOrg;
		GetWindowOrgEx(hDC, &ptWinOrg);
		rcItem.Offset(-ptWinOrg.x, -ptWinOrg.y);

		HRGN hRgn = ::CreateRoundRectRgn(rcItem.left, rcItem.top, rcItem.right + 1, rcItem.bottom + 1, width, height);
		::SaveDC(hDC);
		::ExtSelectClipRgn(hDC, hRgn, RGN_AND);
		::DeleteObject(hRgn);
	}
}

void GdiClip::ClearClip(HDC hDC)
{
	if (hDC != NULL) {
		ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
		::RestoreDC(hDC, -1);
	}
}


/************************************************************************/
/* D2D 版本实现                                                                     */
/************************************************************************/

D2DClip::D2DClip() : m_pClipRender(nullptr)
{

}

D2DClip::~D2DClip()
{
	if (m_pClipRender != nullptr)
	{
		this->ClearClip2(m_pClipRender);
	}
}

void D2DClip::CreateClip2(HDC hDC, ID2D1RenderTarget *pRender, UiRect rcItem)
{
	/*
	D2D1_ANTIALIAS_MODE_PER_PRIMITIVE：这是默认的抗锯齿模式，它对每个图元（如线条、文本、形状等）进行抗锯齿处理。这意味着不仅图形的边缘会被抗锯齿处理，而且图形内部的内容也会被抗锯齿处理，以实现平滑的外观。
	D2D1_ANTIALIAS_MODE_ALIASED：这个模式表示禁用抗锯齿处理，图形不会进行平滑处理，锯齿状的边缘可能会出现。
	*/
	FLOAT x, y;
	pRender->GetDpi(&x, &y);
	pRender->PushAxisAlignedClip(
		D2D1::RectF(DX::ConvertPixelsToDips(rcItem.left,x),
			DX::ConvertPixelsToDips(rcItem.top,y),
			DX::ConvertPixelsToDips(rcItem.right,x),
			DX::ConvertPixelsToDips(rcItem.bottom,y)),
		D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	m_layers.push(0);
	if (m_pClipRender != nullptr)
	{
		assert(m_pClipRender == pRender);
	}
	m_pClipRender = pRender;
}

void D2DClip::CreateRoundClip2(HDC hDC, ID2D1RenderTarget *pRender, UiRect rcItem, int width, int height)
{	
	ID2D1Factory *pFactory = nullptr;
	pRender->GetFactory(&pFactory);
	ID2D1RoundedRectangleGeometry *pRoundedRectGeo = nullptr;
	ID2D1Layer *pLayer = nullptr;
	do 
	{
		FLOAT x, y;
		pRender->GetDpi(&x, &y);
		HRESULT hr = pFactory->CreateRoundedRectangleGeometry(D2D1::RoundedRect(D2D1::RectF(DX::ConvertPixelsToDips(rcItem.left, x),
			DX::ConvertPixelsToDips(rcItem.top, y),
			DX::ConvertPixelsToDips(rcItem.right, x),
			DX::ConvertPixelsToDips(rcItem.bottom, y)), 
			width, height), &pRoundedRectGeo);
		assert(SUCCEEDED(hr));
		if (FAILED(hr))
			break;
		hr = pRender->CreateLayer(&pLayer);
		assert(SUCCEEDED(hr));
		if (FAILED(hr))
			break;
		//pRender->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), pRoundedRectGeo), pLayer);
		pRender->PushLayer(D2D1::LayerParameters(
			D2D1::InfiniteRect(), 
			pRoundedRectGeo, 
			D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
			D2D1::IdentityMatrix(),
			1.0,
			NULL,
			D2D1_LAYER_OPTIONS_INITIALIZE_FOR_CLEARTYPE), pLayer);
		m_layers.push(1);
		if (m_pClipRender != nullptr)
		{
			assert(m_pClipRender == pRender);
		}
		m_pClipRender = pRender;
	} while (false);

	SafeRelease(pFactory);
	SafeRelease(pRoundedRectGeo);
	SafeRelease(pLayer);
}

void D2DClip::ClearClip2(ID2D1RenderTarget *pRender)
{
	if (m_layers.empty())
		return;

	switch (m_layers.top())
	{
	case 0:
		pRender->PopAxisAlignedClip();
		break;
	case 1:
		pRender->PopLayer();
		break;
	}
	m_layers.pop();
}


} // namespace ui
