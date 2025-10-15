#include "StdAfx.h"
#include "shlwapi.h"
#include "../Animation/AnimationPlayer.h"

namespace ui 
{
	const int Control::m_nVirtualEventGifStop = 1;
Control::Control() :
	OnXmlEvent(),
	OnEvent(),
	m_pUserDataBase(),
	m_bMenuUsed(false),
	m_bEnabled(true),
	m_bMouseEnabled(true),
	m_bKeyboardEnabled(true),
	m_bFocused(false),
	m_bMouseFocused(false),
	m_bSetPos(false),
	m_bNoFocus(false),
	m_bClip(true),
	m_bGifPlay(true),
	m_bGifPlayEx(true),
	m_bReceivePointerMsg(true),
	m_bNeedButtonUpWhenKillFocus(false),
	m_szEstimateSize(),
	m_renderOffset(),
	m_cxyBorderRound(),
	m_rcMargin(),
	m_rcPaint(),
	m_rcBorderSize(),
	m_cursorType(kCursorArrow),
	m_uButtonState(kControlStateNormal),
	m_nBorderSize(0),
	m_nTooltipWidth(300),
	m_nAlpha(255),
	m_nHotAlpha(0),
	m_sToolTipText(),
	m_sToolTipTextId(),
	m_sUserData(),
	m_strBkColor(),
	m_bBKGradient(false),
	m_strBkGradientColor1(),
	m_strBkGradientColor2(),
	m_nBKGradientMode(0),
	m_colorMap(),
	m_strBorderColor(),
	m_gifWeakFlag(),
	m_animationManager(),
	m_imageMap(),
	m_bkImage(),
	m_loadBkImageWeakFlag()
{
	m_colorMap.SetControl(this);
	m_imageMap.SetControl(this);
	m_animationManager.Init(this);
}

Control::Control(const Control& r) :
	PlaceHolder(r),
	OnXmlEvent(),
	OnEvent(),
	m_pUserDataBase(),
	m_bMenuUsed(r.m_bMenuUsed),
	m_bEnabled(r.m_bEnabled),
	m_bMouseEnabled(r.m_bMouseEnabled),
	m_bKeyboardEnabled(r.m_bKeyboardEnabled),
	m_bFocused(r.m_bFocused),
	m_bMouseFocused(r.m_bMouseFocused),
	m_bSetPos(r.m_bSetPos),
	m_bNoFocus(r.m_bNoFocus),
	m_bClip(r.m_bClip),
	m_bGifPlay(r.m_bGifPlay),
	m_bGifPlayEx(r.m_bGifPlayEx),
	m_bReceivePointerMsg(r.m_bReceivePointerMsg),
	m_bNeedButtonUpWhenKillFocus(r.m_bNeedButtonUpWhenKillFocus),
	m_szEstimateSize(r.m_szEstimateSize),
	m_renderOffset(r.m_renderOffset),
	m_cxyBorderRound(r.m_cxyBorderRound),
	m_rcMargin(r.m_rcMargin),
	m_rcPaint(r.m_rcPaint),
	m_rcBorderSize(r.m_rcBorderSize),
	m_cursorType(r.m_cursorType),
	m_uButtonState(kControlStateNormal),
	m_nBorderSize(r.m_nBorderSize),
	m_nTooltipWidth(r.m_nTooltipWidth),
	m_nAlpha(r.m_nAlpha),
	m_nHotAlpha(r.m_nHotAlpha),
	m_sToolTipText(r.m_sToolTipText),
	m_sToolTipTextId(r.m_sToolTipTextId),
	m_sUserData(r.m_sUserData),
	m_strBkColor(r.m_strBkColor),
	m_bBKGradient(r.m_bBKGradient),
	m_strBkGradientColor1(r.m_strBkGradientColor1),
	m_strBkGradientColor2(r.m_strBkGradientColor2),
	m_nBKGradientMode(r.m_nBKGradientMode),
	m_colorMap(r.m_colorMap),
	m_strBorderColor(r.m_strBorderColor),
	m_gifWeakFlag(),
	m_animationManager(r.m_animationManager),
	m_imageMap(r.m_imageMap),
	m_bkImage(r.m_bkImage),
	m_loadBkImageWeakFlag()
{
	m_colorMap.SetControl(this);
	m_imageMap.SetControl(this);
	m_animationManager.Init(this);
	if (r.m_bGifPlay)
	{
		this->GifPlay();
	}
	m_nBkColorAlpha = r.m_nBkColorAlpha;
}

Control::~Control()
{
	if (m_pWindow) {
		m_pWindow->ReapObjects(this);
	}
}

std::wstring Control::GetBkColor() const
{
	return m_strBkColor;
}

void Control::SetBkColor(const std::wstring& strColor)
{
	ASSERT(strColor.empty() || GlobalManager::GetTextColor(strColor) != 0);
	if( m_strBkColor == strColor ) return;

	m_strBkColor = strColor;
	Invalidate();
}


void Control::SetBkColorAlpha(int alpha)
{
	m_nBkColorAlpha = alpha<<24;
}

bool Control::GetBKGradient()
{
	return m_bBKGradient;
}

void Control::SetBKGradient(const bool bGradient)
{
	if (m_bBKGradient != bGradient)
	{
		m_bBKGradient = bGradient;
		Invalidate();
	}
}
void Control::SetBkGradientColor(const std::wstring& strColor, const std::wstring& strColor1)
{
	ASSERT(strColor.empty() || GlobalManager::GetTextColor(strColor) != 0);
	ASSERT(strColor1.empty() || GlobalManager::GetTextColor(strColor1) != 0);
	if (m_strBkGradientColor1 != strColor ||
		m_strBkGradientColor2 != strColor1)
	{
		m_strBkGradientColor1 = strColor;
		m_strBkGradientColor2 = strColor1;
		Invalidate();
	}
}

void Control::SetBkGradientColor1(const std::wstring& strColor)
{
	ASSERT(strColor.empty() || GlobalManager::GetTextColor(strColor) != 0);
	if (m_strBkGradientColor1 != strColor)
	{
		m_strBkGradientColor1 = strColor;
	}
}

void Control::SetBkGradientColor2(const std::wstring& strColor)
{
	ASSERT(strColor.empty() || GlobalManager::GetTextColor(strColor) != 0);
	if (m_strBkGradientColor2 != strColor )
	{
		m_strBkGradientColor2 = strColor;
	}
}

void Control::SetBKGradientMode(const int mode)
{
	if (m_nBKGradientMode != mode)
	{
		m_nBKGradientMode = mode;
		Invalidate();
	}
}

std::wstring Control::GetStateColor(ControlStateType stateType)
{
	return m_colorMap[stateType];
}

void Control::SetStateColor(ControlStateType stateType, const std::wstring& strColor)
{
	ASSERT(GlobalManager::GetTextColor(strColor) != 0);
	if( m_colorMap[stateType] == strColor ) return;

	if (stateType == kControlStateHot) {
		m_animationManager.SetFadeHot(true);
	}
	m_colorMap[stateType] = strColor;
	Invalidate();
}

std::wstring Control::GetBkImage() const
{
	return m_bkImage.imageAttribute.simageString;
}

std::string Control::GetUTF8BkImage() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(m_bkImage.imageAttribute.simageString.c_str(), strOut, CP_UTF8);
	return strOut;
}

void Control::SetBkImage(const std::wstring& strImage)
{
	StopGifPlay();
	m_bkImage.SetImageString(strImage);
	if (m_bGifPlayEx)
		m_bGifPlay = m_bkImage.imageAttribute.nPlayCount != 0;
	else
		m_bGifPlay = false;

	if (GetFixedWidth() == DUI_LENGTH_AUTO || GetFixedHeight() == DUI_LENGTH_AUTO) {
		ArrangeAncestor();
	}
	else {
		Invalidate();
	}
}

void Control::SetUTF8BkImage(const std::string& strImage)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strImage, strOut, CP_UTF8);
	SetBkImage(strOut);
}

std::wstring Control::GetStateImage(ControlStateType stateType)
{
	return m_imageMap.GetImagePath(kStateImageBk, stateType);
}

void Control::SetStateImage(ControlStateType stateType, const std::wstring& strImage)
{
	if (stateType == kControlStateHot) {
		m_animationManager.SetFadeHot(true);
	}
	m_imageMap.SetImage(kStateImageBk, stateType, strImage);
	if (GetFixedWidth() == DUI_LENGTH_AUTO || GetFixedHeight() == DUI_LENGTH_AUTO) {
		ArrangeAncestor();
	}
	else {
		Invalidate();
	}
}

std::wstring Control::GetForeStateImage(ControlStateType stateType)
{
	return m_imageMap.GetImagePath(kStateImageFore, stateType);
}

void Control::SetForeStateImage(ControlStateType stateType, const std::wstring& strImage)
{
	if (stateType == kControlStateHot) {
		m_animationManager.SetFadeHot(true);
	}
	m_imageMap.SetImage(kStateImageFore, stateType, strImage);
	Invalidate();
}

ControlStateType Control::GetState() const
{
	return m_uButtonState;
}

