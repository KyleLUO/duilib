#ifndef UI_CORE_RENDER_H_
#define UI_CORE_RENDER_H_

#pragma once
#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_VISTA
#endif
#include <d2d1_1.h>
#include <dwrite_1.h>
#include <wrl/client.h>
#include <wincodec.h>
#include "TextRender.h"

namespace ui 
{
	extern HRESULT createWicBitmap(
		IWICImagingFactory *pWicFactory,
		ID2D1RenderTarget *pRender,
		LPCTSTR szFileName,
		ID2D1Bitmap ***pppBitmap,
		UINT *pWidth,
		UINT *pHeight,
		UINT **ppDuration,
		UINT *pFrameCount);

	struct tagTFontInfo;

class UILIB_API RenderContext_GdiPlus : public IRenderContext
{
public:
	RenderContext_GdiPlus();
	virtual ~RenderContext_GdiPlus();

	virtual HDC GetDC() override;
	virtual bool Resize(int width, int height, bool flipBItmap = true) override;
	virtual void Clear() override;
	virtual std::unique_ptr<IRenderContext> Clone() override;

	virtual HBITMAP DetachBitmap() override;
	virtual BYTE* GetBits() override;
	virtual int	GetWidth() override;
	virtual int GetHeight() override;
	virtual void ClearAlpha(const UiRect& rcDirty, int alpha = 0) override;
	virtual void RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding = UiRect(), int alpha = 0) override;

	virtual bool IsRenderTransparent() const override;
	virtual bool SetRenderTransparent(bool bTransparent) override;

	virtual void Save() override;
	virtual void Restore() override;

	virtual CPoint OffsetWindowOrg(CPoint ptOffset) override;
	virtual CPoint SetWindowOrg(CPoint ptOffset) override;
	virtual CPoint GetWindowOrg() const override;

	virtual void SetClip(const UiRect& rc) override;
	virtual void SetRoundClip(const UiRect& rc, int width, int height) override;
	virtual void ClearClip() override;

	virtual HRESULT BitBlt(int x, int y, int cx, int cy, HDC hdcSrc, int xSrc = 0, int yScr = 0, DWORD rop = SRCCOPY) override;
	virtual bool AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, HDC hdcSrc, int xSrc, int yScr, int widthSrc, int heightSrc, BYTE uFade = 255) override;

	virtual HRESULT BitBlt(int x, int y, int cx, int cy, IRenderContext *srcCtx, int xSrc = 0, int yScr = 0, DWORD rop = SRCCOPY) override;
	virtual bool AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, IRenderContext *srcCtx, int xSrc, int yScr, int widthSrc, int heightSrc, BYTE uFade = 255) override;

	virtual void DrawImage(const UiRect& rcPaint, HBITMAP hBitmap, bool bAlphaChannel,
		const UiRect& rcImageDest, const UiRect& rcImageSource, const UiRect& rcCorners, BYTE uFade = 255, bool xtiled = false, bool ytiled = false) override;

	virtual void DrawColor(const UiRect& rc, DWORD dwColor, BYTE uFade = 255) override;
	virtual void DrawColor(const UiRect& rc, const std::wstring& colorStr, BYTE uFade = 255) override;
	virtual void DrawGradientColor(const UiRect& rc, const std::wstring& colorStr, const std::wstring& colorStr1, const int mode = 0) override;

	virtual void DrawLine(const UiRect& rc, int nSize, DWORD dwPenColor) override;
	virtual void DrawLine(const IPen* pen, int x1, int y1, int x2, int y2) override;
	virtual void DrawBezier(const IPen* pen, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) override;
	virtual void DrawRect(const UiRect& rc, int nSize, DWORD dwPenColor) override;
	virtual void DrawRoundRect(const UiRect& rc, const CSize& roundSize, int nSize, DWORD dwPenColor) override;
	virtual void DrawText(const UiRect& rc, const std::wstring& strText, DWORD dwTextColor, const std::wstring& strFontId, UINT uStyle, BYTE uFade = 255, bool bLineLimit = false) override;
	virtual void DrawHighlightText(const UiRect& rc, const std::wstring& strText, DWORD dwTextColor, 
		const std::wstring& strHighlightText, DWORD dwHighlightColor,
		const std::wstring& strFontId, UINT uStyle, BYTE uFade = 255, bool bLineLimit = false) override;

	virtual void DrawHtmlText(const UiRect& rc_item, const std::wstring& strText, DWORD dwTextColor, const std::wstring& strFontId, UINT uStyle, RECT* prcLinks);

	virtual void DrawEllipse(const UiRect& rc, int nSize, DWORD dwColor) override;
	virtual void FillEllipse(const UiRect& rc, DWORD dwColor) override;

	virtual UiRect MeasureText(const std::wstring& strText, const std::wstring& strFontId, UINT uStyle, int width = DUI_NOSET_VALUE) override;

	virtual void DrawPath(const IPath* path, const IPen* pen) override;
	virtual void FillPath(const IPath* path, const IBrush* brush) override;

	virtual void SetResourceRoot(const std::wstring &path) override;

	//矩阵变换相关
	virtual void SetTransform() override;
	virtual void ResetTransform() override;
	virtual void TranslateTransform(int dx, int dy) override;
	virtual void RotateTransform(float angle) override;

	//GDI+ 无任何操作，为了兼容DX
	virtual void BeginPaint(const UiRect* rcPaint) override;
	virtual void EndPaint() override;
	virtual bool UseDirectX() override;
	virtual ID2D1RenderTarget* GetRender() override;
