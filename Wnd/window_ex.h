#pragma once

#define FIND_SUB_CONTROL(class_name,control_name) dynamic_cast<class_name*>(FindSubControl(control_name));
#define FIND_CONTROL(class_name,control_name) dynamic_cast<class_name*>(FindControl(control_name));

namespace ui
{

/** @class ShadowWnd
* @brief 一个附加到基础窗口周围的窗口，带有 WS_EX_LAYERED 属性来实现阴影
* @copyright (c) 2016, NetEase Inc. All rights reserved
* @date 2019-03-22
*/
class UILIB_API ShadowWnd : public ui::WindowImplBase
{
public:
	ShadowWnd();

	void SetShadowCorner(const ui::UiRect &rect);
	void SetShadowImage(const std::wstring &image);
	void SetRoundCorner(int cx, int cy);
	void ResetShadowBox();

	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;
	virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

	virtual HWND Create(Window* window);
private:
	ui::Window*	window_ = nullptr;
	bool   first_show_window_ = false;
	nbase::WeakCallbackFlag delay_timer_;
};

//////////////////////////////////////////////////////////////////////////
class UILIB_API CShadowUI
{
public:
	friend class CPaintManagerUI;

	CShadowUI(void);
	virtual ~CShadowUI(void);

public:
	// bShow为真时才会创建阴影
	void ShowShadow(bool bShow);
	bool IsShowShadow() const;

	void DisableShadow(bool bDisable);
	bool IsDisableShadow() const;

	// 算法阴影的函数
	bool SetSize(int NewSize = 0);
	bool SetSharpness(unsigned int NewSharpness = 5);
	bool SetDarkness(unsigned int NewDarkness = 200);
	bool SetPosition(int NewXOffset = 5, int NewYOffset = 5);
	bool SetColor(COLORREF NewColor = 0);

	// 图片阴影的函数
	bool SetImage(LPCTSTR szImage);
	bool SetShadowCorner(RECT rcCorner);	// 九宫格方式描述阴影

	// 把自己的阴影样式复制到传入参数
	bool CopyShadow(CShadowUI* pShadow);

	//	创建阴影窗体，由CPaintManagerUI自动调用,除非自己要单独创建阴影
	void Create(HWND hParentWnd);

	void SetHideShadow(bool hide = true);
protected:

	//	初始化并注册阴影类
	static bool Initialize(HINSTANCE hInstance);

	// 保存已经附加的窗体句柄和与其关联的阴影类,方便在ParentProc()函数中通过句柄得到阴影类
	static std::map<HWND, CShadowUI *>& GetShadowMap();

	//	子类化父窗体
	static LRESULT CALLBACK ParentProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 父窗体改变大小，移动，或者主动重绘阴影时调用
	void Update(HWND hParent);

	// 通过算法计算阴影
	void MakeShadow(UINT32 *pShadBits, HWND hParent, RECT *rcParent);

	// 计算alpha预乘值
	inline DWORD PreMultiply(COLORREF cl, unsigned char nAlpha)
	{
		return (GetRValue(cl) * (DWORD)nAlpha / 255) |
			(GetGValue(cl) * (DWORD)nAlpha / 255) << 8 |
			(GetBValue(cl) * (DWORD)nAlpha / 255) << 16;
	}

protected:
	enum ShadowStatus
	{
		SS_ENABLED = 1,				// Shadow is enabled, if not, the following one is always false
		SS_VISABLE = 1 << 1,		// Shadow window is visible
		SS_PARENTVISIBLE = 1 << 2	// Parent window is visible, if not, the above one is always false
	};


	static bool s_bHasInit;

	HWND			 m_hWnd;			// 阴影窗体的句柄
	LONG_PTR		 m_OriParentProc;	// 子类化父窗体
	BYTE			 m_Status;
	bool			 m_bIsImageMode;	// 是否为图片阴影模式
	bool			 m_bIsShowShadow;	// 是否要显示阴影
	bool			m_bIsDisableShadow;
	// 算法阴影成员变量
	unsigned char m_nDarkness;	// Darkness, transparency of blurred area
	unsigned char m_nSharpness;	// Sharpness, width of blurred border of shadow window
	signed char m_nSize;	// Shadow window size, relative to parent window size

	// The X and Y offsets of shadow window,
	// relative to the parent window, at center of both windows (not top-left corner), signed
	signed char m_nxOffset;
	signed char m_nyOffset;

	// Restore last parent window size, used to determine the update strategy when parent window is resized
	LPARAM m_WndSize;

	// Set this to true if the shadow should not be update until next WM_PAINT is received
	bool m_bUpdate;

	COLORREF m_Color;	// Color of shadow

	// 图片阴影成员变量
	std::wstring	m_sShadowImage;
	RECT		m_rcShadowCorner;
};

//////////////////////////////////////////////////////////////////////////
class UILIB_API WindowEx : public ui::WindowImplBase
{
public:
	WindowEx(const wchar_t *bk_path= L"file='../../global/res/bk/bk_shadow3.png' corner='15,15,15,15'");
	virtual ~WindowEx();

	/**
	* 创建窗口
	* @param[in] hwndParent 父窗口句柄
	* @param[in] pstrName 窗口名称
	* @param[in] dwStyle 窗口样式
	* @param[in] dwExStyle 窗口扩展样式
	* @param[in] isLayeredWindow 是否创建分层窗口
	* @param[in] rc 窗口位置
	* @return HWND 窗口句柄
	*/
	virtual HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, 
		bool isLayeredWindow = true, const ui::UiRect& rc = ui::UiRect(0, 0, 0, 0)) override;
	
	/**
	* 处理窗口被销毁的消息
	* @param[in] uMsg 消息
	* @param[in] wParam 参数
	* @param[in] lParam 参数
	* @param[out] bHandled 消息是否被处理
	* @return LRESULT 处理结果
	*/
	virtual LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/**
	* 处理ESC键单击的消息
	* @param[out] bHandled 消息是否被处理
	* @return void 无返回值
	*/
	virtual void OnEsc(BOOL &bHandled);
	
	/**
	* 获取窗口类名的接口
	* @return wstring 窗口类名
	*/
	virtual std::wstring GetWindowClassName(void) const = 0;

	/**
	* 获取窗口id的接口
	* @return wstring 窗口id
	*/
	virtual std::wstring GetWindowId(void) const = 0;

	/**
	* 处理窗口消息
	* @param[in] uMsg 消息
	* @param[in] wParam 参数
	* @param[in] lParam 参数
	* @return LRESULT 处理结果
	*/
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 计算窗口右下角位置
	ui::UiRect CalculateWindowRBPos();

	void SetHideShadowUI(bool hide = true);
private:
	/**
	* 从WindowManager中注册自己
	* @return bool true 注册成功，false 注册失败
	*/
	bool RegisterWnd();

	/**
	* 从WindowManager中反注册自己
	* @return void 无返回值
	*/
	void UnRegisterWnd();

protected:
	ShadowWnd* shadow_wnd_ = nullptr;

	CShadowUI m_shadow;
};
 
/**
* 获取弹出窗口可以显示在右下角位置的坐标
* @return POINT 窗口坐标
*/
UILIB_API POINT GetPopupWindowPos(WindowEx* window);

enum SystemBar_Pos
{
	TOP = 1,//
	BOTTOM,
	LEFT,
	RIGHT
};

UILIB_API void GetSystemBarPos(RECT& rc, SystemBar_Pos& pos);

}