void Control::SetState(ControlStateType pStrState) 
{
	if (pStrState == kControlStateNormal) {
		m_nHotAlpha = 0;
	}
	else if (pStrState == kControlStateHot) {
		m_nHotAlpha = 255;
	}

	m_uButtonState = pStrState;
	Invalidate();
}

Image* Control::GetEstimateImage()
{
	Image* estimateImage = nullptr;
	if (!m_bkImage.imageAttribute.sImageName.empty()) {
		estimateImage = &m_bkImage;
	}
	else {
		estimateImage = m_imageMap.GetEstimateImage(kStateImageBk);
	}

	return estimateImage;
}

int Control::GetBorderSize() const
{
	return m_nBorderSize;
}

void Control::SetBorderSize(int nSize)
{
	DpiManager::GetInstance()->ScaleInt(nSize);
	if (m_nBorderSize == nSize) return;

	m_nBorderSize = nSize;
	Invalidate();
}

std::wstring Control::GetBorderColor() const
{
    return m_strBorderColor;
}

void Control::SetBorderColor(const std::wstring& strBorderColor)
{
    if( m_strBorderColor == strBorderColor ) return;

    m_strBorderColor = strBorderColor;
    Invalidate();
}

void Control::SetBorderSize(UiRect rc)
{
	DpiManager::GetInstance()->ScaleRect(rc);
	m_rcBorderSize = rc;
	Invalidate();
}

int Control::GetLeftBorderSize() const
{
	return m_rcBorderSize.left;
}

void Control::SetLeftBorderSize(int nSize)
{
	DpiManager::GetInstance()->ScaleInt(nSize);
	m_rcBorderSize.left = nSize;
	Invalidate();
}

int Control::GetTopBorderSize() const
{
	return m_rcBorderSize.top;
}

void Control::SetTopBorderSize(int nSize)
{
	DpiManager::GetInstance()->ScaleInt(nSize);
	m_rcBorderSize.top = nSize;
	Invalidate();
}

int Control::GetRightBorderSize() const
{
	return m_rcBorderSize.right;
}

void Control::SetRightBorderSize(int nSize)
{
	DpiManager::GetInstance()->ScaleInt(nSize);
	m_rcBorderSize.right = nSize;
	Invalidate();
}

int Control::GetBottomBorderSize() const
{
	return m_rcBorderSize.bottom;
}

void Control::SetBottomBorderSize(int nSize)
{
	DpiManager::GetInstance()->ScaleInt(nSize);
	m_rcBorderSize.bottom = nSize;
	Invalidate();
}

CSize Control::GetBorderRound() const
{
    return m_cxyBorderRound;
}

void Control::SetBorderRound(CSize cxyRound)
{
	DpiManager::GetInstance()->ScaleSize(cxyRound);
    m_cxyBorderRound = cxyRound;
    Invalidate();
}

void Control::SetBorderRound2(float fRound)
{
	m_fBorderRound2 = DpiManager::GetInstance()->ScaleDouble(fRound);
	Invalidate();
}

CursorType Control::GetCursorType() const
{
	return m_cursorType;
}

void Control::SetCursorType(CursorType flag)
{
	m_cursorType = flag;
}

std::wstring Control::GetToolTipText() const
{
	std::wstring strText = m_sToolTipText;
	if (strText.empty() && !m_sToolTipTextId.empty()) {
		strText = MutiLanSupport::GetInstance()->GetStringViaID(m_sToolTipTextId);
	}

	return strText;
}

std::string Control::GetUTF8ToolTipText() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(GetToolTipText(), strOut, CP_UTF8);
	return strOut;
}

void Control::SetToolTipText(const std::wstring& strText)
{
	std::wstring strTemp(strText);
	StringHelper::ReplaceAll(_T("<n>"),_T("\r\n"), strTemp);
	m_sToolTipText = strTemp;

	Invalidate();
}

void  Control::SetToolTipMode(const std::wstring& mode)
{
	std::wstring strTemp(mode);
	StringHelper::ReplaceAll(_T("<n>"), _T("\r\n"), strTemp);

	m_sToolTipMode = strTemp;
}

std::wstring  Control::GetToolTipMode()
{
	return m_sToolTipMode;
} 

void Control::SetToolTipDistance(int distance)
{
	m_sToolTipDistance = distance;
}

int  Control::GetToolTipDistance()
{
	return m_sToolTipDistance;
}

void Control::SetUTF8ToolTipText(const std::string& strText)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strText, strOut, CP_UTF8);
	if (strOut.empty()) {
		m_sToolTipText = _T("");
		Invalidate();//为空则一律重刷
		return ;
	}

	if (m_sToolTipText != strOut) {
		SetToolTipText(strOut);
	}
}

void Control::SetToolTipTextId(const std::wstring& strTextId)
{
	if (m_sToolTipTextId == strTextId) return;
	m_sToolTipTextId = strTextId;

	Invalidate();
}

void Control::SetUTF8ToolTipTextId(const std::string& strTextId)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strTextId, strOut, CP_UTF8);
	SetToolTipTextId(strOut);
}

void Control::SetToolTipWidth( int nWidth )
{
	DpiManager::GetInstance()->ScaleInt(nWidth);
	m_nTooltipWidth=nWidth;
}

int Control::GetToolTipWidth(void) const
{
	return m_nTooltipWidth;
}

bool Control::IsContextMenuUsed() const
{
    return m_bMenuUsed;
}

void Control::SetContextMenuUsed(bool bMenuUsed)
{
    m_bMenuUsed = bMenuUsed;
}

std::wstring Control::GetDataID() const
{
    return m_sUserData;
}

std::string Control::GetUTF8DataID() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(m_sUserData, strOut, CP_UTF8);
	return strOut;
}

void Control::SetDataID(const std::wstring& strText)
{
    m_sUserData = strText;
}

void Control::SetUTF8DataID(const std::string& strText)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strText, strOut, CP_UTF8);
	m_sUserData = strOut;
}

UserDataBase* Control::GetUserDataBase() const
{
	return m_pUserDataBase.get();
}

void Control::SetUserDataBase(UserDataBase* pUserDataBase)
{
	m_pUserDataBase.reset(pUserDataBase);
}

void Control::SetVisible(bool bVisible)
{
	if (bVisible) {
		m_animationManager.Appear();
	}
	else {
		m_animationManager.Disappear();
	}
}

void Control::SetInternVisible(bool bVisible)
{
	m_bInternVisible = bVisible;
	if (!bVisible && m_pWindow && m_pWindow->GetFocus() == this) {
		m_pWindow->SetFocus(NULL) ;
	}

	if (!IsVisible()) {
		StopGifPlay();
	}
}

void Control::SetVisible_(bool bVisible)
{
	if (m_bVisible == bVisible) return;
	bool v = IsVisible();
	m_bVisible = bVisible;
	if (m_bFocused) m_bFocused = false;
	if (!bVisible && m_pWindow && m_pWindow->GetFocus() == this) {
		m_pWindow->SetFocus(NULL);
	}
	if (IsVisible() != v) {
		ArrangeAncestor();
	}

	if (!IsVisible()) {
		StopGifPlay();
	}
}

bool Control::IsEnabled() const
{
    return m_bEnabled;
}

void Control::SetEnabled(bool bEnabled)
{
    if( m_bEnabled == bEnabled ) return;

    m_bEnabled = bEnabled;
	if (m_bEnabled) {
		m_uButtonState = kControlStateNormal;
		m_nHotAlpha = 0;
	}
	else {
		m_uButtonState = kControlStateDisabled;
	}
    Invalidate();
}

bool Control::IsMouseEnabled() const
{
    return m_bMouseEnabled;
}

void Control::SetMouseEnabled(bool bEnabled)
{
    m_bMouseEnabled = bEnabled;
}

bool Control::IsKeyboardEnabled() const
{
	return m_bKeyboardEnabled ;
}
void Control::SetKeyboardEnabled(bool bEnabled)
{
	m_bKeyboardEnabled = bEnabled ; 
}

bool Control::IsFocused() const
{
    return m_bFocused;
}

void Control::SetFocus()
{
	if( m_bNoFocus )
		return;
    if( m_pWindow != NULL ) m_pWindow->SetFocus(this);
}

UINT Control::GetControlFlags() const
{
	return UIFLAG_TABSTOP;
}

void Control::SetNoFocus()
{
	m_bNoFocus = true;
}

void Control::Activate()
{

}

bool Control::IsActivatable() const
{
	if (!IsVisible()) return false;
	if (!IsEnabled()) return false;
	return true;
}

Control* Control::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags, CPoint scrollPos)
{
    if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
    if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
	if( (uFlags & UIFIND_HITTEST) != 0 && (!m_bMouseEnabled || !::PtInRect(&m_rcItem, * static_cast<LPPOINT>(pData))) ) return NULL;
    return Proc(this, pData);
}

UiRect Control::GetPos(bool bContainShadow) const
{
	UiRect pos = m_rcItem;
	if (m_pWindow && !bContainShadow) {
		UiRect shadowLength = m_pWindow->GetShadowCorner();
		pos.Offset(-shadowLength.left, -shadowLength.top);
	}
	return pos;
}

