#include "StdAfx.h"
#include "base/time/time.h"
#include "base/util/string_util.h"

namespace ui
{

	Watermark::Watermark()
	{
		m_nType = 0;
		m_bUseWatermark = false;
		m_strText = L"null object";
		m_rcMargin_ = { 25, 0, 22, 65 };   //默认下边距

		m_sFontId = L"yahei_12";
		m_strColor = L"color_70dadada"; //默认颜色
		SetWatermarkTextColor_i(m_strColor);
	}

	void Watermark::SetUseWatermark(bool bUseWatermark)
	{
		if (m_bUseWatermark != bUseWatermark)
		{
			SetUseWatermark_i(bUseWatermark);
			OnWatermarkChange();
		}
	}

	bool Watermark::GetUseWatermark()
	{
		return m_bUseWatermark;
	}

	void Watermark::SetWatermarkFont(std::wstring sFontId)
	{
		if (m_sFontId != sFontId)
		{
			SetWatermarkFont_i(sFontId);
			OnWatermarkChange();
		}
	}

	std::wstring Watermark::GetWatermarkFont()
	{
		return m_sFontId;
	}


	void Watermark::SetWatermarkTextColor(const std::wstring& strColor)
	{
		if (m_strColor.compare(strColor) != 0)
		{
			SetWatermarkTextColor_i(strColor);
			OnWatermarkChange();
		}
	}

	void Watermark::SetWatermarkTextColor_i(const std::wstring& strColor)
	{
		m_strColor = strColor;

		m_dwTextColor = GlobalManager::GetTextColor(m_strColor);
	}

	std::wstring Watermark::GetWatermarkTextColor()
	{
		return m_strColor; 
	}

	void Watermark::SetWatermarkTextColor(DWORD dwColor)
	{
		if (m_dwTextColor != dwColor)
		{
			SetWatermarkTextColor_i(dwColor);
			OnWatermarkChange();
		}
	}

	DWORD Watermark::GetDWordWatermarkTextColor()
	{
		return m_dwTextColor; 
	}

	void Watermark::SetWatermarkText(const std::wstring& strText)
	{
		if (m_strText.compare(strText) != 0)
		{
			SetWatermarkText_i(strText);
			OnWatermarkChange();
		}
	}

	std::wstring Watermark::GetWatermarkText()
	{ 
		return m_strText;
	}

	void Watermark::SetWatermarkMargin(const ui::UiRect rcMargin)
	{
		if (!m_rcMargin_.Equal(rcMargin))
		{
			SetWatermarkMargin_i(rcMargin);
			OnWatermarkChange();
		}
	}

	ui::UiRect Watermark::GetWatermarkMargin()
	{
		return m_rcMargin_; 
	}

	void Watermark::SetWatermarkType(int nType)
	{ 
		m_nType = nType;
	}

	int Watermark::GetWatermarkType()
	{ 
		return m_nType;
	}

	void Watermark::SetWatermarkInfo(const IWatermark& info)
	{
		bool update = false;
		if (info.m_sFontId != L"" && m_sFontId != info.m_sFontId)
		{
			update = true;
			SetWatermarkFont_i(info.m_sFontId);
		}
		if (!info.m_rcMargin_.IsRectEmpty() && !m_rcMargin_.Equal(info.m_rcMargin_))
		{
			update = true;
			SetWatermarkMargin_i(info.m_rcMargin_);
		}
		if (!info.m_strColor.empty() && m_strColor.compare(info.m_strColor) != 0)
		{
			update = true;
			SetWatermarkTextColor_i(info.m_strColor);
		}
		if (!info.m_strText.empty() && m_strText.compare(info.m_strText) != 0)//如果清空，设置不展示水印，或调用单独接口
		{
			update = true;
			SetWatermarkText_i(info.m_strText);
		}
		if (info.m_bUseWatermark != m_bUseWatermark)
		{
			update = true;
			SetUseWatermark_i(info.m_bUseWatermark);
		}

		if (update)
		{
			OnWatermarkChange();
		}
	}

