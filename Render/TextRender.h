#pragma once

namespace ui {

	enum class UnderlineType
	{
		None = 0,
		Single = 1,
		Double = 2,
		Triple = 3,
		Squiggly
	};

	enum class BackgroundMode
	{
		TextHeight,
		TextHeightWithLineGap,
		LineHeight
	};

	class CharacterFormatSpecifier : IUnknown
	{
	public:
		// IUnknown methods
		virtual ULONG STDMETHODCALLTYPE AddRef() override;
		virtual ULONG STDMETHODCALLTYPE Release() override;
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
			void **ppvObject) override;

		// Public Set and Get methods
		// --------------------------

		// Foreground Spec font
		static HRESULT SetFont(IDWriteTextLayout * textLayout, IDWriteTextFormat *pFmt, DWRITE_TEXT_RANGE textRange);

		// Foreground brush
		static HRESULT SetForegroundBrush(IDWriteTextLayout * textLayout,
			ID2D1Brush * brush,
			DWRITE_TEXT_RANGE textRange);

		ID2D1Brush * GetForegroundBrush()
		{
			return m_foregroundBrush.Get();
		}

		// Background brush
		static HRESULT SetBackgroundBrush(IDWriteTextLayout * textLayout,
			BackgroundMode backgroundMode,
			ID2D1Brush * brush,
			DWRITE_TEXT_RANGE textRange);

		void GetBackgroundBrush(BackgroundMode * pMode, ID2D1Brush ** pBrush)
		{
			*pMode = m_backgroundMode;
			*pBrush = m_backgroundBrush.Get();
		}

		// Underline
		static HRESULT SetUnderline(IDWriteTextLayout * textLayout,
			UnderlineType type,
			ID2D1Brush * brush,
			DWRITE_TEXT_RANGE textRange);

		void GetUnderline(UnderlineType * pType, ID2D1Brush ** pBrush)
		{
			*pType = m_underlineType;
			*pBrush = m_underlineBrush.Get();
		}

		// Strikethrough
		static HRESULT SetStrikethrough(IDWriteTextLayout * textLayout,
			int count,
			ID2D1Brush * brush,
			DWRITE_TEXT_RANGE textRange);

		void GetStrikethrough(int * pCount, ID2D1Brush ** pBrush)
		{
			*pCount = m_strikethroughCount;
			*pBrush = m_strikethroughBrush.Get();
		}

		// Overline
		static HRESULT SetOverline(IDWriteTextLayout * textLayout,
			bool hasOverline,
			ID2D1Brush * brush,
			DWRITE_TEXT_RANGE textRange);

		void GetOverline(bool * pHasOverline, ID2D1Brush ** pBrush)
		{
			*pHasOverline = m_hasOverline;
			*pBrush = m_overlineBrush.Get();
		}

		// Hightlight
		static HRESULT SetHighlight(IDWriteTextLayout * textLayout,
			ID2D1Brush * brush,
			DWRITE_TEXT_RANGE textRange);

		ID2D1Brush * GetHighlight()
		{
			return m_highlightBrush.Get();
		}


	protected:
		CharacterFormatSpecifier();             // constructor
		CharacterFormatSpecifier * Clone();

		static HRESULT SetFormatting(IDWriteTextLayout * textLayout,
			DWRITE_TEXT_RANGE textRange,
			std::function<void(CharacterFormatSpecifier *)> setField);

	private:
		LONG m_refCount;

		Microsoft::WRL::ComPtr<ID2D1Brush> m_foregroundBrush;

		BackgroundMode                     m_backgroundMode;
		Microsoft::WRL::ComPtr<ID2D1Brush> m_backgroundBrush;

		UnderlineType                      m_underlineType;
		Microsoft::WRL::ComPtr<ID2D1Brush> m_underlineBrush;

		int                                m_strikethroughCount;
		Microsoft::WRL::ComPtr<ID2D1Brush> m_strikethroughBrush;

		bool                               m_hasOverline;
		Microsoft::WRL::ComPtr<ID2D1Brush> m_overlineBrush;