void Control::SetPos(UiRect rc)
{
	if (rc.right < rc.left) rc.right = rc.left;
	if (rc.bottom < rc.top) rc.bottom = rc.top;

	if (m_rcItem.Equal(rc)) {
		m_bIsArranged = false;
		return;
	}

	UiRect invalidateRc = m_rcItem;
	if (::IsRectEmpty(&invalidateRc)) invalidateRc = rc;

	m_rcItem = rc;
	if (m_pWindow == NULL) return;

	if (!m_bSetPos) {
		m_bSetPos = true;
		m_pWindow->SendNotify(this, kEventResize, NULL, NULL);
		m_bSetPos = false;
	}

	m_bIsArranged = false;
	invalidateRc.Union(m_rcItem);

	Control* pParent = this;
	UiRect rcTemp;
	UiRect rcParent;
	CPoint offset = GetScrollOffset();
	invalidateRc.Offset(-offset.x, -offset.y);
	while ((pParent = pParent->GetParent()) != nullptr)
	{
		rcTemp = invalidateRc;
		rcParent = pParent->GetPos();
		if (!::IntersectRect(&invalidateRc, &rcTemp, &rcParent)) {
			return;
		}
	}
	m_pWindow->Invalidate(invalidateRc);
}

UiRect Control::GetMargin() const
{
	return m_rcMargin;
}

void Control::SetMargin(UiRect rcMargin, bool bNeedDpiScale)
{
	if (bNeedDpiScale)
		DpiManager::GetInstance()->ScaleRect(rcMargin);

	if (!m_rcMargin.Equal(rcMargin)) {
		m_rcMargin = rcMargin;
		ArrangeAncestor();
	}
}

CSize Control::EstimateSize(CSize szAvailable)
{
	CSize imageSize = m_cxyFixed;
	if (GetFixedWidth() == DUI_LENGTH_AUTO || GetFixedHeight() == DUI_LENGTH_AUTO) {
		if (!m_bReEstimateSize) {
			return m_szEstimateSize;
		}
		Image* image = GetEstimateImage();
		if (image) {
			auto imageAttribute = image->imageAttribute;
			if (imageAttribute.rcSource.left != DUI_NOSET_VALUE && imageAttribute.rcSource.top != DUI_NOSET_VALUE
				&& imageAttribute.rcSource.right != DUI_NOSET_VALUE && imageAttribute.rcSource.bottom != DUI_NOSET_VALUE) {
				if ((GetFixedWidth() != imageAttribute.rcSource.right - imageAttribute.rcSource.left)) {
					SetFixedWidth(imageAttribute.rcSource.right - imageAttribute.rcSource.left);
				}
				if ((GetFixedHeight() != imageAttribute.rcSource.bottom - imageAttribute.rcSource.top)) {
					SetFixedHeight(imageAttribute.rcSource.bottom - imageAttribute.rcSource.top);
				}
				return m_cxyFixed;
			}

			GetImage(*image);
			if (image->imageCache) {
				if (GetFixedWidth() == DUI_LENGTH_AUTO) {
					int image_width = image->imageCache->nX;
					DpiManager::GetInstance()->ScaleInt(image_width);
					imageSize.cx = image_width;
				}
				if (GetFixedHeight() == DUI_LENGTH_AUTO) {
					int image_height = image->imageCache->nY;
					DpiManager::GetInstance()->ScaleInt(image_height);
					imageSize.cy = image_height;
				}
			}
		}

		m_bReEstimateSize = false;
		CSize textSize = EstimateText(szAvailable, m_bReEstimateSize);
		if (GetFixedWidth() == DUI_LENGTH_AUTO && imageSize.cx < textSize.cx) {
			imageSize.cx = textSize.cx;
		}
		if (GetFixedHeight() == DUI_LENGTH_AUTO && imageSize.cy < textSize.cy) {
			imageSize.cy = textSize.cy;
		}

		m_szEstimateSize = imageSize;
	}

	return imageSize;
}

CSize Control::EstimateText(CSize szAvailable, bool& bReEstimateSize)
{
	return CSize();
}

bool Control::IsPointInWithScrollOffset(const CPoint& point) const
{
	CPoint scrollOffset = GetScrollOffset();
	CPoint newPoint = point;
	newPoint.Offset(scrollOffset);
	return m_rcItem.IsPointIn(newPoint);
}

void Control::HandleMessageTemplate(EventType eventType, WPARAM wParam, LPARAM lParam, TCHAR tChar, CPoint mousePos, FLOAT pressure)
{
	EventArgs msg;
	msg.pSender = this;
	msg.Type = eventType;
	msg.chKey = tChar;
	msg.wParam = wParam;
	msg.lParam = lParam;
	msg.pressure = pressure;
	if (0 == mousePos.x == mousePos.y)
		msg.ptMouse = m_pWindow->GetLastMousePos();
	else
		msg.ptMouse = mousePos;
	msg.dwTimestamp = ::GetTickCount();

	HandleMessageTemplate(msg);
}

void Control::HandleMessageTemplate(EventArgs& msg)
{
	if (msg.Type == kEventInternalDoubleClick || msg.Type == kEventInternalMenu
		|| msg.Type == kEventInternalSetFocus || msg.Type == kEventInternalKillFocus) {
		HandleMessage(msg);
		return;
	}
	bool bRet = true;

	if (this == msg.pSender) {
		std::weak_ptr<nbase::WeakFlag> weakflag = GetWeakFlag();
		auto callback = OnEvent.find(msg.Type);
		if (callback != OnEvent.end()) {
			bRet = callback->second(&msg);
		}
		if (weakflag.expired()) {
			return;
		}

		callback = OnEvent.find(kEventAll);
		if (callback != OnEvent.end()) {
			bRet = callback->second(&msg);
		}
		if (weakflag.expired()) {
			return;
		}

		if (bRet) {
			auto callback = OnXmlEvent.find(msg.Type);
			if (callback != OnXmlEvent.end()) {
				bRet = callback->second(&msg);
			}
			if (weakflag.expired()) {
				return;
			}

			callback = OnXmlEvent.find(kEventAll);
			if (callback != OnXmlEvent.end()) {
				bRet = callback->second(&msg);
			}
			if (weakflag.expired()) {
				return;
			}
		}
	}
	else {
		ASSERT(FALSE);
	}
	
    if(bRet) {
		HandleMessage(msg);
	}
}

