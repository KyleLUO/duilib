#ifndef UI_CORE_IMAGEDECODE_H_
#define UI_CORE_IMAGEDECODE_H_

#pragma once

#include <GdiPlus.h>
#include <d2d1.h>

namespace ui 
{
	extern std::wstring GetResourceFullPath(const std::wstring &path, const std::wstring &root);

class UILIB_API ImageInfo
{
public:
	ImageInfo(Gdiplus::Bitmap* pGdiplusBitmap = nullptr, int iFrameCount = 0);
	~ImageInfo();

	void SetAlpha(bool bAlphaChannel) {	m_bAlphaChannel = bAlphaChannel; }
	bool IsAlpha() { return m_bAlphaChannel; }
	bool IsCached()	{ return m_bCached; }
	void SetCached(bool bCached) { m_bCached = bCached; }
	void SetPixelFormat(Gdiplus::PixelFormat fmt) { m_pixelFormat = fmt; }
	void SetPropertyItem(Gdiplus::PropertyItem* pPropertyItem);

	void PushBackHBitmap(HBITMAP hBitmap);
	HBITMAP GetHBitmap(int nIndex);
	int GetFrameCount();
	bool IsGif();
	int GetInterval(int nIndex); //毫秒为单位 
	Gdiplus::Bitmap *GetBitmap();
	Gdiplus::PixelFormat GetPixelFormat(); //获取图像像素格式
	 
	static std::unique_ptr<ImageInfo> LoadImage(const std::wstring& strImageFullPath);
	static std::unique_ptr<ImageInfo> LoadImage(HGLOBAL hGlobal, const std::wstring& imageFullPath);

private:
	static std::unique_ptr<ImageInfo> LoadImageByBitmap(Gdiplus::Bitmap*& pGdiplusBitmap, const std::wstring& imageFullPath);
public:
	int nX;
	int nY;
	std::wstring sImageFullPath;

private:
	std::unique_ptr<Gdiplus::Bitmap> gdiplusBitmap_;
	bool m_bAlphaChannel;
	bool m_bCached;
	std::unique_ptr<Gdiplus::PropertyItem> m_propertyItem;
	Gdiplus::PixelFormat m_pixelFormat;
	std::vector<HBITMAP> m_vecBitmap;

	int iFrameCount_;
	HBITMAP m_oldBitmap;
};

struct UILIB_API ImageAttribute
{
public:
	ImageAttribute();

	void Init();
	void SetImageString(const std::wstring& strImageString);
	static void ModifyAttribute(ImageAttribute& imageAttribute, const std::wstring& strImageString);

	std::wstring simageString;
	std::wstring sImageName;
	UiRect rcDest;
	UiRect rcSource;
	UiRect rcCorner;
	BYTE bFade;
	bool bTiledX;
	bool bTiledY;
	int nPlayCount;//如果是GIF可以指定播放次数 -1 ：一直播放，缺省值。
};

class UILIB_API Image
{
public:
	Image();

	bool IsValid() { return (bool)imageCache; }
	bool IsPlaying() { return m_bPlaying; }
	void SetPlaying(bool bPlaying) { m_bPlaying = bPlaying; }

	void SetImageString(const std::wstring& strImageString);
	void SetPlayCount(int PlayCount);
	void ClearCache();

	bool IncrementCurrentFrame();
	void SetCurrentFrame(int nCurrentFrame);
	HBITMAP GetCurrentHBitmap();
	int GetCurrentInterval();
	int GetCurrentFrameIndex();
	int GetCycledCount();
	void ClearCycledCount();
	bool ContinuePlay();
	ImageAttribute imageAttribute;
	std::shared_ptr<ImageInfo> imageCache;

private:
	int m_nCurrentFrame;
	bool m_bPlaying;
	int m_nCycledCount;//播放次数
};

class UILIB_API StateImage
{
public:
	StateImage();

	void SetControl(Control* control) {	m_pControl = control; }
	Image& operator[](ControlStateType stateType) {	return m_stateImageMap[stateType]; }

	bool HasHotImage();
	bool PaintStatusImage(IRenderContext* pRender, ControlStateType stateType, const std::wstring& sImageModify = L"");
	Image* GetEstimateImage();
	void ClearCache();

private:
	Control* m_pControl;
	std::map<ControlStateType, Image> m_stateImageMap;
};

class UILIB_API StateImageMap
{
public:
	StateImageMap()	{ }

	void SetControl(Control* control);

	void SetImage(StateImageType stateImageType, ControlStateType stateType, const std::wstring& strImagePath);
	std::wstring GetImagePath(StateImageType stateImageType, ControlStateType stateType);

	bool HasHotImage();
	bool PaintStatusImage(IRenderContext* pRender, StateImageType stateImageType, ControlStateType stateType, const std::wstring& sImageModify = L"");
	Image* GetEstimateImage(StateImageType stateImageType);

	void ClearCache();

private:
	std::map<StateImageType, StateImage> m_stateImageMap;
};

class UILIB_API StateColorMap
{
public:
	StateColorMap();

	void SetControl(Control* control);
	std::wstring& operator[](ControlStateType stateType) { return m_stateColorMap[stateType]; }

	bool HasHotColor();
	void PaintStatusColor(IRenderContext* pRender, UiRect rcPaint, ControlStateType stateType);

private:
	Control* m_pControl;
	std::map<ControlStateType, std::wstring> m_stateColorMap;
};

class UILIB_API DrawRound
{
public:
	DrawRound();
	void SetRect(float start_x, float start_y, float width, float height);
	void SetLine(Gdiplus::Color line_color, float line_width, Gdiplus::DashStyle line_style);
	void SetArcSize(float arc_size);
	void SetOnePix(int one_pix);
	void SetStyle(int style = 0);
	void SetFillType(int fill_type);
	void DrawRoundRectange(HDC hdc,Gdiplus::Bitmap *bitmap);
	void DrawRoundByBkColor(HDC hdc,Gdiplus::Color fillColor);
private:
	float start_x_;
	float start_y_;
	float width_;
	float height_;
	Gdiplus::Color line_color_;
	float line_width_;
	Gdiplus::DashStyle line_stye_;
	float arc_size_;
	int   one_pix_;
	int   style_ = 0;
	int fill_type_ = 0;
};

class UILIB_API DrawRoundDX
{
public:
	DrawRoundDX();
	void SetRect(float start_x, float start_y, float width, float height);
	void SetLine(D2D1::ColorF line_color, float line_width, D2D1_DASH_STYLE line_style);
	void SetArcSize(float arc_size);
	void SetOnePix(int one_pix);
	void SetStyle(int style = 0);
	void SetFillType(int fill_type);
	void DrawRoundRectange(ID2D1RenderTarget *pRender, HBITMAP hBitmap);
	void DrawRoundByBkColor(ID2D1RenderTarget *pRender, D2D1::ColorF fillColor);
private:
	float start_x_;
	float start_y_;
	float width_;
	float height_;
	D2D1::ColorF line_color_;
	float line_width_;
	D2D1_DASH_STYLE line_stye_;
	float arc_size_;
	int   one_pix_;
	int   style_ = 0;
	int fill_type_ = 0;
};
} // namespace ui

#endif // UI_CORE_IMAGEDECODE_H_
