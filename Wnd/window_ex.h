#pragma once

#define FIND_SUB_CONTROL(class_name,control_name) dynamic_cast<class_name*>(FindSubControl(control_name));
#define FIND_CONTROL(class_name,control_name) dynamic_cast<class_name*>(FindControl(control_name));

namespace ui
{

/** @class ShadowWnd
* @brief һ�����ӵ�����������Χ�Ĵ��ڣ����� WS_EX_LAYERED ������ʵ����Ӱ
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
	// bShowΪ��ʱ�Żᴴ����Ӱ
	void ShowShadow(bool bShow);
	bool IsShowShadow() const;

	void DisableShadow(bool bDisable);
	bool IsDisableShadow() const;

	// �㷨��Ӱ�ĺ���
	bool SetSize(int NewSize = 0);
	bool SetSharpness(unsigned int NewSharpness = 5);
	bool SetDarkness(unsigned int NewDarkness = 200);
	bool SetPosition(int NewXOffset = 5, int NewYOffset = 5);
	bool SetColor(COLORREF NewColor = 0);

	// ͼƬ��Ӱ�ĺ���
	bool SetImage(LPCTSTR szImage);
	bool SetShadowCorner(RECT rcCorner);	// �Ź���ʽ������Ӱ

	// ���Լ�����Ӱ��ʽ���Ƶ��������
	bool CopyShadow(CShadowUI* pShadow);

	//	������Ӱ���壬��CPaintManagerUI�Զ�����,�����Լ�Ҫ����������Ӱ
	void Create(HWND hParentWnd);

	void SetHideShadow(bool hide = true);
protected:

	//	��ʼ����ע����Ӱ��
	static bool Initialize(HINSTANCE hInstance);

	// �����Ѿ����ӵĴ������������������Ӱ��,������ParentProc()������ͨ������õ���Ӱ��
	static std::map<HWND, CShadowUI *>& GetShadowMap();

	//	���໯������
	static LRESULT CALLBACK ParentProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// ������ı��С���ƶ������������ػ���Ӱʱ����
	void Update(HWND hParent);

	// ͨ���㷨������Ӱ
	void MakeShadow(UINT32 *pShadBits, HWND hParent, RECT *rcParent);

	// ����alphaԤ��ֵ
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

	HWND			 m_hWnd;			// ��Ӱ����ľ��
	LONG_PTR		 m_OriParentProc;	// ���໯������
	BYTE			 m_Status;
	bool			 m_bIsImageMode;	// �Ƿ�ΪͼƬ��Ӱģʽ
	bool			 m_bIsShowShadow;	// �Ƿ�Ҫ��ʾ��Ӱ
	bool			m_bIsDisableShadow;
	// �㷨��Ӱ��Ա����
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

	// ͼƬ��Ӱ��Ա����
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
	* ��������
	* @param[in] hwndParent �����ھ��
	* @param[in] pstrName ��������
	* @param[in] dwStyle ������ʽ
	* @param[in] dwExStyle ������չ��ʽ
	* @param[in] isLayeredWindow �Ƿ񴴽��ֲ㴰��
	* @param[in] rc ����λ��
	* @return HWND ���ھ��
	*/
	virtual HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, 
		bool isLayeredWindow = true, const ui::UiRect& rc = ui::UiRect(0, 0, 0, 0)) override;
	
	/**
	* �����ڱ����ٵ���Ϣ
	* @param[in] uMsg ��Ϣ
	* @param[in] wParam ����
	* @param[in] lParam ����
	* @param[out] bHandled ��Ϣ�Ƿ񱻴���
	* @return LRESULT ������
	*/
	virtual LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/**
	* ����ESC����������Ϣ
	* @param[out] bHandled ��Ϣ�Ƿ񱻴���
	* @return void �޷���ֵ
	*/
	virtual void OnEsc(BOOL &bHandled);
	
	/**
	* ��ȡ���������Ľӿ�
	* @return wstring ��������
	*/
	virtual std::wstring GetWindowClassName(void) const = 0;

	/**
	* ��ȡ����id�Ľӿ�
	* @return wstring ����id
	*/
	virtual std::wstring GetWindowId(void) const = 0;

	/**
	* ��������Ϣ
	* @param[in] uMsg ��Ϣ
	* @param[in] wParam ����
	* @param[in] lParam ����
	* @return LRESULT ������
	*/
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	// ���㴰�����½�λ��
	ui::UiRect CalculateWindowRBPos();

	void SetHideShadowUI(bool hide = true);
private:
	/**
	* ��WindowManager��ע���Լ�
	* @return bool true ע��ɹ���false ע��ʧ��
	*/
	bool RegisterWnd();

	/**
	* ��WindowManager�з�ע���Լ�
	* @return void �޷���ֵ
	*/
	void UnRegisterWnd();

protected:
	ShadowWnd* shadow_wnd_ = nullptr;

	CShadowUI m_shadow;
};
 
/**
* ��ȡ�������ڿ�����ʾ�����½�λ�õ�����
* @return POINT ��������
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