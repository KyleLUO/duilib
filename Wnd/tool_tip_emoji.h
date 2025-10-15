#pragma once

namespace ui
{

class UILIB_API ToolTipEmoji : public ui::WindowImplBase
{
public:
	ToolTipEmoji();
	~ToolTipEmoji();
	static const LPCTSTR kClassName;
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;
	virtual UINT GetClassStyle() const override;
	virtual void OnFinalMessage(HWND hWnd);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual void InitWindow();
public:
	void CreateWnd(HWND parent_wnd);
	//模式:0左 1上 2右 3下 默认-1不带箭头
	void SetDistance(const int& distance);
	void SetShowMode(const int& show_mode);
	void SetTipText(const std::wstring &tip_txt);
	void SetShowPos(const ui::UiRect& ctrl_pos);

	void CloseWnd();

private:
	bool ModityStyle(HWND hWnd, DWORD dwRemove, DWORD dwAdd);

private:
	ui::Label*		tip_lab_ = nullptr;
	ui::Box*		main_box_ = nullptr;

private:
	int  show_mode_		= -1;
	int  distance_		= 0;
	HWND parent_wnd_	= nullptr;
};

}