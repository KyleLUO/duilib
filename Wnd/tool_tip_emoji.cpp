#include "stdafx.h"
#include "tool_tip_emoji.h"

namespace ui{

	const LPCTSTR ToolTipEmoji::kClassName = L"ToolTipEmoji";

	ToolTipEmoji::ToolTipEmoji()
	{
	}

	ToolTipEmoji::~ToolTipEmoji()
	{
	}

	std::wstring ToolTipEmoji::GetSkinFolder()
	{
		return L"tooltip";
	}

	std::wstring ToolTipEmoji::GetSkinFile()
	{
		return L"tool_tip_emoji.xml";
	}

	std::wstring ToolTipEmoji::GetWindowClassName() const
	{
		return kClassName;
	}


	UINT ToolTipEmoji::GetClassStyle() const
	{
		return UI_CLASSSTYLE_FRAME;
	}

	void ToolTipEmoji::OnFinalMessage(HWND hWnd)
	{
		__super::OnFinalMessage(hWnd);
	}

	LRESULT ToolTipEmoji::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return __super::HandleMessage(uMsg, wParam, lParam);
	}

	LRESULT ToolTipEmoji::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return __super::OnKillFocus(uMsg, wParam, lParam, bHandled);
	}

	void ToolTipEmoji::InitWindow()
	{
		tip_lab_  = dynamic_cast<ui::Label*>(FindControl(L"tip_lab"));
		main_box_ = static_cast<ui::Box*>(FindControl(L"main_box"));

	}

	void  ToolTipEmoji::SetDistance(const int& distance)
	{
		distance_ = DpiManager::GetInstance()->ScaleIntEx(distance);
	}

	void ToolTipEmoji::SetShowMode(const int& show_mode)
	{
		show_mode_ = show_mode;
	}

	void ToolTipEmoji::CreateWnd(HWND parent_wnd)
	{
		parent_wnd_ = parent_wnd;

		DWORD dwStyle = WS_POPUP | ~WS_VISIBLE;
		Create(parent_wnd, L"ToolTip", dwStyle, 0, true);
	}

	void ToolTipEmoji::SetTipText(const std::wstring &tip_txt)
	{
		if (!tip_lab_)
		{
			return;
		}
		tip_lab_->SetText(tip_txt);

		int32_t size =  tip_txt.size();
		if (size == 1)
		{
			main_box_->SetBkImage(L"res/emoji_tip.png");
			main_box_->SetFixedWidth(48);
			main_box_->SetFixedHeight(49);
		}
		else if (size == 2)
		{
			main_box_->SetBkImage(L"res/emoji_tip2.png");
			main_box_->SetFixedWidth(60);
			main_box_->SetFixedHeight(49);
		}
		else
		{
			main_box_->SetBkImage(L"res/emoji_tip3.png");
			main_box_->SetFixedWidth(72);
			main_box_->SetFixedHeight(49);
		}

		ui::UiRect wnd_pos = GetPos();
		wnd_pos.right = wnd_pos.left + main_box_->GetFixedWidth();
		wnd_pos.bottom = wnd_pos.top + main_box_->GetFixedHeight();/*+ tip_size.cy*//* + height*/;

		SetPos(wnd_pos, false, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER, HWND_NOTOPMOST);
	}


	void ToolTipEmoji::SetShowPos(const ui::UiRect& ctrl_pos)
	{
		if (!tip_lab_)
		{
			return;
		}
		ui::UiRect wnd_pos = GetPos();
		int wnd_height = wnd_pos.GetHeight();
		int wnd_width = wnd_pos.GetWidth();
		ui::CPoint tip_pos;
	
		tip_pos.x = ctrl_pos.left + (ctrl_pos.right - ctrl_pos.left) / 2;
		tip_pos.y = ctrl_pos.top;
		
		::ClientToScreen(parent_wnd_, &tip_pos);

		ui::UiRect new_wnd_pos;
		new_wnd_pos.left = tip_pos.x - wnd_width / 2;
		new_wnd_pos.right = new_wnd_pos.left + wnd_width;
		new_wnd_pos.bottom = tip_pos.y - distance_;
		new_wnd_pos.top = new_wnd_pos.bottom - wnd_height;
		

		SetPos(new_wnd_pos, false, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER, HWND_NOTOPMOST);
	}

	void ToolTipEmoji::CloseWnd()
	{
		this->Close();
	}



	bool ToolTipEmoji::ModityStyle(HWND hWnd, DWORD dwRemove, DWORD dwAdd)
	{
		DWORD dwStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
		DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
		if (dwStyle == dwNewStyle)
			return FALSE;

		::SetWindowLong(hWnd, GWL_EXSTYLE, dwNewStyle);
		return TRUE;
	}


}


