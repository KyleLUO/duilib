#include "stdafx.h"
#include "windows_manager.h"
#include "base/log/ky_log.h"
#include "base/thread/thread_manager.h"
#include "threads_define.h"


namespace ui
{

WindowEx* GetWindowsManager(const std::wstring &wnd_class_name, const std::wstring &wnd_id)
{
	return ui::WindowsManager::GetInstance()->GetWindow(wnd_class_name, wnd_id);
}


void UnRegisterWindowsManager(const std::wstring &wnd_class_name, const std::wstring &wnd_id)
{
	return ui::WindowsManager::GetInstance()->UnRegisterWindow(wnd_class_name, wnd_id,nullptr);
}

void DestroyAllWindows()
{
	ui::WindowsManager::GetInstance()->DestroyAllWindows();
}

WindowsManager * WindowsManager::GetInstance()
{
	static WindowsManager wnd_mgr;
	return &wnd_mgr;
}



WindowsManager::WindowsManager()
{
	stop_register_ = false;
	windows_map_.clear();
}

WindowsManager::~WindowsManager()
{
	windows_map_.clear();
}

bool WindowsManager::RegisterWindow(const std::wstring wnd_class_name, const std::wstring wnd_id, WindowEx *wnd)
{
	if (IsStopRegister())
	{
		assert(!stop_register_);
		return false;
	}
	WindowsMap::iterator it = windows_map_.find(wnd_class_name);
	if (it != windows_map_.end())
	{
		auto it2 = it->second.find(wnd_id);
		if (it2 != it->second.end())
		{
			KY_QLOG_PRO(L"The window <class name: {0}, id: {1}> has already registered !")<<wnd_class_name <<wnd_id;
		}
		it->second[wnd_id] = wnd;
	}
	else
	{
		std::map<std::wstring, WindowExWeakObject> id_win;
		id_win[wnd_id] = wnd;
		windows_map_[wnd_class_name] = id_win;
	}
	return true;
}

void WindowsManager::UnRegisterWindow(const std::wstring &wnd_class_name, const std::wstring &wnd_id, WindowEx *wnd)
{
	WindowsMap::iterator it = windows_map_.find(wnd_class_name);
	if (it != windows_map_.end())
	{
		auto it2 = it->second.find(wnd_id);
		if (it2 != it->second.end())
		{
			it->second.erase(it2);
		}
	}
}
	
WindowList WindowsManager::GetAllWindows()
{
	WindowList list;
	WindowsMap::iterator it = windows_map_.begin();
	for ( ; it != windows_map_.end(); ++it)
	{
		auto it2 = it->second.begin();
		for ( ; it2 != it->second.end(); ++it2)
		{
			
			if (!it2->second.first.expired())
			{
				WindowEx *wnd = (WindowEx *)(it2->second.second);
				if (wnd && ::IsWindow(wnd->GetHWND()))
				{
					list.push_back(wnd);
				}
			}
			else
			{
				it2 = it->second.erase(it2);
				continue;
			}
			
		}
	}
	return list;
}
WeakWindowList WindowsManager::GetAllWindowsWithWeak()
{
	WeakWindowList list;
	WindowsMap::iterator it = windows_map_.begin();
	for (; it != windows_map_.end(); ++it)
	{
		auto it2 = it->second.begin();
		for (; it2 != it->second.end(); ++it2)
		{

			if (!it2->second.first.expired())
			{
				WindowEx *wnd = (WindowEx *)(it2->second.second);
				if (wnd && ::IsWindow(wnd->GetHWND()))
				{
					list.push_back(it2->second);
				}
			}
			else
			{
				it2 = it->second.erase(it2);
				continue;
			}

		}
	}
	return list;
}
WindowEx* WindowsManager::GetWindow(const std::wstring &wnd_class_name, const std::wstring &wnd_id)
{
	WindowEx* ret = nullptr;
	WindowsMap::iterator it = windows_map_.find(wnd_class_name);
	if(it != windows_map_.end())
	{
		auto it2 = it->second.find(wnd_id);
		if (it2 != it->second.end())
		{
			if (!it2->second.first.expired())
			{
				WindowEx* wnd = (WindowEx*)(it2->second.second);
				if (wnd && ::IsWindow(wnd->GetHWND()))
					ret = wnd;
				else
					it->second.erase(it2);
			}
			else
			{
				it->second.erase(it2);
			}			
		}
	}
	return ret;
}

WindowList WindowsManager::GetWindowsByClassName(LPCTSTR classname)
{
	WindowList wnd_list;
	WindowsMap::iterator it = windows_map_.find(classname);
	if(it != windows_map_.end())
	{
		auto it2 = it->second.begin();
		for(; it2 != it->second.end(); it2++)
		{
			if (!it2->second.first.expired())
			{
				WindowEx* wnd = (WindowEx*)(it2->second.second);
				if (wnd && ::IsWindow(wnd->GetHWND()))
				{
					wnd_list.push_back(wnd);
				}
				else
				{
					it2 = it->second.erase(it2);
					continue;
				}
			}
			else
			{
				it2 = it->second.erase(it2);
				continue;
			}
		}
	}
	return wnd_list;
}

void WindowsManager::DestroyAllWindows()
{
	SetStopRegister();

	auto lst_wnd = GetAllWindowsWithWeak();
	auto it = lst_wnd.begin();
	for ( ; it != lst_wnd.end(); ++it)
	{
		if (!it->first.expired())
		{
			WindowEx *wnd = (WindowEx *)it->second;
			if (wnd && ::IsWindow(wnd->GetHWND()))
			{
				::DestroyWindow(wnd->GetHWND());
			}
		}
	}
	windows_map_.clear();
}
void WindowsManager::VibrationWindow(ui::Window* wnd, unsigned rate , unsigned times, unsigned amplitude)
{
	auto task = wnd->ToWeakCallback([wnd, rate, times, amplitude](){
		HWND hwnd = wnd->GetHWND();		
		RECT rc;
		::GetWindowRect(hwnd, &rc);		
		::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		::SetForegroundWindow(hwnd);
		::BringWindowToTop(hwnd);
		::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		::SetActiveWindow(hwnd);
		nbase::ThreadManager::PostRepeatedTask(ThreadIdDefine::kThreadUI, wnd->ToWeakCallback([hwnd, rc, amplitude, times, rate](){
			RECT rctemp = rc;
			int i = (((unsigned)rand()) % amplitude) * ((((unsigned)rand()) % 2) == 1 ? -1 : 1);
			int j = (((unsigned)rand()) % amplitude) * ((((unsigned)rand()) % 2) == 1 ? -1 : 1);
			rctemp.left -= i;
			rctemp.right -= i;
			rctemp.top -= j;
			rctemp.bottom -= j;
			::MoveWindow(hwnd, rctemp.left, rctemp.top, rctemp.right - rctemp.left, rctemp.bottom - rctemp.top, TRUE);
		}), nbase::TimeDelta::FromMilliseconds(rate), times);
	});
	HWND hwnd = wnd->GetHWND();
	BOOL isicon = ::IsIconic(hwnd);
	BOOL isvisible = IsWindowVisible(hwnd);
	if (!isvisible || isicon)
	{
		::ShowWindow(hwnd, SW_SHOWNA);
		::SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		nbase::ThreadManager::PostDelayedTask(ThreadIdDefine::kThreadUI, task, nbase::TimeDelta::FromMilliseconds(300));
	}	
	else
	{
		task();
	}
}

ui::WindowExWeakObject WindowExWeakObject::operator=(WindowEx* window)
{
	this->first = window->GetWeakFlag();
	this->second = window;
	return *this;
}

}