		Microsoft::WRL::ComPtr<ID2D1Brush> m_highlightBrush;
	};

	class CharacterFormatter : public IDWriteTextRenderer
	{
	public:
		CharacterFormatter();

		// Draw method
		HRESULT Draw(ID2D1RenderTarget * renderTarget,
			IDWriteTextLayout * textLayout,
			D2D1_POINT_2F origin,
			ID2D1Brush * defaultBrush);

		// IUnknown methods
		virtual ULONG STDMETHODCALLTYPE AddRef() override;
		virtual ULONG STDMETHODCALLTYPE Release() override;
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;

		// IDWritePixelSnapping methods
		virtual HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(void * clientDrawingContext,
			_Out_ BOOL * isDisabled) override;

		virtual HRESULT STDMETHODCALLTYPE GetPixelsPerDip(void * clientDrawingContext,
			_Out_ FLOAT * pixelsPerDip) override;

		virtual HRESULT STDMETHODCALLTYPE GetCurrentTransform(void * clientDrawingContext,
			_Out_ DWRITE_MATRIX * transform) override;

		// IDWriteTextRenderer methods
		virtual HRESULT STDMETHODCALLTYPE DrawGlyphRun(void * clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			DWRITE_MEASURING_MODE measuringMode,
			_In_ const DWRITE_GLYPH_RUN * glyphRun,
			_In_ const DWRITE_GLYPH_RUN_DESCRIPTION *
			glyphRunDescription,
			IUnknown * clientDrawingEffect) override;

		virtual HRESULT STDMETHODCALLTYPE DrawUnderline(void * clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			_In_ const DWRITE_UNDERLINE * underline,
			IUnknown * clientDrawingEffect) override;

		virtual HRESULT STDMETHODCALLTYPE DrawStrikethrough(void * clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			_In_ const DWRITE_STRIKETHROUGH * strikethrough,
			IUnknown * clientDrawingEffect) override;

		virtual HRESULT STDMETHODCALLTYPE DrawInlineObject(void * clientDrawingContext,
			FLOAT originX,
			FLOAT originY,
			IDWriteInlineObject * inlineObject,
			BOOL isSideways,
			BOOL isRightToLeft,
			IUnknown * clientDrawingEffect) override;

	private:
		LONG m_refCount;

		Microsoft::WRL::ComPtr<ID2D1RenderTarget> m_renderTarget;
		Microsoft::WRL::ComPtr<ID2D1Brush>        m_defaultBrush;

		enum class RenderPass
		{
			Initial,
			Main,
			Final
		};

		RenderPass m_renderPass;

		std::vector<DWRITE_LINE_METRICS> m_lineMetrics;
		int                              m_lineIndex;
		int                              m_charIndex;

		D2D1::Matrix3x2F m_dpiTransform;
		D2D1::Matrix3x2F m_renderTransform;
		D2D1::Matrix3x2F m_worldToPixel;
		D2D1::Matrix3x2F m_pixelToWorld;

		D2D1_RECT_F GetRectangle(const DWRITE_GLYPH_RUN * glyphRun,
			const DWRITE_LINE_METRICS * lineMetrics,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			BackgroundMode backgroundMode);

		void FillRectangle(ID2D1RenderTarget * renderTarget,
			ID2D1Brush * brush,
			float x, float y,
			float width, float thickness,
			int offset);
	};

	class InlineImage : public IDWriteInlineObject {
	public:
		// 构造函数
		InlineImage(ID2D1RenderTarget *pRT, IWICImagingFactory *pImgFactory);
		// 设置图像
		HRESULT SetImage(const std::wstring& resourceRoot, const std::wstring &uri, FLOAT width = -1.0f, FLOAT height=-1.0f);
		HRESULT SetImage(const std::wstring& resourceRoot, const std::wstring &uri, int count, int index);
		// IDWriteInlineObject 接口方法
		STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();
		STDMETHOD(Draw)(
			void *clientDrawingContext,
			IDWriteTextRenderer *renderer,
			FLOAT originX,
			FLOAT originY,
			BOOL isSideways,
			BOOL isRightToLeft,
			IUnknown *clientDrawingEffect
			);
		STDMETHOD(GetMetrics)(DWRITE_INLINE_OBJECT_METRICS *metrics);
		STDMETHOD(GetOverhangMetrics)(DWRITE_OVERHANG_METRICS *overhangs);
		STDMETHOD(GetBreakConditions)(DWRITE_BREAK_CONDITION *breakConditionBefore, DWRITE_BREAK_CONDITION *breakConditionAfter);

	private:
		// 插入图片的相关数据和方法
		Microsoft::WRL::ComPtr<ID2D1Bitmap>  m_imageBitmap;
		Microsoft::WRL::ComPtr<ID2D1RenderTarget>  m_pRT;
		Microsoft::WRL::ComPtr<IWICImagingFactory> m_imgFactory;
		FLOAT m_width;
		FLOAT m_height;

		// 引用计数
		ULONG m_refCount;
	};
}
