#include "StdAfx.h"
#include "Image.h"
#include <shlwapi.h>
#include "DirectXHelp.h"
#include <wrl/client.h>

namespace ui 
{
	std::wstring GetResourceFullPath(const std::wstring &sImageName, const std::wstring &root)
	{
		std::wstring imageFullPath = sImageName;
		bool is_relative = false;
		if (::PathIsRelative(sImageName.c_str()))
		{
			imageFullPath = GlobalManager::GetResourcePath() + root + sImageName;
			is_relative = true;
		}
		else
		{
			std::wstring module_path = GlobalManager::GetCurrentPath();
			int nfind = sImageName.find(module_path);
			if (nfind != std::wstring::npos)
			{
				is_relative = true;
			}
		}

		imageFullPath = StringHelper::ReparsePath(imageFullPath);

		int dpi = DpiManager::GetInstance()->GetScale();
		if (is_relative && dpi != 100)
		{
			std::wstring tempImageFullPath = imageFullPath;
			std::wstring scale_tail = L"@" + std::to_wstring(dpi);
			int nFind = tempImageFullPath.rfind(L".");
			if (nFind != -1)
			{
				tempImageFullPath.insert(nFind, scale_tail);
				if (::PathFileExists(tempImageFullPath.c_str()))
				{
					imageFullPath = tempImageFullPath;
				}
				else
				{
					//这边到时要断言  本地 125和150的图片没有
					ASSERT(0);
				}
			}
		}
		return imageFullPath;
	}