void Control::HandleMessage(EventArgs& msg)
{
	if( !IsMouseEnabled() && msg.Type > kEventMouseBegin && msg.Type < kEventMouseEnd ) {
		if( m_pParent != NULL ) m_pParent->HandleMessageTemplate(msg);
		return;
	}
	else if( msg.Type == kEventSetCursor ) {
		if (m_cursorType == kCursorHand) {
			if (IsEnabled()) {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
			}
			else {
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
			}
			return;
		}
		else if (m_cursorType == kCursorArrow){
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
			return;
		}
		else if (m_cursorType == kCursorHandIbeam){
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
			return;
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if (msg.Type == kEventInternalSetFocus) {
		SetState(kControlStateHot);
        m_bFocused = true;
        Invalidate();
		return;
    }
	else if (msg.Type == kEventInternalKillFocus) {
		if (m_bEnabled)
		{
			SetState(kControlStateNormal);
		}
        m_bFocused = false;
        Invalidate();
		return;
    }
	else if (msg.Type == kEventInternalMenu && IsEnabled()) {
        if( IsContextMenuUsed() ) {
            m_pWindow->SendNotify(this, kEventMouseMenu, msg.wParam, msg.lParam);
            return;
        }
    }
	else if( msg.Type == kEventMouseEnter ) {
		if (msg.pSender != this && m_pWindow) {
			if (!IsChild(this, m_pWindow->GetNewHover())) {
				return;
			}
		}
		MouseEnter(msg);
	}
	else if( msg.Type == kEventMouseLeave ) {
		if (msg.pSender != this && m_pWindow) {
			if (IsChild(this, m_pWindow->GetNewHover())) {
				return;
			}
		}
		MouseLeave(msg);
	}
	else if (msg.Type == kEventMouseButtonDown || msg.Type == kEventInternalDoubleClick) {
		ButtonDown(msg);
		return;
	}
	else if (msg.Type == kEventMouseButtonUp) {
		ButtonUp(msg);
		return;
	}
	else if (msg.Type == kEventPointDown && m_bReceivePointerMsg) {
		ButtonDown(msg);
		return;
	}
	else if (msg.Type == kEventPointUp && m_bReceivePointerMsg) {
		ButtonUp(msg);
		return;
	}

    if( m_pParent != NULL ) m_pParent->HandleMessageTemplate(msg);
}

bool Control::HasHotState()
{
	// 判断本控件是否有hot状态
	return m_colorMap.HasHotColor() || m_imageMap.HasHotImage();
}

bool Control::MouseEnter(EventArgs& msg)
{
	if( IsEnabled() ) {
		if ( m_uButtonState == kControlStateNormal) {
			m_uButtonState = kControlStateHot;
			if (HasHotState()) {
				m_animationManager.MouseEnter();
				Invalidate();
			}
			return true;
		}
	}

	return false;
}

bool Control::MouseLeave(EventArgs& msg)
{
	if( IsEnabled() ) {
		if (m_uButtonState == kControlStateHot) {
			m_uButtonState = kControlStateNormal;
			if (HasHotState()) {
				m_animationManager.MouseLeave();
				Invalidate();
			}
			return true;
		}
	}

	return false;
}

bool Control::ButtonDown(EventArgs& msg)
{
	bool ret = false;
	if( IsEnabled() ) {
		m_uButtonState = kControlStatePushed;
		SetMouseFocused(true);
		Invalidate();
		ret = true;
	}

	return ret;
}

bool Control::ButtonUp(EventArgs& msg)
{
	bool ret = false;
	if( IsMouseFocused() ) {
		SetMouseFocused(false);
		auto player = m_animationManager.GetAnimationPlayer(kAnimationHot);
		if (player)
			player->Stop();

		Invalidate();
		if( IsPointInWithScrollOffset(msg.ptMouse) ) {
			if (msg.Type == kEventPointUp) {
				m_uButtonState = kControlStateNormal;
				m_nHotAlpha = 0;
			}
			else {
				m_uButtonState = kControlStateHot;
				m_nHotAlpha = 255;
			}
			Activate();
			ret = true;
		}
		else {
			m_uButtonState = kControlStateNormal;
			m_nHotAlpha = 0;
		}
	}

	return ret;
}

int GetDotPosByKey(const std::wstring &strValue, const std::wstring &key)
{
	int nPos1 = strValue.find(key);
	if (nPos1 == -1)
		return -1;

	int nPos2 = strValue.find(_T("'"), nPos1 + key.size());
	if (nPos2 == -1)
		return -1; //first

	int nPos3 = strValue.find(_T("'"), nPos2 + 1);
	if (nPos3 == -1)
		return -1; //second

	return nPos2;
}

/// 合成图片解析 <compositeimage="file='sys_combo_btn.png' dest='0,0,20,20' size='80,16' frame='5' state='3,0,2'"/>
//一帧 kControlStateDisabled 二帧kControlStateNormal 三帧kControlStatePushed
void ParseCompositeImage(Control *control,const std::wstring& strValue,std::function<void(int,std::wstring)> OnStateImage)
{
	SIZE imageSize = { 0 };
	int frameCount = 0;

	int findPos = GetDotPosByKey(strValue, L"size");
	if (findPos == -1) return;

	LPTSTR lpszValue = NULL;
	imageSize.cx = _tcstol(strValue.c_str() + findPos + 1, &lpszValue, 10);  ASSERT(lpszValue);
	imageSize.cy = _tcstol(lpszValue + 1, &lpszValue, 10);    ASSERT(lpszValue);

	findPos = GetDotPosByKey(strValue, L"frame");
	if (findPos == -1) return;

	frameCount = _tcstol(strValue.c_str() + findPos + 1, &lpszValue, 10);  ASSERT(lpszValue);
	int frameSize = imageSize.cx / frameCount;	//单帧的大小

	int state[4] = { -1, -1, -1, -1 };
	findPos = GetDotPosByKey(strValue, L"state");
	if (findPos != -1)
	{
		state[0] = _tcstol(strValue.c_str() + findPos + 1, &lpszValue, 10);  ASSERT(lpszValue);
		state[1] = _tcstol(lpszValue + 1, &lpszValue, 10);    ASSERT(lpszValue);
		if (lpszValue[0] != '\'')
		{
			state[2] = _tcstol(lpszValue + 1, &lpszValue, 10);    ASSERT(lpszValue);
		}
		if (lpszValue[0] != '\'')
		{
			state[3] = _tcstol(lpszValue + 1, &lpszValue, 10);    ASSERT(lpszValue);
		}
	}

	ui::UiRect destRc;
	findPos = GetDotPosByKey(strValue, L"dest");
	if (findPos != -1)
	{
		destRc.left= _tcstol(strValue.c_str() + findPos + 1, &lpszValue, 10);  ASSERT(lpszValue);
		destRc.top = _tcstol(lpszValue + 1, &lpszValue, 10);    ASSERT(lpszValue);
		destRc.right = _tcstol(lpszValue + 1, &lpszValue, 10);    ASSERT(lpszValue);
		destRc.bottom = _tcstol(lpszValue + 1, &lpszValue, 10);    ASSERT(lpszValue);
	}

	//////////////////////////////////////////////////////////////////////////
	int startPos = strValue.find(L"file");
	if (startPos == -1) return;

	int endPos = strValue.find(_T("'"), startPos + 4);
	if (endPos == -1) return;

	endPos = strValue.find(_T("'"), endPos + 1);
	if (endPos == -1) return;

	std::wstring file = strValue.substr(startPos, endPos - startPos + 1);
	TCHAR AttributeValue[128] = { 0 };

	int curFrame = 0;
	while (curFrame < frameCount)
	{
		if (destRc.IsRectEmpty())
		{
			wsprintf(AttributeValue, L"%s source='%d,%d,%d,%d'", file.c_str(), curFrame*frameSize, 0, (curFrame + 1)*frameSize, imageSize.cy);
		}
		else
		{
			wsprintf(AttributeValue, L"%s dest='%d,%d,%d,%d' source='%d,%d,%d,%d'", file.c_str(), destRc.left,destRc.top,destRc.right,destRc.bottom,
				curFrame*frameSize, 0, (curFrame + 1)*frameSize, imageSize.cy);
		}
		
		if (OnStateImage)
		{
			if ( curFrame<=3 && state[curFrame] != -1)
			{
				OnStateImage(state[curFrame], AttributeValue);
			}
			else
			{
				OnStateImage(curFrame, AttributeValue);
			}
		}
		curFrame++;
	}
}

void Control::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if ( strName == _T("class") ) {
		SetClass(strValue);
	}
	else if (strName == _T("usegdiplus")) {
		m_UseGdiPlus = _wtoi(strValue.c_str()) == 0 ? false : true;
	}
	else if (strName == _T("roundbkimage"))
	{
		m_isRoundBkImage = (strValue == L"true");
	}
	else if (strName == _T("bksolidcircle"))
	{
		m_bkSolidCircle = (strValue == L"true");
	}
	else if (strName == _T("roundstyle"))
	{
		LPCTSTR pValue = strValue.c_str();
		while (*pValue > _T('\0') && *pValue <= _T(' ')) pValue = ::CharNext(pValue);
		m_bRoundStyle = _ttoi(pValue);
	}
	else if (strName == _T("filltype")) {
		m_FillType = _wtoi(strValue.c_str());
	}
	else if( strName == _T("halign") ) {
		if (strValue == _T("left")) {
			SetHorAlignType(kHorAlignLeft);
		}
		else if (strValue == _T("center")) {
			SetHorAlignType(kHorAlignCenter);
		}
		else if (strValue == _T("right")) {
			SetHorAlignType(kHorAlignRight);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if( strName == _T("valign") ) {
		if (strValue == _T("top")) {
			SetVerAlignType(kVerAlignTop);
		}
		else if (strValue == _T("center")) {
			SetVerAlignType(kVerAlignCenter);
		}
		else if (strValue == _T("bottom")) {
			SetVerAlignType(kVerAlignBottom);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if( strName == _T("margin") ) {
        UiRect rcMargin;
        LPTSTR pstr = NULL;
        rcMargin.left = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);    
        rcMargin.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
        rcMargin.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
        rcMargin.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
        SetMargin(rcMargin);
    }
    else if( strName == _T("bkcolor")) { 
		LPCTSTR pValue = strValue.c_str();
        while( *pValue > _T('\0') && *pValue <= _T(' ') ) pValue = ::CharNext(pValue);
        SetBkColor(pValue);
	}
	else if (strName == _T("bkcolor1")) {
		LPCTSTR pValue = strValue.c_str();
		while (*pValue > _T('\0') && *pValue <= _T(' ')) pValue = ::CharNext(pValue);
		SetBkGradientColor1(pValue);
	}
	else if (strName == _T("bkcolor2")) {
		LPCTSTR pValue = strValue.c_str();
		while (*pValue > _T('\0') && *pValue <= _T(' ')) pValue = ::CharNext(pValue);
		SetBkGradientColor2(pValue);
	}
	else if (strName == _T("bkgradient")) {
		SetBKGradient(strValue == _T("true"));
	}
	else if (strName == _T("bkgradientmode")) {
		LPCTSTR pValue = strValue.c_str();
		while (*pValue > _T('\0') && *pValue <= _T(' ')) pValue = ::CharNext(pValue);
		SetBKGradientMode(_ttoi(pValue));
	}
	else if (strName == _T("bordersize")) {
		std::wstring nValue = strValue;
		if (nValue.find(',') == std::wstring::npos) {
			SetBorderSize(_ttoi(strValue.c_str()));
			UiRect rcBorder;
			SetBorderSize(rcBorder);
		}
		else {
			UiRect rcBorder;
			LPTSTR pstr = NULL;
			rcBorder.left = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
			rcBorder.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcBorder.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcBorder.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			SetBorderSize(rcBorder);
		}
	}
	else if (strName == _T("borderstyle")) {
		m_nBorderStyle = (Gdiplus::DashStyle)_ttoi(strValue.c_str());
	}
    else if( strName == _T("borderround") ) {
        CSize cxyRound;
        LPTSTR pstr = NULL;
        cxyRound.cx = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);    
        cxyRound.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);     
        SetBorderRound(cxyRound);
    }
	else if (strName == _T("borderround2")) 
	{
		float fRound = _tcstof(strValue.c_str(), nullptr);
		SetBorderRound2(fRound);
	}
	else if( strName == _T("width") ) {
		if ( strValue == _T("stretch") ) {
			SetFixedWidth(DUI_LENGTH_STRETCH);
		}
		else if ( strValue == _T("auto") ) {
			SetFixedWidth(DUI_LENGTH_AUTO);
		}
		else {
			ASSERT(_ttoi(strValue.c_str()) >= 0);
			SetFixedWidth(_ttoi(strValue.c_str()));
		}
	}
	else if( strName == _T("height") ) {
		if ( strValue == _T("stretch") ) {
			SetFixedHeight(DUI_LENGTH_STRETCH);
		}
		else if ( strValue == _T("auto") ) {
			SetFixedHeight(DUI_LENGTH_AUTO);
		}
		else {
			ASSERT(_ttoi(strValue.c_str()) >= 0);
			SetFixedHeight(_ttoi(strValue.c_str()));
		}
	}
	else if( strName == _T("maxwidth") ) {
		if ( strValue == _T("stretch") ) {
			SetMaxWidth(DUI_LENGTH_STRETCH);
		}
		else if ( strValue == _T("auto") ) {
			SetMaxWidth(DUI_LENGTH_AUTO);
		}
		else {
			ASSERT(_ttoi(strValue.c_str()) >= 0);
			SetMaxWidth(_ttoi(strValue.c_str()));
		}
	}
	else if( strName == _T("maxheight") ) {
		if ( strValue == _T("stretch") ) {
			SetMaxHeight(DUI_LENGTH_STRETCH);
		}
		else if ( strValue == _T("auto") ) {
			SetMaxHeight(DUI_LENGTH_AUTO);
		}
		else {
			ASSERT(_ttoi(strValue.c_str()) >= 0);
			SetMaxHeight(_ttoi(strValue.c_str()));
		}
	}
	else if( strName == _T("state") ) {
		if( strValue == _T("normal") ) SetState(kControlStateNormal);
		else if( strValue == _T("hot") ) SetState(kControlStateHot);
		else if( strValue == _T("pushed") ) SetState(kControlStatePushed);
		else if( strValue == _T("disabled") ) SetState(kControlStateDisabled);
		else ASSERT(FALSE);
	}
	else if( strName == _T("cursortype") ) {
		if (strValue == _T("arrow")) {
			SetCursorType(kCursorArrow);
		}
		else if ( strValue == _T("hand") ) {
			SetCursorType(kCursorHand);
		}
		else if (strValue == _T("ibeam")) {
			SetCursorType(kCursorHandIbeam);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if (strName == _T("renderoffset")) {
		CPoint renderOffset;
		LPTSTR pstr = NULL;
		renderOffset.x = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
		renderOffset.y = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);

		DpiManager::GetInstance()->ScalePoint(renderOffset);
		SetRenderOffset(renderOffset);
	}
	else if (strName == _T("normalcolor"))	SetStateColor(kControlStateNormal, strValue);
	else if (strName == _T("hotcolor"))	SetStateColor(kControlStateHot, strValue);
	else if (strName == _T("pushedcolor"))	SetStateColor(kControlStatePushed, strValue);
	else if (strName == _T("disabledcolor"))	SetStateColor(kControlStateDisabled, strValue);
	else if (strName == _T("bordercolor")) SetBorderColor(strValue);
	else if (strName == _T("leftbordersize")) SetLeftBorderSize(_ttoi(strValue.c_str()));
	else if (strName == _T("topbordersize")) SetTopBorderSize(_ttoi(strValue.c_str()));
	else if (strName == _T("rightbordersize")) SetRightBorderSize(_ttoi(strValue.c_str()));
	else if (strName == _T("bottombordersize")) SetBottomBorderSize(_ttoi(strValue.c_str()));
	else if (strName == _T("bkimage")) SetBkImage(strValue);
	else if (strName == _T("minwidth")) SetMinWidth(_ttoi(strValue.c_str()));
	else if (strName == _T("minheight")) SetMinHeight(_ttoi(strValue.c_str()));
	else if (strName == _T("name")) SetName(strValue);
	else if (strName == _T("tooltiptext")) SetToolTipText(strValue);
	else if (strName == _T("tooltiptextid")) SetToolTipTextId(strValue);
	else if (strName == _T("tooltipmode")) SetToolTipMode(strValue);
	else if (strName == _T("tooltipdistance")) SetToolTipDistance(_ttoi(strValue.c_str()));
	else if (strName == _T("dataid")) SetDataID(strValue);
	else if (strName == _T("enabled")) SetEnabled(strValue == _T("true"));
	else if (strName == _T("mouse")) SetMouseEnabled(strValue == _T("true"));
	else if (strName == _T("keyboard")) SetKeyboardEnabled(strValue == _T("true"));
	else if (strName == _T("visible")) SetVisible_(strValue == _T("true"));
	else if (strName == _T("fadevisible")) SetVisible(strValue == _T("true"));
	else if (strName == _T("float")) SetFloat(strValue == _T("true"));
	else if (strName == _T("menu")) SetContextMenuUsed(strValue == _T("true"));
	else if (strName == _T("cache")) SetUseCache(strValue == _T("true"));
	else if (strName == _T("nofocus")) SetNoFocus();
	else if (strName == _T("alpha")) SetAlpha(_ttoi(strValue.c_str()));
	else if (strName == _T("normalimage") ) SetStateImage(kControlStateNormal, strValue);
	else if (strName == _T("hotimage") ) SetStateImage(kControlStateHot, strValue);
	else if (strName == _T("pushedimage") ) SetStateImage(kControlStatePushed, strValue);
	else if (strName == _T("disabledimage") ) SetStateImage(kControlStateDisabled, strValue);
	else if (strName == _T("forenormalimage") ) SetForeStateImage(kControlStateNormal, strValue);
	else if (strName == _T("forehotimage") ) SetForeStateImage(kControlStateHot, strValue);
	else if (strName == _T("forepushedimage") ) SetForeStateImage(kControlStatePushed, strValue);
	else if (strName == _T("foredisabledimage") ) SetForeStateImage(kControlStateDisabled, strValue);
	else if (strName == _T("fadespeedratio")) m_animationManager.SetSpeedRatio(_ttof(strValue.c_str()));   //XML中，fadespeedratio 属性必须在最前面，才能生效
	else if (strName == _T("fadealpha")) m_animationManager.SetFadeAlpha(strValue == _T("true"));
	else if (strName == _T("fadehot")) m_animationManager.SetFadeHot(strValue == _T("true"));
	else if (strName == _T("fadewidth")) m_animationManager.SetFadeWidth(strValue == _T("true"));
	else if (strName == _T("fadeheight")) m_animationManager.SetFadeHeight(strValue == _T("true"));
	else if (strName == _T("fadeinoutxfromleft")) m_animationManager.SetFadeInOutX(strValue == _T("true"), false);
	else if (strName == _T("fadeinoutxfromright")) m_animationManager.SetFadeInOutX(strValue == _T("true"), true);
	else if (strName == _T("fadeinoutyfromtop")) m_animationManager.SetFadeInOutY(strValue == _T("true"), false);
	else if (strName == _T("fadeinoutyfrombottom")) m_animationManager.SetFadeInOutY(strValue == _T("true"), true);
	else if (strName == _T("receivepointer")) SetReceivePointerMsg(strValue == _T("true"));
	else if (strName == _T("gifplay")) SetGifPlay(strValue == _T("true"));
	else if (strName == _T("compositeimage"))
	{
		ParseCompositeImage(this, strValue, [this](int stateType, std::wstring AttributeValue)
		{
			SetStateImage((ControlStateType)stateType, AttributeValue);
		});
	}
	else
    {
        ASSERT(FALSE);
    }
}

void Control::SetClass(const std::wstring& strClass)
{
	std::list<std::wstring> splitList = StringHelper::Split(strClass, L" ");
	for (auto it = splitList.begin(); it != splitList.end(); it++) {
		std::wstring pDefaultAttributes = GlobalManager::GetClassAttributes((*it));
		if (pDefaultAttributes.empty() && m_pWindow) {
			pDefaultAttributes = m_pWindow->GetClassAttributes(*it);
		}

		ASSERT(!pDefaultAttributes.empty());
		if( !pDefaultAttributes.empty() ) {
			ApplyAttributeList(pDefaultAttributes);
		}
	}
}

void Control::ApplyAttributeList(const std::wstring& strList)
{
    std::wstring sItem;
    std::wstring sValue;
	LPCTSTR pstrList = strList.c_str();
    while( *pstrList != _T('\0') ) {
        sItem.clear();
        sValue.clear();
        while( *pstrList != _T('\0') && *pstrList != _T('=') ) {
            LPTSTR pstrTemp = ::CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sItem += *pstrList++;
            }
        }
        ASSERT( *pstrList == _T('=') );
        if( *pstrList++ != _T('=') ) return;
        ASSERT( *pstrList == _T('\"') );
        if( *pstrList++ != _T('\"') ) return;
        while( *pstrList != _T('\0') && *pstrList != _T('\"') ) {
            LPTSTR pstrTemp = ::CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sValue += *pstrList++;
            }
        }
        ASSERT( *pstrList == _T('\"') );
        if( *pstrList++ != _T('\"') ) return;
        SetAttribute(sItem, sValue);
        if( *pstrList++ != _T(' ') ) return;
    }
    return;
}

bool Control::OnApplyAttributeList(const std::wstring& strReceiver, const std::wstring& strList, EventArgs* eventArgs)
{
	Control* pReceiverControl;
	if (strReceiver.substr(0, 2) == L".\\" || strReceiver.substr(0, 2) == L"./") {
		pReceiverControl = ((Box*)this)->FindSubControl(strReceiver.substr(2));
	}
	else {
		pReceiverControl = GetWindow()->FindControl(strReceiver);
	}

	if (pReceiverControl) {
		pReceiverControl->ApplyAttributeList(strList);
	}
	else {
		ASSERT(FALSE);
	}

	return true;
}

void Control::GetImage(Image& duiImage) const
{
	if (duiImage.imageCache) {
		return;
	}
	std::wstring sImageName = duiImage.imageAttribute.sImageName;
	std::wstring imageFullPath = sImageName;
	bool is_relative = false;
	if (::PathIsRelative(sImageName.c_str()))
	{
		imageFullPath = GlobalManager::GetResourcePath() + m_pWindow->GetWindowResourcePath() + sImageName;
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
			}
		}
	}

	if (!duiImage.imageCache || duiImage.imageCache->sImageFullPath != imageFullPath) {
		duiImage.imageCache = GlobalManager::GetImage(imageFullPath);
	}
}

