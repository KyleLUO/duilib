#include "stdafx.h"
#include "tool_tip_ctrl.h"

namespace ui{

	const LPCTSTR ToolTipCtrl::kClassName = L"ToolTipCtrl";

	ToolTipCtrl::ToolTipCtrl()
	{
	}

	ToolTipCtrl::~ToolTipCtrl()
	{
	}

	std::wstring ToolTipCtrl::GetSkinFolder()
	{
		return L"tooltip";
	}

	std::wstring ToolTipCtrl::GetSkinFile()
	{
		return L"tool_tip_ctrl.xml";
	}

	std::wstring ToolTipCtrl::GetWindowClassName() const
	{
		return kClassName;
	}


	UINT ToolTipCtrl::GetClassStyle() const
	{
		return UI_CLASSSTYLE_FRAME;
	}

	void ToolTipCtrl::OnFinalMessage(HWND hWnd)
	{
		__super::OnFinalMessage(hWnd);
	}

	LRESULT ToolTipCtrl::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return __super::HandleMessage(uMsg, wParam, lParam);
	}

	LRESULT ToolTipCtrl::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return __super::OnKillFocus(uMsg, wParam, lParam, bHandled);
	}

	void ToolTipCtrl::InitWindow()
	{
		tip_lab_  = dynamic_cast<ui::Label*>(FindControl(L"tip_lab"));
		main_box_ = static_cast<ui::Box*>(FindControl(L"main_box"));

		ctl_tooltip_left_  = static_cast<ui::Control*>(FindControl(L"ctl_tooltip_left"));
		ctl_tooltip_top_   = static_cast<ui::Control*>(FindControl(L"ctl_tooltip_top"));
		ctl_tooltip_right_ = static_cast<ui::Control*>(FindControl(L"ctl_tooltip_right"));
		ctl_tooltip_bottom_ = static_cast<ui::Control*>(FindControl(L"ctl_tooltip_bottom"));
	}

	void  ToolTipCtrl::SetDistance(const int& distance)
	{
		if (distance > 0)
		{
			distance_ = DpiManager::GetInstance()->ScaleIntEx(distance);
		}
		else
		{
			distance_ = 0;
		}
	}

	void ToolTipCtrl::SetShowMode(const int& show_mode)
	{
		show_mode_ = show_mode;
		if (0 == show_mode_)
		{
			ctl_tooltip_left_->SetVisible(true);
			ctl_tooltip_top_->SetVisible(false);
			ctl_tooltip_right_->SetVisible(false);
			ctl_tooltip_bottom_->SetVisible(false);
		}
		else if (1 == show_mode_)
		{
			ctl_tooltip_left_->SetVisible(false);
			ctl_tooltip_top_->SetVisible(true);
			ctl_tooltip_right_->SetVisible(false);
			ctl_tooltip_bottom_->SetVisible(false);
		}
		else if (2 == show_mode_)
		{
			ctl_tooltip_left_->SetVisible(false);
			ctl_tooltip_top_->SetVisible(false);
			ctl_tooltip_right_->SetVisible(true);
			ctl_tooltip_bottom_->SetVisible(false);
		}
		else if (3 == show_mode_)
		{
			ctl_tooltip_left_->SetVisible(false);
			ctl_tooltip_top_->SetVisible(false);
			ctl_tooltip_right_->SetVisible(false);
			ctl_tooltip_bottom_->SetVisible(true);
		}
		else
		{
			ctl_tooltip_left_->SetVisible(false);
			ctl_tooltip_top_->SetVisible(false);
			ctl_tooltip_right_->SetVisible(false);
			ctl_tooltip_bottom_->SetVisible(false);
		}
	}

	void ToolTipCtrl::CreateWnd(HWND parent_wnd)
	{
		parent_wnd_ = parent_wnd;

		DWORD dwStyle = WS_POPUP | ~WS_VISIBLE;
		Create(parent_wnd, L"ToolTip", dwStyle, 0, true);
	}

	void ToolTipCtrl::SetTipText(const std::wstring &tip_txt)
	{
		if (!tip_lab_)
		{
			return;
		}
		tip_lab_->SetText(tip_txt);

		bool re_estimate_size = false; 
		ui::CSize tip_size = { 0, 0 };
		tip_size = tip_lab_->EstimateText(CSize(0, 0), re_estimate_size);
		if (!tip_size.cy)
		{
			tip_size.cy = tip_lab_->GetFixedHeight();
		}
		int width = 0;
		int height = 0;
		if (0 == show_mode_ || 2 == show_mode_)
		{
			width = DpiManager::GetInstance()->ScaleIntEx(9);
		}
		else if (1 == show_mode_ || 3 == show_mode_)
		{
			height = DpiManager::GetInstance()->ScaleIntEx(9);
		}
		int main_width = tip_size.cx + DpiManager::GetInstance()->ScaleIntEx(16) + DpiManager::GetInstance()->ScaleIntEx(2);
		ui::UiRect wnd_pos = GetPos();
		wnd_pos.right = wnd_pos.left + main_width + width;
		wnd_pos.bottom = wnd_pos.top + tip_size.cy + height;

		main_box_->SetFixedWidth(main_width, true, false);

		SetPos(wnd_pos, false, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER, HWND_NOTOPMOST);
	}


	void ToolTipCtrl::SetShowPos(const ui::UiRect& ctrl_pos)
	{
		if (!tip_lab_)
		{
			return;
		}
		ui::UiRect wnd_pos = GetPos();
		int wnd_height = wnd_pos.GetHeight();
		int wnd_width = wnd_pos.GetWidth();
		ui::CPoint tip_pos;
		if (0 == show_mode_)
		{
			tip_pos.x = ctrl_pos.left + ctrl_pos.GetWidth();
			tip_pos.y = ctrl_pos.top + (ctrl_pos.bottom - ctrl_pos.top) / 2;
		}
		else if (1 == show_mode_)
		{
			tip_pos.x = ctrl_pos.left + (ctrl_pos.right - ctrl_pos.left) / 2 ;
			tip_pos.y = ctrl_pos.bottom;
		}
		else if (2 == show_mode_)
		{
			tip_pos.x = ctrl_pos.left;
			tip_pos.y = ctrl_pos.top + (ctrl_pos.bottom - ctrl_pos.top) / 2;
		}
		else if (3 == show_mode_)
		{
			tip_pos.x = ctrl_pos.left + (ctrl_pos.right - ctrl_pos.left) / 2;
			tip_pos.y = ctrl_pos.top ;
		}
		else
		{
			tip_pos.x = ctrl_pos.left + (ctrl_pos.right - ctrl_pos.left) / 2;
			tip_pos.y = ctrl_pos.bottom;
		}
		::ClientToScreen(parent_wnd_, &tip_pos);

	
		ui::UiRect new_wnd_pos;
		if (0 == show_mode_)
		{
			new_wnd_pos.left = tip_pos.x + distance_;
			new_wnd_pos.right = new_wnd_pos.left + wnd_width;
			new_wnd_pos.top = tip_pos.y - wnd_height / 2;
			new_wnd_pos.bottom = new_wnd_pos.top + wnd_height;
		}
		else if (1 == show_mode_)
		{
			new_wnd_pos.left = tip_pos.x - wnd_width / 2;
			new_wnd_pos.right = new_wnd_pos.left + wnd_width;
			new_wnd_pos.top = tip_pos.y + distance_;
			new_wnd_pos.bottom = new_wnd_pos.top + wnd_height;
		}
		else if (2 == show_mode_)
		{
			new_wnd_pos.right = tip_pos.x - distance_;
			new_wnd_pos.left = new_wnd_pos.right - wnd_width;
			new_wnd_pos.top = tip_pos.y - wnd_height / 2;
			new_wnd_pos.bottom = new_wnd_pos.top + wnd_height;
		}
		else if (3 == show_mode_)
		{
			new_wnd_pos.left = tip_pos.x - wnd_width / 2;
			new_wnd_pos.right = new_wnd_pos.left + wnd_width;
			new_wnd_pos.bottom = tip_pos.y - distance_;
			new_wnd_pos.top = new_wnd_pos.bottom - wnd_height;
		}
		else
		{
			new_wnd_pos.left = tip_pos.x - wnd_width / 2;
			new_wnd_pos.right = new_wnd_pos.left + wnd_width;
			new_wnd_pos.top = tip_pos.y + distance_;
			new_wnd_pos.bottom = new_wnd_pos.top + wnd_height;
		}

		SetPos(new_wnd_pos, false, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER, HWND_NOTOPMOST);
	}

	void ToolTipCtrl::CloseWnd()
	{
		this->Close();
	}



	bool ToolTipCtrl::ModityStyle(HWND hWnd, DWORD dwRemove, DWORD dwAdd)
	{
		DWORD dwStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
		DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
		if (dwStyle == dwNewStyle)
			return FALSE;

		::SetWindowLong(hWnd, GWL_EXSTYLE, dwNewStyle);
		return TRUE;
	}


}