private:
	HDC			m_hDC;
	int			m_saveDC;
	HBITMAP		m_hOldBitmap;

	bool		m_bTransparent;
	GdiClip		m_clip;
	GdiBitmap	m_bitmap;
};

class UILIB_API RenderContext_D3D : public IRenderContext
{
public:
	struct _HtmlTextEntry {
		_HtmlTextEntry() : start(0), len(0), childs(nullptr), next(nullptr)
		{
			memset(name, '\0', ARRAYSIZE(name));
			memset(color, 0, ARRAYSIZE(color));
		}
		char name[8];
		std::wstring text;
		size_t start;
		size_t len;
		_HtmlTextEntry* childs;
		_HtmlTextEntry* next;

		//attr
		union
		{
			char color[16];
			int  font_id;
			struct _img_attr {
				int  count;
				int  index;
				FLOAT max_width;
				FLOAT min_height;
			}img_attr;
		};
		std::wstring path; // href / src
	};
public:
	RenderContext_D3D(HWND wnd);
	RenderContext_D3D(RenderContext_D3D *pSourceRender);

	virtual ~RenderContext_D3D();

	virtual HDC GetDC() override;
	virtual bool Resize(int width, int height, bool flipBItmap = true) override;
	virtual void Clear() override;
	virtual std::unique_ptr<IRenderContext> Clone() override;