bool Control::DrawImage(IRenderContext* pRender, Image& duiImage, const std::wstring& strModify /*= L""*/, int nFade /*= DUI_NOSET_VALUE*/)
{
	if (duiImage.imageAttribute.sImageName.empty()) {
		return false;
	}

	GetImage(duiImage);

	if (!duiImage.imageCache) {
		ASSERT(FALSE);
		duiImage.imageAttribute.Init();
		return false;
	}

	ImageAttribute newImageAttribute = duiImage.imageAttribute;
	if (!strModify.empty()) {
		ImageAttribute::ModifyAttribute(newImageAttribute, strModify);
	}
	UiRect rcNewDest = m_rcItem;
	if (newImageAttribute.rcDest.left != DUI_NOSET_VALUE && newImageAttribute.rcDest.top != DUI_NOSET_VALUE
		&& newImageAttribute.rcDest.right != DUI_NOSET_VALUE && newImageAttribute.rcDest.bottom != DUI_NOSET_VALUE) {
		rcNewDest.left = m_rcItem.left + newImageAttribute.rcDest.left;
		rcNewDest.right = m_rcItem.left + newImageAttribute.rcDest.right;
		rcNewDest.top = m_rcItem.top + newImageAttribute.rcDest.top;
		rcNewDest.bottom = m_rcItem.top + newImageAttribute.rcDest.bottom;
	}
	UiRect rcNewSource = newImageAttribute.rcSource;
	if (rcNewSource.left == DUI_NOSET_VALUE || rcNewSource.top == DUI_NOSET_VALUE
		|| rcNewSource.right == DUI_NOSET_VALUE || rcNewSource.bottom == DUI_NOSET_VALUE) {
		rcNewSource.left = 0;
		rcNewSource.top = 0;
		rcNewSource.right = duiImage.imageCache->nX;
		rcNewSource.bottom = duiImage.imageCache->nY;
	}

	if (m_bkImage.imageCache && m_bkImage.imageCache->IsGif() && m_bGifPlay && !m_bkImage.IsPlaying()) {
		GifPlay();
	}
	else 
	{
		// 绘制圆形背景图
		if (m_isRoundBkImage)
		{
			if (m_strBorderColor.empty() || m_bkImage.imageCache == nullptr) {
				return false;
			}
			DWORD dwBorderColor = 0;
			if (!m_strBorderColor.empty()) {
				dwBorderColor = GlobalManager::GetTextColor(m_strBorderColor);
			}

			if (dwBorderColor == 0)
				return false;

			if (pRender->UseDirectX())
			{
				DrawRoundDX draw_round;
				int one_pix = DpiManager::GetInstance()->ScaleDouble(1.5);		//预留抗锯齿处理
				draw_round.SetRect(m_rcItem.left, m_rcItem.top, m_rcItem.GetWidth() - one_pix, m_rcItem.GetHeight() - one_pix);
				draw_round.SetLine(D2D1::ColorF(dwBorderColor & 0x00ffffffff, dwBorderColor >>24), m_nBorderSize, ui::RenderContext_D3D::Translate2D2DPenDashStyle(m_nBorderStyle));
				draw_round.SetArcSize(m_fBorderRound2);
				draw_round.SetOnePix(one_pix);
				draw_round.SetStyle(m_bRoundStyle);
				draw_round.SetFillType(m_FillType);
				draw_round.DrawRoundRectange(pRender->GetRender(), m_bkImage.GetCurrentHBitmap());
			}
			else {
				HDC  hDC = pRender->GetDC();

				DrawRound draw_round;
				int one_pix = DpiManager::GetInstance()->ScaleDouble(1.5);		//预留抗锯齿处理
				draw_round.SetRect(m_rcItem.left, m_rcItem.top, m_rcItem.GetWidth() - one_pix, m_rcItem.GetHeight() - one_pix);
				draw_round.SetLine(Gdiplus::Color(dwBorderColor), m_nBorderSize, m_nBorderStyle);
				draw_round.SetArcSize(m_fBorderRound2);
				draw_round.SetOnePix(one_pix);
				draw_round.SetStyle(m_bRoundStyle);
				draw_round.SetFillType(m_FillType);
				draw_round.DrawRoundRectange(hDC, m_bkImage.imageCache->GetBitmap());
			}
			
			return true;
		}

		if (m_UseGdiPlus)
		{
			ui::CSize image_size;
			image_size.cx = duiImage.imageCache->nX;
			image_size.cy = duiImage.imageCache->nY;
			int srcx = 0;
			int srcy = 0;

			if (m_FillType == 1)	//填充模式只考虑了宽高相等的情况
			{
				if (image_size.cx > image_size.cy) {
					srcx = (image_size.cx - image_size.cy) / 2;
					image_size.cx = image_size.cy;

				}
				else {
					srcy = (image_size.cy - image_size.cx) / 2;
					image_size.cy = image_size.cx;
				}
			}
			if (pRender->UseDirectX()) {
				int iFade = nFade == DUI_NOSET_VALUE ? newImageAttribute.bFade : nFade;
				ImageInfo* imageInfo = duiImage.imageCache.get();
				pRender->DrawImage(m_rcPaint, duiImage.GetCurrentHBitmap(), imageInfo->IsAlpha(),
					rcNewDest, UiRect(srcx, srcy, image_size.cx,image_size.cy), newImageAttribute.rcCorner, iFade, newImageAttribute.bTiledX, newImageAttribute.bTiledY);
			}
			else {
				HDC  hDC = pRender->GetDC();
				Gdiplus::Graphics graph(hDC);
				Gdiplus::RectF destRect((Gdiplus::REAL)rcNewDest.left, (Gdiplus::REAL)rcNewDest.top, (Gdiplus::REAL)(rcNewDest.GetWidth()), (Gdiplus::REAL)(rcNewDest.GetHeight()));

				graph.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
				graph.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
				graph.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeHighQualityBicubic);
				graph.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

				if (duiImage.imageCache->IsGif() && duiImage.IsPlaying())
				{
					duiImage.GetCurrentHBitmap();
				}
				graph.DrawImage(duiImage.imageCache->GetBitmap(), destRect, srcx, srcy, image_size.cx, image_size.cy, Gdiplus::UnitPixel);
			}
		}
		else
		{
			int iFade = nFade == DUI_NOSET_VALUE ? newImageAttribute.bFade : nFade;
			ImageInfo* imageInfo = duiImage.imageCache.get();
			pRender->DrawImage(m_rcPaint, duiImage.GetCurrentHBitmap(), imageInfo->IsAlpha(),
				rcNewDest, rcNewSource, newImageAttribute.rcCorner, iFade, newImageAttribute.bTiledX, newImageAttribute.bTiledY);
		}
	}

	return true;
}