	ImageInfo::ImageInfo(Gdiplus::Bitmap* pGdiplusBitmap, int iFrameCount) :
	m_bAlphaChannel(false),
	m_bCached(false),
	m_propertyItem(),
	m_vecBitmap()
{
	gdiplusBitmap_.reset(pGdiplusBitmap);
	iFrameCount_ = iFrameCount;
	m_oldBitmap = nullptr;
}

ImageInfo::~ImageInfo()
{
	for (auto it = m_vecBitmap.begin(); it != m_vecBitmap.end(); it++) {
		::DeleteObject(*it);
	}
	if (m_oldBitmap)
		::DeleteObject(m_oldBitmap);
}

void ImageInfo::SetPropertyItem(Gdiplus::PropertyItem* pPropertyItem)
{
	m_propertyItem.reset(pPropertyItem);
}

void ImageInfo::PushBackHBitmap(HBITMAP hBitmap)
{
	m_vecBitmap.push_back(hBitmap);
}

HBITMAP ImageInfo::GetHBitmap(int nIndex)
{
	HBITMAP hBitmap = NULL;
	if (!IsGif())
	{
		hBitmap = m_oldBitmap;
	}
	else if (gdiplusBitmap_)
	{
		Gdiplus::Status status = gdiplusBitmap_->SelectActiveFrame(&Gdiplus::FrameDimensionTime, nIndex);
		if(status == Gdiplus::Ok){
			status = gdiplusBitmap_->GetHBITMAP(Gdiplus::Color(), &hBitmap);
		}
		if (m_oldBitmap)
			::DeleteObject(m_oldBitmap);
		m_oldBitmap = hBitmap;
	}
	return hBitmap;
}



int ImageInfo::GetFrameCount()
{
	return iFrameCount_;
}

bool ImageInfo::IsGif()
{
	return iFrameCount_ > 1;
}

int ImageInfo::GetInterval(int nIndex)
{
	if (m_propertyItem == nullptr)
		return 0;

	if (nIndex >= iFrameCount_)
		return 0;

	int interval = ((long*)(m_propertyItem->value))[nIndex] * 10;
	if (interval < 30) {
		interval = 100;
	}
	else if (interval < 50)	{
		interval = 50;
	}
	return interval;
}

Gdiplus::Bitmap * ImageInfo::GetBitmap()
{
	return gdiplusBitmap_.get();
}

std::unique_ptr<ImageInfo> ImageInfo::LoadImage(const std::wstring& strImageFullPath)
{
	Gdiplus::Bitmap* gdiplusBitmap = Gdiplus::Bitmap::FromFile(strImageFullPath.c_str());
	return LoadImageByBitmap(gdiplusBitmap, strImageFullPath);
}

std::unique_ptr<ImageInfo> ImageInfo::LoadImage(HGLOBAL hGlobal, const std::wstring& imageFullPath)
{
	if (hGlobal == NULL)
	{
		return nullptr;
	}
	IStream* stream = NULL;
	GlobalLock(hGlobal);
	CreateStreamOnHGlobal(hGlobal, true, &stream);
	if (stream == NULL)
	{
		GlobalUnlock(hGlobal);
		return nullptr;
	}
	
	Gdiplus::Bitmap* gdiplusBitmap = Gdiplus::Bitmap::FromStream(stream);
	stream->Release();
	GlobalUnlock(hGlobal);
	return LoadImageByBitmap(gdiplusBitmap, imageFullPath);
}

std::unique_ptr<ImageInfo> ImageInfo::LoadImageByBitmap(Gdiplus::Bitmap*& pGdiplusBitmap, const std::wstring& strImageFullPath)
{
	Gdiplus::Status status;
	status = pGdiplusBitmap->GetLastStatus();
	ASSERT(status == Gdiplus::Ok);
	if (status != Gdiplus::Ok) {
		return nullptr;
	}

	UINT nCount = pGdiplusBitmap->GetFrameDimensionsCount();
	std::unique_ptr<GUID[]> pDimensionIDs(new GUID[nCount]);
	pGdiplusBitmap->GetFrameDimensionsList(pDimensionIDs.get(), nCount);
	int iFrameCount = pGdiplusBitmap->GetFrameCount(&pDimensionIDs.get()[0]);

	std::unique_ptr<ImageInfo> imageInfo(new ImageInfo(pGdiplusBitmap, iFrameCount));
	if (iFrameCount > 1) {
		int iSize = pGdiplusBitmap->GetPropertyItemSize(PropertyTagFrameDelay);
		Gdiplus::PropertyItem* pPropertyItem = (Gdiplus::PropertyItem*)malloc(iSize);
		status = pGdiplusBitmap->GetPropertyItem(PropertyTagFrameDelay, iSize, pPropertyItem);
		ASSERT(status == Gdiplus::Ok);
		if (status != Gdiplus::Ok) {
			return nullptr;
		}
		imageInfo->SetPropertyItem(pPropertyItem);
	}

	//保持单帧缓存
	HBITMAP hBitmap;
	status = pGdiplusBitmap->GetHBITMAP(Gdiplus::Color(), &hBitmap);
	ASSERT(status == Gdiplus::Ok);
	if (status != Gdiplus::Ok) {
		return nullptr;
	}
	imageInfo->m_oldBitmap = hBitmap;

	imageInfo->nX = pGdiplusBitmap->GetWidth();
	imageInfo->nY = pGdiplusBitmap->GetHeight();
	imageInfo->sImageFullPath = strImageFullPath;
	Gdiplus::PixelFormat format = pGdiplusBitmap->GetPixelFormat();
	imageInfo->SetAlpha((format & PixelFormatAlpha) != 0);
	imageInfo->SetPixelFormat(format);

	if ((format & PixelFormatIndexed) != 0) {
		int nPalSize = pGdiplusBitmap->GetPaletteSize();
		if (nPalSize > 0) {
			Gdiplus::ColorPalette *palette = (Gdiplus::ColorPalette*)malloc(nPalSize);;
			status = pGdiplusBitmap->GetPalette(palette, nPalSize);
			if (status == Gdiplus::Ok) {
				imageInfo->SetAlpha((palette->Flags & Gdiplus::PaletteFlagsHasAlpha) != 0);
			}
			free(palette);
		}
	}

	if (format == PixelFormat32bppARGB) {
		//for (int nFrameIndex = 0; nFrameIndex < iFrameCount; nFrameIndex++) {
			HBITMAP hFirstBitmap = imageInfo->GetHBitmap(0);
			if (nullptr != hFirstBitmap)
			{
				BITMAP bm;
				::GetObject(hFirstBitmap, sizeof(bm), &bm);
				LPBYTE imageBits = (LPBYTE)bm.bmBits;
				for (int i = 0; i < bm.bmHeight; ++i) {
					for (int j = 0; j < bm.bmWidthBytes; j += 4) {
						int x = i * bm.bmWidthBytes + j;
						if (imageBits[x + 3] != 255) {
							imageInfo->SetAlpha(true);
							return imageInfo;
						}
					}
				}
			}
		//}

		imageInfo->SetAlpha(false);
		return imageInfo;
	}

	return imageInfo;
}

ImageAttribute::ImageAttribute()
{
	Init();
}

void ImageAttribute::Init()
{
	simageString.clear();
	sImageName.clear();
	bFade = 0xFF;
	bTiledX = false;
	bTiledY = false;
	rcDest.left = rcDest.top = rcDest.right = rcDest.bottom = DUI_NOSET_VALUE;
	rcSource.left = rcSource.top = rcSource.right = rcSource.bottom = DUI_NOSET_VALUE;
	rcCorner.left = rcCorner.top = rcCorner.right = rcCorner.bottom = 0;
	nPlayCount = -1;
}

void ImageAttribute::SetImageString(const std::wstring& strImageString)
{
	Init();
	simageString = strImageString;
	sImageName = strImageString;
	ModifyAttribute(*this, strImageString);
}

void ImageAttribute::ModifyAttribute(ImageAttribute& imageAttribute, const std::wstring& strImageString)
{
	std::wstring sItem;
	std::wstring sValue;
	LPTSTR pstr = NULL;
	bool bScaleDest = true;

	LPCTSTR pStrImage = strImageString.c_str();
	while (*pStrImage != _T('\0')) {
		sItem.clear();
		sValue.clear();
		while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
		while (*pStrImage != _T('\0') && *pStrImage != _T('=') && *pStrImage > _T(' ')) {
			LPTSTR pstrTemp = ::CharNext(pStrImage);
			while (pStrImage < pstrTemp) {
				sItem += *pStrImage++;
			}
		}
		while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
		if (*pStrImage++ != _T('=')) break;
		while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
		if (*pStrImage++ != _T('\'')) break;
		while (*pStrImage != _T('\0') && *pStrImage != _T('\'')) {
			LPTSTR pstrTemp = ::CharNext(pStrImage);
			while (pStrImage < pstrTemp) {
				sValue += *pStrImage++;
			}
		}
		if (*pStrImage++ != _T('\'')) break;
		if (!sValue.empty()) {
			if (sItem == _T("file") || sItem == _T("res")) 
			{
#if 0
				int dpi =  DpiManager::GetInstance()->GetScale();
				if (dpi != 100)
				{
					std::wstring scale_tail = L"@" + std::to_wstring(dpi);
					int nFind = sValue.rfind(L".");
					if (nFind != -1)
					{
						sValue.insert(nFind, scale_tail);
					}
				}
#endif
				imageAttribute.sImageName = sValue;
			}
			else if (sItem == _T("destscale")) {
				bScaleDest = (_tcscmp(sValue.c_str(), _T("true")) == 0);
			}
			else if (sItem == _T("dest")) {
				imageAttribute.rcDest.left = _tcstol(sValue.c_str(), &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcDest.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
				imageAttribute.rcDest.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcDest.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);

				if (bScaleDest)
					DpiManager::GetInstance()->ScaleRect(imageAttribute.rcDest); 
			}
			else if (sItem == _T("source")) {
				imageAttribute.rcSource.left = _tcstol(sValue.c_str(), &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcSource.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
				imageAttribute.rcSource.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcSource.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);

				if (!bScaleDest)
				{
					return;
				}
				int source_width  = imageAttribute.rcSource.GetWidth();
				int source_height = imageAttribute.rcSource.GetHeight();

				int height_index = imageAttribute.rcSource.top / source_height;
				int width_index = imageAttribute.rcSource.left / source_width;

				int scale_source_width = DpiManager::GetInstance()->ScaleIntEx(source_width);
				int scale_source_height = DpiManager::GetInstance()->ScaleIntEx(source_height);

				if (height_index > 0 && width_index == 0)
				{
					DpiManager::GetInstance()->ScaleRect(imageAttribute.rcSource);
					imageAttribute.rcSource.top    = (height_index) *  scale_source_height;
					imageAttribute.rcSource.bottom = (height_index + 1) *  scale_source_height;
				}
				else if (height_index == 0 && width_index > 0)
				{
					DpiManager::GetInstance()->ScaleRect(imageAttribute.rcSource);
					imageAttribute.rcSource.left = (width_index)*  scale_source_width;
					imageAttribute.rcSource.right = (width_index + 1) *  scale_source_width;
				}
				else
				{
					DpiManager::GetInstance()->ScaleRect(imageAttribute.rcSource);
				}
			}
			else if (sItem == _T("corner")) {
				imageAttribute.rcCorner.left = _tcstol(sValue.c_str(), &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcCorner.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
				imageAttribute.rcCorner.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcCorner.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);

				if (bScaleDest)
					DpiManager::GetInstance()->ScaleRect(imageAttribute.rcCorner);
			}
			else if (sItem == _T("fade")) {
				imageAttribute.bFade = (BYTE)_tcstoul(sValue.c_str(), &pstr, 10);
			}
			else if (sItem == _T("xtiled")) {
				imageAttribute.bTiledX = (_tcscmp(sValue.c_str(), _T("true")) == 0);
			}
			else if (sItem == _T("ytiled")) {
				imageAttribute.bTiledY = (_tcscmp(sValue.c_str(), _T("true")) == 0);
			}
			else if (sItem == _T("playcount"))
			{
				imageAttribute.nPlayCount = _tcstol(sValue.c_str(), &pstr, 10);  ASSERT(pstr);
			}
		}
		if (*pStrImage++ != _T(' ')) break;
	}
}


Image::Image() :
	imageAttribute(),
	imageCache(),
	m_nCurrentFrame(0),
	m_bPlaying(false),
	m_nCycledCount(0)
{

}

void Image::SetImageString(const std::wstring& strImageString)
{

	ClearCache();
	imageAttribute.SetImageString(strImageString);
}

void Image::SetPlayCount(int PlayCount)
{
	imageAttribute.nPlayCount = PlayCount;
}

void Image::ClearCache()
{
	m_nCurrentFrame = 0;
	m_bPlaying = false;
	imageCache.reset();
	m_nCycledCount = 0;
}

bool Image::IncrementCurrentFrame()
{
	if (!imageCache) {
		return false;
	}
	m_nCurrentFrame++;
	if (m_nCurrentFrame == imageCache->GetFrameCount()) {
		m_nCurrentFrame = 0;
		m_nCycledCount += 1;
	}
	return true;
}

void Image::SetCurrentFrame(int nCurrentFrame)
{
	m_nCurrentFrame = nCurrentFrame;
}

HBITMAP Image::GetCurrentHBitmap()
{
	if (!imageCache) {
		return NULL;
	}
	return imageCache->GetHBitmap(m_nCurrentFrame);
}

int Image::GetCurrentInterval()
{
	if (!imageCache) {
		return 0;
	}
	return imageCache->GetInterval(m_nCurrentFrame);
}
int Image::GetCurrentFrameIndex()
{
	return m_nCurrentFrame;
}
int Image::GetCycledCount()
{
	return m_nCycledCount;
}
void Image::ClearCycledCount()
{
	m_nCycledCount = 0;
}
bool Image::ContinuePlay()
{
	if (imageAttribute.nPlayCount < 0)
		return true;
	else if (imageAttribute.nPlayCount == 0)
		return m_bPlaying;
	else
		return m_nCycledCount < imageAttribute.nPlayCount;
}
StateImage::StateImage() :
	m_pControl(nullptr),
	m_stateImageMap()
{

}

bool StateImage::HasHotImage()
{
	return !m_stateImageMap[kControlStateHot].imageAttribute.simageString.empty();
}

bool StateImage::PaintStatusImage(IRenderContext* pRender, ControlStateType stateType, const std::wstring& sImageModify /*= L""*/)
{
	if (m_pControl) {
		bool bFadeHot = m_pControl->GetAnimationManager().GetAnimationPlayer(kAnimationHot) != nullptr;
		int nHotAlpha = m_pControl->GetHotAlpha();
		if (bFadeHot) {
			if (stateType == kControlStateNormal || stateType == kControlStateHot) {
				std::wstring strNormalImagePath = m_stateImageMap[kControlStateNormal].imageAttribute.sImageName;
				std::wstring strHotImagePath = m_stateImageMap[kControlStateHot].imageAttribute.sImageName;

				if (strNormalImagePath.empty() || strHotImagePath.empty()
					|| strNormalImagePath != strHotImagePath
					|| !m_stateImageMap[kControlStateNormal].imageAttribute.rcSource.Equal(m_stateImageMap[kControlStateHot].imageAttribute.rcSource)) {

					m_pControl->DrawImage(pRender, m_stateImageMap[kControlStateNormal], sImageModify);
					int nHotFade = m_stateImageMap[kControlStateHot].imageAttribute.bFade;
					nHotFade = int(nHotFade * (double)nHotAlpha / 255);
					return m_pControl->DrawImage(pRender, m_stateImageMap[kControlStateHot], sImageModify, nHotFade);
				}
				else {
					int nNormalFade = m_stateImageMap[kControlStateNormal].imageAttribute.bFade;
					int nHotFade = m_stateImageMap[kControlStateHot].imageAttribute.bFade;
					int nBlendFade = int((1 - (double)nHotAlpha / 255) * nNormalFade + (double)nHotAlpha / 255 * nHotFade);
					return m_pControl->DrawImage(pRender, m_stateImageMap[kControlStateHot], sImageModify, nBlendFade);
				}
			}
		}
	}

	if (stateType == kControlStatePushed && m_stateImageMap[kControlStatePushed].imageAttribute.simageString.empty()) {
		stateType = kControlStateHot;
		m_stateImageMap[kControlStateHot].imageAttribute.bFade = 255;
	}
	if (stateType == kControlStateHot && m_stateImageMap[kControlStateHot].imageAttribute.simageString.empty()) {
		stateType = kControlStateNormal;
	}
	if (stateType == kControlStateDisabled && m_stateImageMap[kControlStateDisabled].imageAttribute.simageString.empty()) {
		stateType = kControlStateNormal;
	}

	return m_pControl->DrawImage(pRender, m_stateImageMap[stateType], sImageModify);
}

Image* StateImage::GetEstimateImage()
{
	Image* pEstimateImage = nullptr;
	if (!m_stateImageMap[kControlStateNormal].imageAttribute.sImageName.empty()){
		pEstimateImage = &m_stateImageMap[kControlStateNormal];
	}
	else if (!m_stateImageMap[kControlStateHot].imageAttribute.sImageName.empty()) {
		pEstimateImage = &m_stateImageMap[kControlStateHot];
	}
	else if (!m_stateImageMap[kControlStatePushed].imageAttribute.sImageName.empty()) {
		pEstimateImage = &m_stateImageMap[kControlStatePushed];
	}
	else if (!m_stateImageMap[kControlStateDisabled].imageAttribute.sImageName.empty()) {
		pEstimateImage = &m_stateImageMap[kControlStateDisabled];
	}

	return pEstimateImage;
}

void StateImage::ClearCache()
{
	auto it = m_stateImageMap.find(kControlStateNormal);
	if (it != m_stateImageMap.end())
	{
		it->second.ClearCache();
	}
	it = m_stateImageMap.find(kControlStateHot);
	if (it != m_stateImageMap.end())
	{
		it->second.ClearCache();
	}
	it = m_stateImageMap.find(kControlStatePushed);
	if (it != m_stateImageMap.end())
	{
		it->second.ClearCache();
	}
	it = m_stateImageMap.find(kControlStateDisabled);
	if (it != m_stateImageMap.end())
	{
		it->second.ClearCache();
	}
}


void StateImageMap::SetControl(Control* control)
{
	m_stateImageMap[kStateImageBk].SetControl(control);
	m_stateImageMap[kStateImageFore].SetControl(control);
	m_stateImageMap[kStateImageSelectedBk].SetControl(control);
	m_stateImageMap[kStateImageSelectedFore].SetControl(control);
}

void StateImageMap::SetImage(StateImageType stateImageType, ControlStateType stateType, const std::wstring& strImagePath)
{
	m_stateImageMap[stateImageType][stateType].SetImageString(strImagePath);
}

std::wstring StateImageMap::GetImagePath(StateImageType stateImageType, ControlStateType stateType)
{
	return m_stateImageMap[stateImageType][stateType].imageAttribute.simageString;
}

bool StateImageMap::HasHotImage()
{
	for (auto& it : m_stateImageMap) {
		if (it.second.HasHotImage()) {
			return true;
		}
	}
	return false;
}

bool StateImageMap::PaintStatusImage(IRenderContext* pRender, StateImageType stateImageType, ControlStateType stateType, const std::wstring& sImageModify /*= L""*/)
{
	auto it = m_stateImageMap.find(stateImageType);
	if (it != m_stateImageMap.end()) {
		return it->second.PaintStatusImage(pRender, stateType, sImageModify);
	}
	return false;
}

Image* StateImageMap::GetEstimateImage(StateImageType stateImageType)
{
	auto it = m_stateImageMap.find(stateImageType);
	if (it != m_stateImageMap.end()) {
		return it->second.GetEstimateImage();
	}
	return nullptr;
}

void StateImageMap::ClearCache()
{
	m_stateImageMap[kStateImageBk].ClearCache();
	m_stateImageMap[kStateImageFore].ClearCache();
	m_stateImageMap[kStateImageSelectedBk].ClearCache();
	m_stateImageMap[kStateImageSelectedFore].ClearCache();
}


StateColorMap::StateColorMap() :
	m_pControl(nullptr),
	m_stateColorMap()
{
}

void StateColorMap::SetControl(Control* control)
{
	m_pControl = control;
}

bool StateColorMap::HasHotColor()
{
	return !m_stateColorMap[kControlStateHot].empty();
}

void StateColorMap::PaintStatusColor(IRenderContext* pRender, UiRect rcPaint, ControlStateType stateType)
{
	if (!pRender)
	{
		return;
	}

	if (m_pControl) {
		bool bFadeHot = m_pControl->GetAnimationManager().GetAnimationPlayer(kAnimationHot) != nullptr;
		int nHotAlpha = m_pControl->GetHotAlpha();
		if (bFadeHot) {
			if ((stateType == kControlStateNormal || stateType == kControlStateHot)
				&& !m_stateColorMap[kControlStateHot].empty()) {

				pRender->DrawColor(rcPaint, m_stateColorMap[kControlStateNormal]);
				if (nHotAlpha > 0) {
					pRender->DrawColor(rcPaint, m_stateColorMap[kControlStateHot], nHotAlpha);
				}
				return;
			}
		}
	}

	if (stateType == kControlStatePushed && m_stateColorMap[kControlStatePushed].empty()) {
		stateType = kControlStateHot;
	}
	if (stateType == kControlStateHot && m_stateColorMap[kControlStateHot].empty()) {
		stateType = kControlStateNormal;
	}
	if (stateType == kControlStateDisabled && m_stateColorMap[kControlStateDisabled].empty()) {
		stateType = kControlStateNormal;
	}

	pRender->DrawColor(rcPaint, m_stateColorMap[stateType]);
}

//////////////////////////////////////////////////////////////////////////
DrawRound::DrawRound()
{

}

void DrawRound::SetRect(float start_x, float start_y, float width, float height)
{
	start_x_	= start_x;
	start_y_	= start_y;
	width_		= width;
	height_		= height;
}

void DrawRound::SetLine(Gdiplus::Color line_color, float line_width, Gdiplus::DashStyle line_style)
{
	line_color_ = line_color;
	line_width_ = line_width;
	this->line_stye_ = line_style;
}

void DrawRound::SetArcSize(float arc_size)
{
	arc_size_ = arc_size;
}

void DrawRound::SetOnePix(int one_pix)
{
	one_pix_ = one_pix;
}

void DrawRound::SetStyle(int style/* = 0*/)
{
	style_ = style;
}

void DrawRound::SetFillType(int fill_type)
{
	this->fill_type_ = fill_type;
}

void DrawRound::DrawRoundRectange(HDC hdc, Gdiplus::Bitmap *bitmap)
{
	// 小矩形的半宽（hew）和半高（heh）
	float hew = width_ / arc_size_ / 2;
	float heh = height_ / arc_size_ / 2;
	// 圆角修正
	if (fabs(hew - heh) > 10)
	{
		hew = heh = hew > heh ? heh : hew;
	}

	// 创建GDI+对象
	Gdiplus::Graphics  graphics(hdc);

	//设置画图时的滤波模式为消除锯齿现象
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	// 保存绘图路径
	Gdiplus::GraphicsPath roundRectPath;
	if (0 == style_)
	{
		roundRectPath.AddLine(start_x_ + hew, start_y_, start_x_ + width_ - hew, start_y_);  // 顶部横线
		roundRectPath.AddArc(start_x_ + width_ - 2 * hew, start_y_, 2 * hew, 2 * heh, 270, 90); // 右上圆角
		roundRectPath.AddLine(start_x_ + width_, start_y_ + heh, start_x_ + width_, start_y_ + height_ - heh);  // 右侧竖线
		roundRectPath.AddArc(start_x_ + width_ - 2 * hew, start_y_ + height_ - 2 * heh, 2 * hew, 2 * heh, 0, 90); // 右下圆角
		roundRectPath.AddLine(start_x_ + width_ - hew, start_y_ + height_, start_x_ + hew, start_y_ + height_);  // 底部横线
		roundRectPath.AddArc(start_x_, start_y_ + height_ - 2 * heh, 2 * hew, 2 * heh, 90, 90); // 左下圆角
		roundRectPath.AddLine(start_x_, start_y_ + height_ - heh, start_x_, start_y_ + heh);  // 左侧竖线
		roundRectPath.AddArc(start_x_, start_y_, 2 * hew, 2 * heh, 180, 90); // 左上圆角
	}
	else if (1 == style_)
	{
		roundRectPath.AddLine(start_x_, start_y_, start_x_ + width_ , start_y_);  // 顶部横线
		roundRectPath.AddLine(start_x_ + width_, start_y_, start_x_ + width_, start_y_ + height_ - heh);  // 右侧竖线
		roundRectPath.AddArc(start_x_ + width_ - 2 * hew, start_y_ + height_ - 2 * heh, 2 * hew, 2 * heh, 0, 90); // 右下圆角
		roundRectPath.AddLine(start_x_ + width_ - hew, start_y_ + height_, start_x_ + hew, start_y_ + height_);  // 底部横线
		roundRectPath.AddArc(start_x_, start_y_ + height_ - 2 * heh, 2 * hew, 2 * heh, 90, 90); // 左下圆角
		roundRectPath.AddLine(start_x_, start_y_ + height_ - heh, start_x_, start_y_);  // 左侧竖线
	}

	Gdiplus::Pen pen(line_color_, line_width_);
	graphics.DrawPath(&pen, &roundRectPath);

	// 创建画刷
	Gdiplus::Bitmap thumb_bitmap(width_ + one_pix_, height_ + one_pix_, PixelFormat32bppARGB);
	Gdiplus::Graphics thumb_graphic(&thumb_bitmap);
	thumb_graphic.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	thumb_graphic.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
	thumb_graphic.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	if (fill_type_ == 1)
	{
		Gdiplus::REAL width_src = (Gdiplus::REAL)bitmap->GetWidth();
		Gdiplus::REAL height_src = (Gdiplus::REAL)bitmap->GetHeight();
		int fixed_w = width_;
		int fixed_h = height_;

		Gdiplus::REAL left = 0;
		Gdiplus::REAL top = 0;
		if (width_src * fixed_h /fixed_w <height_src)
		{
			height_src = width_src * fixed_h / fixed_w;
			left = ((Gdiplus::REAL)bitmap->GetWidth() - width_src)/2;
		}
		else if (height_src * fixed_w / fixed_h < width_src)
		{
			width_src = height_src * fixed_w / fixed_h;
			top = ((Gdiplus::REAL)bitmap->GetHeight() - height_src) / 2;
		}

		Gdiplus::Rect destRc(0, 0, (int)width_ + one_pix_, (int)height_ + one_pix_);
		thumb_graphic.DrawImage(bitmap, destRc,(int)left, (int)top, (int)width_src, (int)height_src, Gdiplus::UnitPixel);
	}
	else
	{
		thumb_graphic.DrawImage(bitmap, 0, 0, (int)width_ + one_pix_, (int)height_ + one_pix_);
	}

	Gdiplus::TextureBrush brush(&thumb_bitmap);
	brush.TranslateTransform(start_x_, start_y_);

	graphics.FillPath(&brush, &roundRectPath);
}


void DrawRound::DrawRoundByBkColor(HDC hdc,  Gdiplus::Color fillColor)
{
	// 小矩形的半宽（hew）和半高（heh）
	float hew = width_ / arc_size_ / 2;
	float heh = height_ / arc_size_ / 2;
	// 圆角修正
	if (fabs(hew - heh) > 10)
	{
		hew = heh = hew > heh ? heh : hew;
	}

	// 创建GDI+对象
	Gdiplus::Graphics  graphics(hdc);

	//设置画图时的滤波模式为消除锯齿现象
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	// 保存绘图路径
	Gdiplus::GraphicsPath roundRectPath;
	if (0 == style_)
	{
		roundRectPath.AddLine(start_x_ + hew, start_y_, start_x_ + width_ - hew, start_y_);  // 顶部横线
		roundRectPath.AddArc(start_x_ + width_ - 2 * hew, start_y_, 2 * hew, 2 * heh, 270, 90); // 右上圆角
		roundRectPath.AddLine(start_x_ + width_, start_y_ + heh, start_x_ + width_, start_y_ + height_ - heh);  // 右侧竖线
		roundRectPath.AddArc(start_x_ + width_ - 2 * hew, start_y_ + height_ - 2 * heh, 2 * hew, 2 * heh, 0, 90); // 右下圆角
		roundRectPath.AddLine(start_x_ + width_ - hew, start_y_ + height_, start_x_ + hew, start_y_ + height_);  // 底部横线
		roundRectPath.AddArc(start_x_, start_y_ + height_ - 2 * heh, 2 * hew, 2 * heh, 90, 90); // 左下圆角
		roundRectPath.AddLine(start_x_, start_y_ + height_ - heh, start_x_, start_y_ + heh);  // 左侧竖线
		roundRectPath.AddArc(start_x_, start_y_, 2 * hew, 2 * heh, 180, 90); // 左上圆角
	}
	else if (1 == style_)
	{
		roundRectPath.AddLine(start_x_, start_y_, start_x_ + width_, start_y_);  // 顶部横线
		roundRectPath.AddLine(start_x_ + width_, start_y_, start_x_ + width_, start_y_ + height_ - heh);  // 右侧竖线
		roundRectPath.AddArc(start_x_ + width_ - 2 * hew, start_y_ + height_ - 2 * heh, 2 * hew, 2 * heh, 0, 90); // 右下圆角
		roundRectPath.AddLine(start_x_ + width_ - hew, start_y_ + height_, start_x_ + hew, start_y_ + height_);  // 底部横线
		roundRectPath.AddArc(start_x_, start_y_ + height_ - 2 * heh, 2 * hew, 2 * heh, 90, 90); // 左下圆角
		roundRectPath.AddLine(start_x_, start_y_ + height_ - heh, start_x_, start_y_);  // 左侧竖线
	}
	else if (2 == style_)		//顶部两个圆角，底部无
	{
		roundRectPath.AddLine(start_x_ + hew, start_y_, start_x_ + width_ - hew, start_y_);  // 顶部横线
		roundRectPath.AddArc(start_x_ + width_ - 2 * hew, start_y_, 2 * hew, 2 * heh, 270, 90); // 右上圆角
		roundRectPath.AddLine(start_x_ + width_, start_y_ , start_x_ + width_, start_y_ + height_ );  // 右侧竖线
		roundRectPath.AddLine(start_x_ + width_ - hew, start_y_ + height_, start_x_ + hew, start_y_ + height_);  // 底部横线
		roundRectPath.AddLine(start_x_, start_y_ + height_, start_x_, start_y_);  // 左侧竖线
		roundRectPath.AddArc(start_x_, start_y_, 2 * hew, 2 * heh, 180, 90); // 左上圆角
	}

	Gdiplus::Pen pen(line_color_, line_width_);
	pen.SetDashStyle(line_stye_);

	graphics.DrawPath(&pen, &roundRectPath);

	// 创建画刷
	Gdiplus::SolidBrush brush(fillColor);
	graphics.FillPath(&brush, &roundRectPath);
}



/************************************************************************/
/* DirectX                                                                     */
/************************************************************************/
DrawRoundDX::DrawRoundDX():line_color_(D2D1::ColorF::White),style_(D2D1_DASH_STYLE_DASH)
{

}

void DrawRoundDX::SetRect(float start_x, float start_y, float width, float height)
{
	start_x_ = start_x;
	start_y_ = start_y;
	width_ = width;
	height_ = height;
}

void DrawRoundDX::SetLine(D2D1::ColorF line_color, float line_width, D2D1_DASH_STYLE line_style)
{
	line_color_ = line_color;
	line_width_ = line_width;
	this->line_stye_ = line_style;
}

void DrawRoundDX::SetArcSize(float arc_size)
{
	arc_size_ = arc_size;
}

void DrawRoundDX::SetOnePix(int one_pix)
{
	one_pix_ = one_pix;
}

void DrawRoundDX::SetStyle(int style/* = 0*/)
{
	style_ = style;
}

void DrawRoundDX::SetFillType(int fill_type)
{
	this->fill_type_ = fill_type;
}

void DrawRoundDX::DrawRoundRectange(ID2D1RenderTarget *pRender, HBITMAP hBitmap)
{
	assert(hBitmap != NULL);
	if (hBitmap == NULL) return;

	BITMAP info;
	if (0 == ::GetObject(hBitmap, sizeof(info), &info))
	{
		assert(0);
		return;
	}
	LPBYTE pPixel = LPBYTE(info.bmBits);
	WORD bytesPerPixel = info.bmBitsPixel / 8;
	D2D1_ALPHA_MODE alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	FLOAT opacity = 1.0;

	D2D1_BITMAP_PROPERTIES bitmapProperties = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, alphaMode));
	Microsoft::WRL::ComPtr<ID2D1Bitmap> pImageBitmap;
	HRESULT hr = pRender->CreateBitmap(D2D1::SizeU(info.bmWidth, info.bmHeight), bitmapProperties, &pImageBitmap);
	assert(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		return;
	}

	FLOAT dpiX, dpiY;
	pRender->GetDpi(&dpiX, &dpiY);
	Microsoft::WRL::ComPtr<ID2D1Factory> pD2DFactory = nullptr;
	DX::GetD2D1Factory(&pD2DFactory);
	Microsoft::WRL::ComPtr<ID2D1BitmapBrush> pFillColorBrush;
	hr = pRender->CreateBitmapBrush(pImageBitmap.Get(), &pFillColorBrush);
	assert(SUCCEEDED(hr));
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pDrawColorBrush;
	hr = pRender->CreateSolidColorBrush(line_color_, &pDrawColorBrush);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		D2D1_RECT_F rect = D2D1::RectF(
			DX::ConvertPixelsToDips(start_x_,dpiX),
			DX::ConvertPixelsToDips(start_y_,dpiY), 
			DX::ConvertPixelsToDips(start_x_ + width_ + one_pix_, dpiX),
			DX::ConvertPixelsToDips(start_y_ + height_ + one_pix_, dpiY)
		);
		D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
			rect,
			DX::ConvertPixelsToDips(arc_size_, dpiX),
			DX::ConvertPixelsToDips(arc_size_, dpiY)
		);
		Microsoft::WRL::ComPtr<ID2D1StrokeStyle> pStrokeStyle;
		HRESULT hr = pD2DFactory->CreateStrokeStyle(
			D2D1::StrokeStyleProperties(
				D2D1_CAP_STYLE_FLAT,
				D2D1_CAP_STYLE_FLAT,
				D2D1_CAP_STYLE_FLAT,
				D2D1_LINE_JOIN_MITER,
				10.0f,
				line_stye_,
				0.0f),
			nullptr,
			0,
			&pStrokeStyle
		);
		assert(SUCCEEDED(hr));
		pRender->FillRoundedRectangle(roundedRect, pFillColorBrush.Get());
		pRender->DrawRoundedRectangle(roundedRect, pDrawColorBrush.Get(), DX::ConvertPixelsToDips(line_width_, dpiX), pStrokeStyle.Get());
	}
}