	virtual HBITMAP DetachBitmap() override;
	virtual BYTE* GetBits() override;
	virtual int	GetWidth() override;
	virtual int GetHeight() override;
	virtual void ClearAlpha(const UiRect& rcDirty, int alpha = 0) override;
	virtual void RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding = UiRect(), int alpha = 0) override;

	virtual bool IsRenderTransparent() const override;
	virtual bool SetRenderTransparent(bool bTransparent) override;

	virtual void Save() override;
	virtual void Restore() override;

	virtual CPoint OffsetWindowOrg(CPoint ptOffset) override;
	virtual CPoint SetWindowOrg(CPoint ptOffset) override;
	virtual CPoint GetWindowOrg() const override;

	virtual void SetClip(const UiRect& rc) override;
	virtual void SetRoundClip(const UiRect& rc, int width, int height) override;
	virtual void ClearClip() override;

	virtual HRESULT BitBlt(int x, int y, int cx, int cy, HDC hdcSrc, int xSrc = 0, int yScr = 0, DWORD rop = SRCCOPY) override;
	virtual bool AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, HDC hdcSrc, int xSrc, int yScr, int widthSrc, int heightSrc, BYTE uFade = 255) override;

	virtual HRESULT BitBlt(int x, int y, int cx, int cy, IRenderContext *srcCtx, int xSrc = 0, int yScr = 0, DWORD rop = SRCCOPY) override;
	virtual bool AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, IRenderContext *srcCtx, int xSrc, int yScr, int widthSrc, int heightSrc, BYTE uFade = 255) override;

	virtual void DrawImage(const UiRect& rcPaint, HBITMAP hBitmap, bool bAlphaChannel,
		const UiRect& rcImageDest, const UiRect& rcImageSource, const UiRect& rcCorners, BYTE uFade = 255, bool xtiled = false, bool ytiled = false) override;

	virtual void DrawColor(const UiRect& rc, DWORD dwColor, BYTE uFade = 255) override;
	virtual void DrawColor(const UiRect& rc, const std::wstring& colorStr, BYTE uFade = 255) override;
	virtual void DrawGradientColor(const UiRect& rc, const std::wstring& colorStr, const std::wstring& colorStr1, const int mode = 0) override;

	virtual void DrawLine(const UiRect& rc, int nSize, DWORD dwPenColor) override;
	virtual void DrawLine(const IPen* pen, int x1, int y1, int x2, int y2) override;
	virtual void DrawBezier(const IPen* pen, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) override;
	virtual void DrawRect(const UiRect& rc, int nSize, DWORD dwPenColor) override;
	virtual void DrawRoundRect(const UiRect& rc, const CSize& roundSize, int nSize, DWORD dwPenColor) override;
	virtual void DrawText(const UiRect& rc, const std::wstring& strText, DWORD dwTextColor, const std::wstring& strFontId, UINT uStyle, BYTE uFade = 255, bool bLineLimit = false) override;
	virtual void DrawHighlightText(const UiRect& rc, const std::wstring& strText, DWORD dwTextColor,
		const std::wstring& strHighlightText, DWORD dwHighlightColor,
		const std::wstring& strFontId, UINT uStyle, BYTE uFade = 255, bool bLineLimit = false) override;

	virtual void DrawHtmlText(const UiRect& rc_item, const std::wstring& strText, DWORD dwTextColor, const std::wstring& strFontId, UINT uStyle, RECT* prcLinks);

	virtual void DrawEllipse(const UiRect& rc, int nSize, DWORD dwColor) override;
	virtual void FillEllipse(const UiRect& rc, DWORD dwColor) override;

	virtual UiRect MeasureText(const std::wstring& strText, const std::wstring& strFontId, UINT uStyle, int width = DUI_NOSET_VALUE) override;

	virtual void DrawPath(const IPath* path, const IPen* pen) override;
	virtual void FillPath(const IPath* path, const IBrush* brush) override;

	virtual void SetResourceRoot(const std::wstring &path) override;
	//矩阵变换相关
	virtual void SetTransform() override;
	virtual void ResetTransform() override;
	virtual void TranslateTransform(int dx, int dy) override;
	virtual void RotateTransform(float angle) override;

	virtual void BeginPaint(const UiRect* rcPaint) override;
	virtual void EndPaint() override;
	virtual bool UseDirectX() override;
	virtual ID2D1RenderTarget* GetRender() override;
	
	ID2D1DeviceContext* GetDeviceContext();

	HDC  BeginGdiDraw(const ui::UiRect& drawRc);
	void EndGdiDraw(HDC hDC, const ui::UiRect& drawRc, DWORD fixAlphaColor);

	ID2D1RenderTarget*  BeginD2DDraw();
	void EndD2DDraw(const ui::UiRect& drawRc);
private:
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> CreateD2DContext();
	Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> CreateDCRender(ID2D1Factory *pD2DFactory);

	void DrawImageFunction(ID2D1Bitmap *pImage, const UiRect& destRt, const UiRect& srcRt, bool bTransparent, bool bAlphaChannel, FLOAT opacity);