ui::IRenderContext* Control::GetRenderContext(ui::IRenderContext* pSourceRender /*= nullptr*/)
{
	if (!m_renderContext) {
		m_renderContext = GlobalManager::CreateRenderContext(pSourceRender);
	}
	return m_renderContext.get();
}


void Control::ClearRenderContext()
{
	if (m_renderContext) {
		m_renderContext.reset();
	}
}

void Control::AlphaPaint(IRenderContext* pRender, const UiRect& rcPaint)
{
	if (m_nAlpha == 0) {
		return;
	}

	UiRect rcUnion;
	if( !::IntersectRect(&rcUnion, &rcPaint, &m_rcItem) ) return;

	bool bRoundClip = false;
	if (m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0) {
		bRoundClip = true;
	}

	if (IsAlpha()) {
		CSize size;
		size.cx = m_rcItem.right - m_rcItem.left;
		size.cy = m_rcItem.bottom - m_rcItem.top;
		auto pCacheRender = GetRenderContext(pRender);
		if (pCacheRender) {
			if (pCacheRender->Resize(size.cx, size.cy)) {
				SetCacheDirty(true);
			}

			// IsCacheDirty与m_bCacheDirty意义不一样
			if (m_bCacheDirty) {
				pCacheRender->Clear();
				UiRect rcClip = { 0, 0, size.cx, size.cy };
				AutoClip alphaClip(pCacheRender, rcClip, m_bClip);
				AutoClip roundAlphaClip(pCacheRender, rcClip, m_cxyBorderRound.cx, m_cxyBorderRound.cy, bRoundClip);
				if (m_isRoundBkImage) {
					roundAlphaClip.ClearClip();
				}

				bool bOldCanvasTrans = m_pWindow->SetRenderTransparent(true);
				CPoint ptOffset(m_rcItem.left + m_renderOffset.x, m_rcItem.top + m_renderOffset.y);
				CPoint ptOldOrg = pCacheRender->OffsetWindowOrg(ptOffset);
				Paint(pCacheRender, m_rcItem);
				PaintChild(pRender, rcPaint);
				pCacheRender->SetWindowOrg(ptOldOrg);
				m_pWindow->SetRenderTransparent(bOldCanvasTrans);
				SetCacheDirty(false);
			}

			pRender->AlphaBlend(rcUnion.left, rcUnion.top, rcUnion.right - rcUnion.left, rcUnion.bottom - rcUnion.top, pCacheRender,
				rcUnion.left - m_rcItem.left, rcUnion.top - m_rcItem.top, rcUnion.right - rcUnion.left, rcUnion.bottom - rcUnion.top, m_nAlpha);

			m_renderContext.reset();
		}
	}
	else if (IsUseCache()) {
		CSize size;
		size.cx = m_rcItem.right - m_rcItem.left;
		size.cy = m_rcItem.bottom - m_rcItem.top;
		auto pCacheRender = GetRenderContext(pRender);
		if (pCacheRender) {
			if (pCacheRender->Resize(size.cx, size.cy)) {
				SetCacheDirty(true);
			}

			if (IsCacheDirty()) {
				pCacheRender->Clear();
				UiRect rcClip = { 0, 0, size.cx, size.cy };
				AutoClip alphaClip(pCacheRender, rcClip, m_bClip);
				AutoClip roundAlphaClip(pCacheRender, rcClip, m_cxyBorderRound.cx, m_cxyBorderRound.cy, bRoundClip);
				if (m_isRoundBkImage) {
					roundAlphaClip.ClearClip();
				}

				bool bOldCanvasTrans = m_pWindow->SetRenderTransparent(true);
				CPoint ptOffset(m_rcItem.left + m_renderOffset.x, m_rcItem.top + m_renderOffset.y);
				CPoint ptOldOrg = pCacheRender->OffsetWindowOrg(ptOffset);
				Paint(pCacheRender, m_rcItem);
				pCacheRender->SetWindowOrg(ptOldOrg);
				m_pWindow->SetRenderTransparent(bOldCanvasTrans);
				SetCacheDirty(false);
			}

			pRender->AlphaBlend(rcUnion.left, rcUnion.top, rcUnion.right - rcUnion.left, rcUnion.bottom - rcUnion.top, pCacheRender,
				rcUnion.left - m_rcItem.left, rcUnion.top - m_rcItem.top, rcUnion.right - rcUnion.left, rcUnion.bottom - rcUnion.top, m_nAlpha);
			PaintChild(pRender, rcPaint);
		}
	}
	else {
		AutoClip clip(pRender, m_rcItem, m_bClip);
		AutoClip roundClip(pRender, m_rcItem, m_cxyBorderRound.cx, m_cxyBorderRound.cy, bRoundClip);
		if (m_isRoundBkImage) {
			roundClip.ClearClip();
		}

		CPoint ptOldOrg = pRender->OffsetWindowOrg(m_renderOffset);
		Paint(pRender, rcPaint);
		PaintChild(pRender, rcPaint);
		pRender->SetWindowOrg(ptOldOrg);
	}
}

