#ifndef UI_WATERMARK_H_
#define UI_WATERMARK_H_

#pragma once

namespace ui
{
	class UILIB_API IWatermark
	{
	public:
		IWatermark() : m_rcMargin_(0, 0, 0, 0){};
		virtual ~IWatermark() {}
	public:
		std::wstring	m_sFontId = L"";
		std::wstring	m_strText = L"";
		std::wstring	m_strColor = L"";
		ui::UiRect		m_rcMargin_;
		bool			m_bUseWatermark = false;
		int				m_nType = 0;
	};

	class UILIB_API Watermark : protected IWatermark
	{
	public:
		Watermark();
		virtual ~Watermark(){}
	public:
		//设置是否使用水印
		void SetUseWatermark(bool bUseWatermark);
		//获取是否使用水印
		bool GetUseWatermark();
		//设置水印字体大小
		void SetWatermarkFont(std::wstring sFontId);
		//获取水印字体大小
		std::wstring GetWatermarkFont();
		//设置水印字体颜色
		void SetWatermarkTextColor(const std::wstring& strColor);
		//获取水印字体颜色
		std::wstring GetWatermarkTextColor();
		//设置水印字体颜色
		void SetWatermarkTextColor(DWORD dwColor);
		//获取水印字体颜色
		DWORD GetDWordWatermarkTextColor();
		//设置水印内容
		void SetWatermarkText(const std::wstring& strText);
		//获取水印内容
		std::wstring GetWatermarkText();
		//设置水印外边距
		void SetWatermarkMargin(const ui::UiRect rcMargin);
		//获取水印外边距
		ui::UiRect GetWatermarkMargin();
		//设置水印类型
		void SetWatermarkType(int nType);
		//获取水印类型
		int  GetWatermarkType();
		//设置水印信息
		void SetWatermarkInfo(const IWatermark& info);

		//画水印
		void PaintWatermark(IRenderContext* pRender, const ui::UiRect rcDraw);

	protected:
		virtual void OnWatermarkChange() = 0;
		virtual void SetUseWatermark_i(bool bUse){ m_bUseWatermark = bUse; }
		virtual void SetWatermarkFont_i(std::wstring sFontId){ m_sFontId = sFontId; }
		virtual void SetWatermarkTextColor_i(const std::wstring& strColor);
		virtual void SetWatermarkTextColor_i(DWORD dwColor){ m_dwTextColor = dwColor; }
		virtual void SetWatermarkText_i(const std::wstring& strText){ m_strText = strText; }
		virtual void SetWatermarkMargin_i(const ui::UiRect rcMargin){ m_rcMargin_ = rcMargin; }
	private:
		DWORD m_dwTextColor = 0;
	};
}
#endif // UI_CORE_PLACE_HOLDER_H_