public:
	static ID2D1StrokeStyle* CreateStrokeFromPen(const ui::IPen* pen);
	static D2D1_CAP_STYLE Translate2D2DPenCapStyle(int style);
	static D2D1_LINE_JOIN Translate2D2DPenLineJoin(int style);
	static D2D1_DASH_STYLE Translate2D2DPenDashStyle(int style);
	static DWRITE_FONT_WEIGHT Translate2D2DTextWeight(tagTFontInfo* pFontInf);
	static DWRITE_FONT_STYLE Translate2D2DTextStyle(tagTFontInfo* pFontInf);
private:
	static void CreateDWriteTextResource(IDWriteFactory1 **ppWriteFactory);
	tagTFontInfo* _GetAndGenD2DTextResource(const std::wstring& strFontId);
	FLOAT _GetFontHeight(tagTFontInfo* pFontInf);
	Microsoft::WRL::ComPtr<IDWriteTextLayout> _BuildTextLayout(const std::wstring &strText, const D2D1_RECT_F& rc, tagTFontInfo* pFontInf, UINT uStyle, bool bLineLimit, D2D1_DRAW_TEXT_OPTIONS *pTextOptions);
	void _ReleaseHtmlTextEntryList(_HtmlTextEntry *pEntry);
	size_t _BuildHtmlTextSpecifier(_HtmlTextEntry* root, IDWriteTextLayout *pTextLayout, ID2D1Brush *pDefautBrush);
	Microsoft::WRL::ComPtr<ID2D1Bitmap> compatibleBitmap(RenderContext_D3D *tarRenderContext);
	static Microsoft::WRL::ComPtr<ID2D1Bitmap> CompatibleBitmapFromHBITMAP(HDC hDC, HBITMAP hBitmap, ID2D1RenderTarget *pRenderTar, D2D1_ALPHA_MODE alphaModel, bool flip=true, bool needDrawBk=false);
	void saveRepaint(const UiRect& destRt);
private:
	//shared d2d resource
	Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> 	m_d2dTar;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext>		m_d2dBitmapTar;
	Microsoft::WRL::ComPtr<ID2D1GdiInteropRenderTarget> m_wndGdiTar;
	Microsoft::WRL::ComPtr<CharacterFormatter>      m_characterFormatter;
	D2D1_SIZE_F									    m_dpi;
	//D2D transform (DIPS PIXEL)
	D2D1::Matrix3x2F								m_dpiTransform;
	D2D1::Matrix3x2F								m_renderTransform;
	D2D1::Matrix3x2F								m_worldToPixel;
	D2D1::Matrix3x2F								m_pixelToWorld;

	//use for transform
	D2D1::Matrix3x2F								m_preTransform;

	//Device - Independent 
	static Microsoft::WRL::ComPtr<IDWriteFactory1>	m_d2dWriteFactory;
	
	//wic
	static Microsoft::WRL::ComPtr<IWICImagingFactory>      m_widImagingFactory;

	bool m_gpu{false};
	HWND			m_wnd;       
	HDC				m_hDC;
	int				m_saveDC;
	bool			m_bTransparent;
	bool			m_bFlipDraw{ false };
	D2DClip			m_clip;
	bool            m_isCopy{ false };
	bool			m_beginDraw;
	std::wstring    m_resourceRoot;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> m_curBitmap;		//当前paint的bitmap
	Microsoft::WRL::ComPtr<ID2D1Image> m_paintBitmap;		//用于记录上一次paint区域的bitmap

	HBITMAP		m_hOldGdiBitmap;
	GdiBitmap	m_gdiBitmap;

	//用于兼容上层Gdi与D2D的互操作
	HDC				m_compatGdiDC;
	Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> m_compatD2DBitmap;  
	GdiBitmap   m_compatGdiBitmap;

	UiRect      m_repaintRC;
};

} // namespace ui

#endif // UI_CORE_RENDER_H_