	void Watermark::PaintWatermark(IRenderContext* pRender, const ui::UiRect rcDraw)
	{		
		if (m_dwTextColor == 0)
			return;

		ui::UiRect rcDrawWatermark = rcDraw;

		if (pRender->UseDirectX())
		{
			//use direct2d
			int item_height = rcDrawWatermark.GetHeight();
			int watermark_width = DpiManager::GetInstance()->ScaleIntEx(120);
			int watermark_height = DpiManager::GetInstance()->ScaleIntEx(60);
			if (item_height < watermark_height)
			{
				watermark_height = item_height;
			}
			int container_width = watermark_width + m_rcMargin_.left + m_rcMargin_.right;
			int container_height = watermark_height + m_rcMargin_.top + m_rcMargin_.bottom;

			UINT uStyle = DT_SINGLELINE | DT_NOCLIP | DT_LEFT;
			pRender->TranslateTransform(rcDrawWatermark.left, rcDrawWatermark.top);
			pRender->RotateTransform(-30);
			pRender->TranslateTransform(-rcDrawWatermark.GetHeight(), 0);
			//设置转换
			pRender->SetTransform();

			int left_offset = 0, top_offset = 0, last_bottom = 0;
			//保证旋转后尽可能的覆盖更多的区域
			int y_coordinate = rcDrawWatermark.GetWidth() > rcDrawWatermark.GetHeight() ? rcDrawWatermark.GetWidth() : rcDrawWatermark.GetHeight();

			while (true)
			{
				top_offset = m_rcMargin_.top + last_bottom;	//新的一行开始顶部偏移

				while (left_offset <= rcDrawWatermark.GetWidth() + rcDrawWatermark.GetHeight())
				{
					ui::UiRect  rc = { left_offset + m_rcMargin_.left, top_offset, 0, 0 };
					rc.right = rc.left + watermark_width;
					rc.bottom = rc.top + watermark_height;

					left_offset += container_width;

					if (rc.left > rcDrawWatermark.GetWidth() + rcDrawWatermark.GetHeight())
						break;

					int len_dpi_6 = DpiManager::GetInstance()->ScaleIntEx(10);

					ui::UiRect  rcText = rc;
					rcText.bottom -= len_dpi_6;

					pRender->DrawText(rcText, m_strText, m_dwTextColor, m_sFontId, uStyle, 255, true);

					rc.bottom += len_dpi_6;
					//时间  
					time_t now_time_t = nbase::Time::Now().ToTimeT();
					nbase::Time now_time_time = nbase::Time::FromTimeT(now_time_t);
					nbase::Time::TimeStruct msg_time_tm = now_time_time.ToTimeStruct(true);

					std::wstring show_time = nbase::StringPrintf(L"%04d-%02d-%02d %02d:%02d:%02d", msg_time_tm.year(), msg_time_tm.month(), msg_time_tm.day_of_month(), msg_time_tm.hour(), msg_time_tm.minute(), msg_time_tm.second());

					UiRect rcPaint1((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)(rc.right - rc.left), (Gdiplus::REAL)(rc.bottom - rc.top));
					pRender->DrawText(rcPaint1, show_time, m_dwTextColor, m_sFontId, uStyle, 255, true);
				}

				left_offset = 0;
				last_bottom += container_height;	 //结束行的底部位置

				if (last_bottom > y_coordinate)
					break;
			}
			//重置转换
			pRender->ResetTransform();
		}
		else {
			//use gdi+
			Gdiplus::Graphics graphics(pRender->GetDC());
			Gdiplus::Font font(pRender->GetDC(), GlobalManager::GetFont(m_sFontId));

			int alpha = m_dwTextColor >> 24;
			Gdiplus::SolidBrush tBrush(Gdiplus::Color(alpha, GetBValue(m_dwTextColor), GetGValue(m_dwTextColor), GetRValue(m_dwTextColor)));

			Gdiplus::StringFormat stringFormat = Gdiplus::StringFormat::GenericTypographic();
			stringFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoClip | Gdiplus::StringFormatFlagsNoWrap);
			stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
			stringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);

			int item_height = rcDrawWatermark.GetHeight();
			int watermark_width = DpiManager::GetInstance()->ScaleIntEx(120);
			int watermark_height = DpiManager::GetInstance()->ScaleIntEx(60);
			if (item_height < watermark_height)
			{
				watermark_height = item_height;
			}
			int container_width = watermark_width + m_rcMargin_.left + m_rcMargin_.right;
			int container_height = watermark_height + m_rcMargin_.top + m_rcMargin_.bottom;

			int left_offset = 0, top_offset = 0, last_bottom = 0;

			graphics.TranslateTransform(rcDrawWatermark.left, rcDrawWatermark.top);
			graphics.RotateTransform(-30);
			graphics.TranslateTransform(-rcDrawWatermark.GetHeight(), 0);

			//保证旋转后尽可能的覆盖更多的区域
			int y_coordinate = rcDrawWatermark.GetWidth() > rcDrawWatermark.GetHeight() ? rcDrawWatermark.GetWidth() : rcDrawWatermark.GetHeight();

			while (true)
			{
				top_offset = m_rcMargin_.top + last_bottom;	//新的一行开始顶部偏移

				while (left_offset <= rcDrawWatermark.GetWidth() + rcDrawWatermark.GetHeight())
				{
					ui::UiRect  rc = { left_offset + m_rcMargin_.left, top_offset, 0, 0 };
					rc.right = rc.left + watermark_width;
					rc.bottom = rc.top + watermark_height;

					left_offset += container_width;

					if (rc.left > rcDrawWatermark.GetWidth() + rcDrawWatermark.GetHeight())
						break;

					int len_dpi_6 = DpiManager::GetInstance()->ScaleIntEx(10);

					ui::UiRect  rcText = rc;
					rcText.bottom -= len_dpi_6;

					Gdiplus::RectF rcPaint((Gdiplus::REAL)rcText.left, (Gdiplus::REAL)rcText.top, (Gdiplus::REAL)(rcText.GetWidth()), (Gdiplus::REAL)(rcText.GetHeight()));
					graphics.DrawString(m_strText.c_str(), (int)m_strText.length(), &font, rcPaint, &stringFormat, &tBrush);

					rc.bottom += len_dpi_6;
					//时间  
					time_t now_time_t = nbase::Time::Now().ToTimeT();
					nbase::Time now_time_time = nbase::Time::FromTimeT(now_time_t);
					nbase::Time::TimeStruct msg_time_tm = now_time_time.ToTimeStruct(true);

					std::wstring show_time = nbase::StringPrintf(L"%04d-%02d-%02d %02d:%02d:%02d", msg_time_tm.year(), msg_time_tm.month(), msg_time_tm.day_of_month(), msg_time_tm.hour(), msg_time_tm.minute(), msg_time_tm.second());

					Gdiplus::RectF rcPaint1((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)(rc.right - rc.left), (Gdiplus::REAL)(rc.bottom - rc.top));
					graphics.DrawString(show_time.c_str(), (int)show_time.length(), &font, rcPaint1, &stringFormat, &tBrush);
				}

				left_offset = 0;
				last_bottom += container_height;	 //结束行的底部位置

				if (last_bottom > y_coordinate)
					break;
			}
		}
		
	}

}