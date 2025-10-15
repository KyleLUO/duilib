#pragma once
#include "base/memory/singleton.h"
#ifndef _WND_MANAGER_H
#define _WND_MANAGER_H


namespace ui
{
	class WindowEx;
	class UILIB_API WindowExWeakObject : public std::pair<std::weak_ptr<nbase::WeakFlag>, WindowEx*>
	{
	public:
		WindowExWeakObject operator = (WindowEx* window);
	};
//map<����������map<����id������ָ��>>, ���ͬһ��ֻ��һ�����ڣ�ʹ��������Ϊid
	typedef std::map<std::wstring, std::map<std::wstring, WindowExWeakObject>> WindowsMap;
typedef std::list<WindowEx *> WindowList;
typedef std::list<WindowExWeakObject> WeakWindowList;

#define GET_WINDOW(classType,className)  (classType*)ui::GetWindowsManager(className, className)

UILIB_API  WindowEx* GetWindowsManager(const std::wstring &wnd_class_name, const std::wstring &wnd_id);

UILIB_API  void UnRegisterWindowsManager(const std::wstring &wnd_class_name, const std::wstring &wnd_id);

UILIB_API 	void DestroyAllWindows();;

class UILIB_API WindowsManager
{
public:
	static WindowsManager *GetInstance();
	WindowsManager();
	virtual ~WindowsManager();
	void VibrationWindow(ui::Window* wnd, unsigned rate = 22/*Ƶ�� ms*/, unsigned times = 60/*�񶯴���*/, unsigned amplitude = 8);
	//���ݴ���������idע�ᴰ��
	bool RegisterWindow(const std::wstring wnd_class_name, const std::wstring wnd_id, WindowEx *wnd);
	//���ݴ���������idע������
	void UnRegisterWindow(const std::wstring &wnd_class_name, const std::wstring &wnd_id, WindowEx *wnd);
	//���ݴ���������id��ȡ����
	WindowEx* GetWindow(const std::wstring &wnd_class_name, const std::wstring &wnd_id);
	//��ȡ���д���
	WindowList GetAllWindows();
	//��ȡָ��class��Ӧ�����д���
	WindowList GetWindowsByClassName(LPCTSTR classname);
	//�ر����д���
	void DestroyAllWindows();
	//���ý�ֹ���ڴ���
	void SetStopRegister(bool stop=true){stop_register_ = stop;}
	//�Ƿ��ֹ���ڴ���
	bool IsStopRegister(){return stop_register_;}

	template<typename WindowType>
	static WindowType* SingletonShow(const std::wstring& window_id, HWND hwndFrom = nullptr)
	{
		WindowType *window = (WindowType*)(WindowsManager::GetInstance()->GetWindow(WindowType::kClassName, window_id));
		if (!window)
		{
			window = new WindowType;
			window->Create(NULL, WindowType::kClassName, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
			if (hwndFrom != nullptr)
			{
				window->CenterWindow(hwndFrom, true);
			}
			else
			{
				window->CenterWindow();
			}
			window->ShowWindow();
		}
		else
		{
			window->ActiveWindow();
		}

		return window;
	}
private:
	WeakWindowList GetAllWindowsWithWeak();
private:
	WindowsMap					windows_map_;	//���д���
	std::string					user_id_;
	bool						stop_register_;	//��ֹ���ڴ���
};
}

#endif