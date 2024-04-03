#include "IdtWindow.h"

#include "IdtDraw.h"
#include "IdtDrawpad.h"
#include "IdtText.h"

HWND floating_window = NULL; //����������
HWND drawpad_window = NULL; //���崰��
HWND ppt_window = NULL; //PPT�ؼ�����
HWND setting_window = NULL; //������ⴰ��
HWND freeze_window = NULL; //���񱳾�����

bool FreezePPT;
HWND ppt_show;
wstring ppt_title, ppt_software;
map<wstring, bool> ppt_title_recond;

//�����Ƿ��ö�
bool IsWindowFocused(HWND hWnd)
{
	return GetForegroundWindow() == hWnd;
}
//�������״̬��ȡ
bool isProcessRunning(const std::wstring& processPath)
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry)) {
		do {
			// �򿪽��̾��
			HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ProcessID);
			if (process == NULL) {
				continue;
			}

			// ��ȡ��������·��
			wchar_t path[MAX_PATH];
			DWORD size = MAX_PATH;
			if (QueryFullProcessImageName(process, 0, path, &size)) {
				if (processPath == path) {
					CloseHandle(process);
					CloseHandle(snapshot);
					return true;
				}
			}

			CloseHandle(process);
		} while (Process32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return false;
}
HWND GetLastFocusWindow()
{
	GUITHREADINFO guiThreadInfo;
	guiThreadInfo.cbSize = sizeof(GUITHREADINFO);
	if (GetGUIThreadInfo(0, &guiThreadInfo))
	{
		return guiThreadInfo.hwndFocus;
	}
	return NULL;
}
wstring GetWindowText(HWND hWnd)
{
	// ��ȡ���ڱ���ĳ���
	int length = GetWindowTextLength(hWnd);

	// ����һ���㹻��Ļ��������洢���ڱ���
	std::vector<wchar_t> buffer(length + 1);

	// ��ȡ���ڱ���
	GetWindowText(hWnd, &buffer[0], buffer.size());

	// ���ش��ڱ���
	return &buffer[0];
}

//�ö����򴰿�

vector<wstring> sswindows;

int GetWindowTextSafe(HWND hWnd, LPTSTR lpString, int nMaxCount)
{
	if (NULL == hWnd || FALSE == IsWindow(hWnd) || NULL == lpString || 0 == nMaxCount)
	{
		return GetWindowText(hWnd, lpString, nMaxCount);
	}
	DWORD dwHwndProcessID = 0;
	DWORD dwHwndThreadID = 0;
	dwHwndThreadID = GetWindowThreadProcessId(hWnd, &dwHwndProcessID);		//��ȡ���������Ľ��̺��߳�ID

	if (dwHwndProcessID != GetCurrentProcessId())		//���ڽ��̲��ǵ�ǰ���ý���ʱ������ԭ������
	{
		return GetWindowText(hWnd, lpString, nMaxCount);
	}

	//���ڽ����ǵ�ǰ����ʱ��
	if (dwHwndThreadID == GetCurrentThreadId())			//�����߳̾��ǵ�ǰ�����̣߳�����ԭ������
	{
		return GetWindowText(hWnd, lpString, nMaxCount);
	}

#ifndef _UNICODE
	WCHAR* lpStringUnicode = new WCHAR[nMaxCount];
	InternalGetWindowText(hWnd, lpStringUnicode, nMaxCount);
	int size = WideCharToMultiByte(CP_ACP, 0, lpStringUnicode, -1, NULL, 0, NULL, NULL);
	if (size <= nMaxCount)
	{
		size = WideCharToMultiByte(CP_ACP, 0, lpStringUnicode, -1, lpString, size, NULL, NULL);
		if (NULL != lpStringUnicode)
		{
			delete[]lpStringUnicode;
			lpStringUnicode = NULL;
		}
		return size;
	}
	if (NULL != lpStringUnicode)
	{
		delete[]lpStringUnicode;
		lpStringUnicode = NULL;
	}
	return 0;

#else
	return InternalGetWindowText(hWnd, lpString, nMaxCount);
#endif
}

wstring GetWindowTitle(HWND hWnd)
{
	wchar_t title[256];
	GetWindowTextSafe(hWnd, title, sizeof(title) / sizeof(wchar_t));
	return wstring(title);
}
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	sswindows.push_back(GetWindowTitle(hWnd));

	return TRUE;
}

void TopWindow()
{
	/*
	//����ǿ���ö�
	{
		HWND hForeWnd = NULL;
		DWORD dwForeID = 0;
		DWORD dwCurID = 0;

		hForeWnd = ::GetForegroundWindow();
		dwCurID = ::GetCurrentThreadId();
		dwForeID = ::GetWindowThreadProcessId(hForeWnd, NULL);
		::AttachThreadInput(dwCurID, dwForeID, TRUE);
		//::ShowWindow(hWnd, SW_SHOWNORMAL);
		::SetWindowPos(floating_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		::SetWindowPos(floating_window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		::SetForegroundWindow(floating_window);
		// ��ǰ̨�����߳���������ǰ�̣߳�Ҳ���ǳ���A�еĵ����̣߳�
		::AttachThreadInput(dwCurID, dwForeID, FALSE);
	}
	SetWindowPos(floating_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	*/

	Sleep(3000);
	while (1)
	{
		{
			sswindows.clear();
			EnumWindows(EnumWindowsProc, 0);

			ofstream writejson;
			writejson.imbue(locale("zh_CN.UTF8"));
			writejson.open(wstring_to_string(string_to_wstring(global_path) + L"bug fix 240402.01.log").c_str());

			for (int i = 0; i < (int)sswindows.size(); i++)
			{
				writejson << to_string(i) << " " + wstring_to_string(sswindows[i]) << endl;
			}

			writejson.close();
		}

		try
		{
			SetWindowPos(floating_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			SetWindowPos(ppt_window, floating_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

			if (!choose.select)
			{
				std::shared_lock<std::shared_mutex> lock1(StrokeImageListSm);
				bool flag = StrokeImageList.empty();
				lock1.unlock();
				if (flag)
				{
					SetWindowPos(drawpad_window, ppt_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					SetWindowPos(freeze_window, drawpad_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				}
			}
			else
			{
				SetWindowPos(drawpad_window, ppt_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				SetWindowPos(freeze_window, drawpad_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}
		catch (...)
		{
		};

		Sleep(1000);
	}
}