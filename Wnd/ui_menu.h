#ifndef __UIMENU_H__
#define __UIMENU_H__

#pragma once

//Sagit Code Begin 2019-06-25 14:19:32
#define WM_CLOSEMENU	WM_USER+1000
//Sagit Code End 2019-06-25 14:19:36

namespace ui 
{
/////////////////////////////////////////////////////////////////////////////////////
//

enum MenuAlignment
{
	eMenuAlignment_Left = 1 << 1,
	eMenuAlignment_Top = 1 << 2,
	eMenuAlignment_Right = 1 << 3,
	eMenuAlignment_Bottom = 1 << 4,
};

/////////////////////////////////////////////////////////////////////////////////////
//

extern const TCHAR* const kMenuElementUIInterfaceName;// = _T("MenuElement);
class CMenuElementUI;
class UILIB_API CMenuWnd : public ui::WindowImplBase
{
public:
	CMenuWnd(HWND hParent = NULL);
	virtual ~CMenuWnd();

	enum PopupPosType
	{
		RIGHT_BOTTOM,
		RIGHT_TOP
	};
	void Init(STRINGorID xml, LPCTSTR pSkinType, POINT point, PopupPosType popupPosType = RIGHT_BOTTOM, bool no_focus = false); 
	void Show();

	void SetRoundImage();
protected:
	virtual Control* CreateControl(const std::wstring& pstrClass) override;
	virtual std::wstring GetWindowClassName() const override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;

private:
	HWND m_hParent;
	POINT m_BasedPoint;
	PopupPosType m_popupPosType;
	STRINGorID m_xml;
	std::wstring m_sType;
	bool no_focus_;
};

class ListContainerElement;
class UILIB_API CMenuElementUI : public ListContainerElement
{
	friend CMenuWnd;
public:
    CMenuElementUI();
	~CMenuElementUI();

	virtual bool ButtonUp(EventArgs& msg) override;
};

} // namespace ui

#endif // __UIMENU_H__