void DrawRoundDX::DrawRoundByBkColor(ID2D1RenderTarget *pRender, D2D1::ColorF fillColor)
{
	FLOAT dpiX, dpiY;
	pRender->GetDpi(&dpiX,&dpiY);
	Microsoft::WRL::ComPtr<ID2D1Factory> pD2DFactory = nullptr;
	DX::GetD2D1Factory(&pD2DFactory);
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pFillColorBrush;
	HRESULT hr = pRender->CreateSolidColorBrush(fillColor, &pFillColorBrush);
	assert(SUCCEEDED(hr));
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pDrawColorBrush;
	hr = pRender->CreateSolidColorBrush(line_color_, &pDrawColorBrush);
	assert(SUCCEEDED(hr));
	if (SUCCEEDED(hr))
	{
		D2D1_RECT_F rect = D2D1::RectF(
			DX::ConvertPixelsToDips(start_x_, dpiX),
			DX::ConvertPixelsToDips(start_y_, dpiY),
			DX::ConvertPixelsToDips(start_x_ + width_ + one_pix_, dpiX),
			DX::ConvertPixelsToDips(start_y_ + height_ + one_pix_, dpiY)
		);
		D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
			rect,
			DX::ConvertPixelsToDips(arc_size_, dpiX),
			DX::ConvertPixelsToDips(arc_size_, dpiY)
		);
		Microsoft::WRL::ComPtr<ID2D1StrokeStyle> pStrokeStyle;
		HRESULT hr = pD2DFactory->CreateStrokeStyle(
			D2D1::StrokeStyleProperties(
				D2D1_CAP_STYLE_FLAT,
				D2D1_CAP_STYLE_FLAT,
				D2D1_CAP_STYLE_FLAT,
				D2D1_LINE_JOIN_MITER,
				10.0f,
				line_stye_,
				0.0f),
			nullptr,
			0,
			&pStrokeStyle
		);
		assert(SUCCEEDED(hr));
		pRender->FillRoundedRectangle(roundedRect, pFillColorBrush.Get());
		pRender->DrawRoundedRectangle(roundedRect, pDrawColorBrush.Get(), DX::ConvertPixelsToDips(line_width_, dpiX), pStrokeStyle.Get());
	}
	
}

}