void Control::Paint(IRenderContext* pRender, const UiRect& rcPaint)
{
    if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;

	PaintBkColor(pRender);
	PaintBkImage(pRender);
	AfterPaintBK(pRender);
	PaintStatusColor(pRender);
	PaintStatusImage(pRender);
	PaintText(pRender);
	PaintBorder(pRender);
}

void Control::BeginPaint(IRenderContext* pRender)
{
	pRender->BeginPaint(&m_rcItem);
}

void Control::EndPaint(IRenderContext* pRender)
{
	pRender->EndPaint();
}

void Control::PaintBkColor(IRenderContext* pRender)
{
	//画实心圆
	if (m_bkSolidCircle)
	{
		if (m_strBkColor.empty()) {
			return;
		}
		DWORD dwBackColor = GlobalManager::GetTextColor(m_strBkColor);
		dwBackColor &= m_nBkColorAlpha | 0x00FFFFFF;
		UiRect fillRt { m_rcItem.left + 1 , m_rcItem.top + 1, m_rcItem.right - 1, m_rcItem.bottom - 1};
		pRender->FillEllipse(fillRt, dwBackColor);
		return;
	}

	//有渐变绘制渐变
	if (m_bBKGradient)
	{
		if (m_strBkGradientColor1.empty() || m_strBkGradientColor2.empty())
			return;
		pRender->DrawGradientColor(m_rcPaint, m_strBkGradientColor1, m_strBkGradientColor2, m_nBKGradientMode); 
		return;
	}

	if (m_strBkColor.empty()) {
		return;
	}

	DWORD dwBackColor = GlobalManager::GetTextColor(m_strBkColor);
	if (m_isRoundBkImage)
	{
		DWORD dwBorderColor = GlobalManager::GetTextColor(m_strBorderColor);

		if (pRender->UseDirectX())
		{
			DrawRoundDX draw_round;
			int one_pix = DpiManager::GetInstance()->ScaleDouble(1.5);		//预留抗锯齿处理
			draw_round.SetRect(m_rcItem.left, m_rcItem.top, m_rcItem.GetWidth() - one_pix, m_rcItem.GetHeight() - one_pix);
			draw_round.SetLine(D2D1::ColorF(dwBorderColor & 0x00ffffffff, dwBorderColor >> 24), m_nBorderSize, ui::RenderContext_D3D::Translate2D2DPenDashStyle(m_nBorderStyle));
			draw_round.SetArcSize(m_fBorderRound2);
			draw_round.SetOnePix(one_pix);
			draw_round.SetStyle(m_bRoundStyle);
			draw_round.DrawRoundByBkColor(pRender->GetRender(), D2D1::ColorF(dwBackColor & 0x00ffffffff, dwBackColor >> 24));
		}
		else {
			DrawRound draw_round;
			int one_pix = DpiManager::GetInstance()->ScaleDouble(1.5);		//预留抗锯齿处理
			draw_round.SetRect(m_rcItem.left, m_rcItem.top, m_rcItem.GetWidth() - one_pix, m_rcItem.GetHeight() - one_pix);
			draw_round.SetLine(Gdiplus::Color(dwBorderColor), m_nBorderSize, m_nBorderStyle);
			draw_round.SetArcSize(m_fBorderRound2);
			draw_round.SetOnePix(one_pix);
			draw_round.SetStyle(m_bRoundStyle);
			draw_round.DrawRoundByBkColor(pRender->GetDC(), Gdiplus::Color(dwBackColor));
		}
	}
	else
	{
		if (dwBackColor != 0) {
			if (dwBackColor >= 0xFF000000) pRender->DrawColor(m_rcPaint, dwBackColor);
			else pRender->DrawColor(m_rcItem, dwBackColor);
		}

	}
}

void Control::PaintBkImage(IRenderContext* pRender)
{
	DrawImage(pRender, m_bkImage);
}

void Control::AfterPaintBK(IRenderContext* pRender)
{
	if (GetUseWatermark())
	{
		ui::UiRect rcDraw = m_rcItem;
		PaintWatermark(pRender, rcDraw);
	}
}

void Control::PaintStatusColor(IRenderContext* pRender)
{
	m_colorMap.PaintStatusColor(pRender, m_rcPaint, m_uButtonState);
}

void Control::PaintStatusImage(IRenderContext* pRender)
{
	m_imageMap.PaintStatusImage(pRender, kStateImageBk, m_uButtonState);
	m_imageMap.PaintStatusImage(pRender, kStateImageFore, m_uButtonState);
}

void Control::PaintText(IRenderContext* pRender)
{
    return;
}

void Control::PaintBorder(IRenderContext* pRender)
{
	if (m_isRoundBkImage)
		return;

	if (m_strBorderColor.empty()) {
		return;
	}
	DWORD dwBorderColor = 0;
	if (!m_strBorderColor.empty()) {
		dwBorderColor = GlobalManager::GetTextColor(m_strBorderColor);
	}

	if (dwBorderColor != 0) {
		if (m_rcBorderSize.left > 0 || m_rcBorderSize.top > 0 || m_rcBorderSize.right > 0 || m_rcBorderSize.bottom > 0) {
			UiRect rcBorder;
			if (m_rcBorderSize.left > 0) {
				rcBorder = m_rcItem;
				rcBorder.right = rcBorder.left = m_rcItem.left + m_rcBorderSize.left / 2;
				if (m_rcBorderSize.left == 1) {
					rcBorder.bottom -= 1;
				}
				pRender->DrawLine(rcBorder, m_rcBorderSize.left, dwBorderColor);
			}
			if (m_rcBorderSize.top > 0) {
				rcBorder = m_rcItem;
				rcBorder.bottom = rcBorder.top = m_rcItem.top + m_rcBorderSize.top / 2;
				if (m_rcBorderSize.top == 1) {
					rcBorder.right -= 1;
				}
				pRender->DrawLine(rcBorder, m_rcBorderSize.top, dwBorderColor);
			}
			if (m_rcBorderSize.right > 0) {
				rcBorder = m_rcItem;
				rcBorder.left = rcBorder.right = m_rcItem.right - (m_rcBorderSize.right + 1) / 2;
				if (m_rcBorderSize.right == 1) {
					rcBorder.bottom -= 1;
				}
				pRender->DrawLine(rcBorder, m_rcBorderSize.right, dwBorderColor);
			}
			if (m_rcBorderSize.bottom > 0) {
				rcBorder = m_rcItem;
				rcBorder.top = rcBorder.bottom = m_rcItem.bottom - (m_rcBorderSize.bottom + 1) / 2;
				if (m_rcBorderSize.bottom == 1) {
					rcBorder.right -= 1;
				}
				pRender->DrawLine(rcBorder, m_rcBorderSize.bottom, dwBorderColor);
			}
		}
		else if (m_nBorderSize > 0) {
			UiRect rcDraw = m_rcItem;
			int nDeltaValue = m_nBorderSize / 2;
			rcDraw.top += nDeltaValue;
			rcDraw.bottom -= nDeltaValue;
			if (m_nBorderSize % 2 != 0) {
				rcDraw.bottom -= 1;
			}
			rcDraw.left += nDeltaValue;
			rcDraw.right -= nDeltaValue;
			if (m_nBorderSize % 2 != 0) {
				rcDraw.right -= 1;
			}

			if (m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0)
				pRender->DrawRoundRect(rcDraw, m_cxyBorderRound, m_nBorderSize, dwBorderColor);
			else
				pRender->DrawRect(rcDraw, m_nBorderSize, dwBorderColor);
		}
	}
}

void Control::SetAlpha(int alpha)
{
	ASSERT(alpha >= 0 && alpha <= 255);
	m_nAlpha = alpha;
	Invalidate();
}

void Control::SetHotAlpha(int nHotAlpha)
{
	ASSERT(nHotAlpha >= 0 && nHotAlpha <= 255);
	m_nHotAlpha = nHotAlpha;
	Invalidate();
}

void Control::SetRenderOffset(CPoint renderOffset)
{
	m_renderOffset = renderOffset;
	Invalidate();
}

void Control::SetRenderOffsetX(int renderOffsetX)
{
	m_renderOffset.x = renderOffsetX;
	Invalidate();
}

void Control::SetRenderOffsetY(int renderOffsetY)
{
	m_renderOffset.y = renderOffsetY;
	Invalidate();
}

void Control::GifPlay()
{
	if (!m_bkImage.IsValid() || !m_bkImage.imageCache->IsGif() || !m_bkImage.ContinuePlay()) {
		m_bkImage.SetPlaying(false);
		m_gifWeakFlag.Cancel();
		return;
	}

	if (!m_bkImage.IsPlaying()) {
		m_bkImage.SetCurrentFrame(0);
		m_gifWeakFlag.Cancel();
		int lPause = m_bkImage.GetCurrentInterval();
		if (lPause == 0)
			return;
		m_bkImage.SetPlaying(true);
		auto gifPlayCallback = nbase::Bind(&Control::GifPlay, this);
		TimerManager::GetInstance()->AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(), gifPlayCallback,
			lPause, TimerManager::REPEAT_FOREVER);
	}
	else {
		int lPrePause = m_bkImage.GetCurrentInterval();
		m_bkImage.IncrementCurrentFrame();
		int lPause = m_bkImage.GetCurrentInterval();
		if (!m_bkImage.ContinuePlay())
		{
			StopGifPlayForUI(true, kGifStopLast);
		}
		else
		{
			if (lPrePause == 0 || lPause == 0) {//0表示GetCurrentInterval出错
				m_bkImage.SetPlaying(false);
				m_gifWeakFlag.Cancel();
				return;
			}

			if (lPrePause != lPause) {
				m_gifWeakFlag.Cancel();
				auto gifPlayCallback = nbase::Bind(&Control::GifPlay, this);
				TimerManager::GetInstance()->AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(), gifPlayCallback,
					lPause, TimerManager::REPEAT_FOREVER);
			}
		}			
	}
	Invalidate();
}

void Control::StopGifPlay(GifStopType frame)
{
	if (m_bkImage.imageCache && m_bkImage.imageCache->IsGif()) {
		m_bkImage.SetPlaying(false);
		m_gifWeakFlag.Cancel();
		int index = GetGifFrameIndex(frame);
		m_bkImage.SetCurrentFrame(index);
		Invalidate();
	}
}
void Control::SetGifPlay(bool gif_play)
{ 
	m_bGifPlay = gif_play;
	m_bGifPlayEx = gif_play; 
}

void Control::StartGifPlayForUI(GifStopType frame, int playcount)
{
	GetImage(m_bkImage);
	if (!m_bkImage.IsValid() || !m_bkImage.imageCache->IsGif()) {
		m_bGifPlay = false;
		m_bkImage.SetPlaying(false);
		m_gifWeakFlag.Cancel();
		return;
	}
	if (playcount == 0)
	{
		StopGifPlayForUI(false);
	}		
	else
	{
		m_gifWeakFlag.Cancel();
		m_bGifPlay = true;
		m_bkImage.SetCurrentFrame(GetGifFrameIndex(frame));
		int lPause = m_bkImage.GetCurrentInterval();
		if (lPause == 0) {
			m_bGifPlay = false;
			return;
		}
		m_bkImage.SetPlaying(true);
		m_bkImage.imageAttribute.nPlayCount = playcount;
		m_bkImage.ClearCycledCount();
		auto gifPlayCallback = nbase::Bind(&Control::GifPlay, this);
		TimerManager::GetInstance()->AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(), gifPlayCallback,
			lPause, TimerManager::REPEAT_FOREVER);
		Invalidate();
	}	
}

void Control::StopGifPlayForUI(bool transfer, GifStopType frame)
{
	m_bGifPlay = false;
	StopGifPlay(frame);
	if (transfer)
		BroadcastGifEvent(m_nVirtualEventGifStop);
}
int Control::GetGifFrameIndex(GifStopType frame)
{
	int ret = frame;
	switch (frame)
	{
	case kGifStopCurrent:
		ret = m_bkImage.GetCurrentFrameIndex();
		break;
	case kGifStopFirst:
		ret = 0;
		break;
	case kGifStopLast:
	{
		int nFrameCount = m_bkImage.imageCache->GetFrameCount();
		ret = nFrameCount > 0 ? nFrameCount - 1 : 0;		
	}
	break;
	}
	return ret;
}
void Control::BroadcastGifEvent(int nVirtualEvent)
{
	auto callback = OnGifEvent.find(nVirtualEvent);
	if (callback != OnGifEvent.end()) {
		EventArgs param;
		param.pSender = this;
		callback->second(&param);
	}
}

void Control::InvokeLoadImageCache()
{
	if (m_loadBkImageWeakFlag.HasUsed()) {
		return;
	}
	std::wstring sImageName = m_bkImage.imageAttribute.sImageName;
	if (sImageName.empty()) {
		return;
	}
	std::wstring imageFullPath = sImageName;
	if (::PathIsRelative(sImageName.c_str())) {
		imageFullPath = GlobalManager::GetResourcePath() + m_pWindow->GetWindowResourcePath() + sImageName;
	}
	imageFullPath = StringHelper::ReparsePath(imageFullPath);

	if (!m_bkImage.imageCache || m_bkImage.imageCache->sImageFullPath != imageFullPath) {
		auto shared_image = GlobalManager::IsImageCached(imageFullPath);
		if (shared_image) {
			m_bkImage.imageCache = shared_image;
			return;
		}
	}
}

void Control::UnLoadImageCache()
{
	m_loadBkImageWeakFlag.Cancel();
	m_bkImage.ClearCache();
}

void Control::ClearImageCache()
{
	m_imageMap.ClearCache();
	m_bkImage.ClearCache();
}

void Control::DetachEvent(EventType type)
{
	auto event = OnEvent.find(type);
	if (event != OnEvent.end())
	{
		OnEvent.erase(event);
	}
}

void Control::OnWatermarkChange()
{
	Invalidate();
}

} // namespace ui
