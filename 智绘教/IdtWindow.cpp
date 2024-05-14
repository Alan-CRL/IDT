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

void TopWindow()
{
	/*
	//����ǿ���ö�
	{
		HWND hForeWnd = GetForegroundWindow();
		DWORD dwForeID = GetCurrentThreadId();
		DWORD dwCurID = GetWindowThreadProcessId(hForeWnd, NULL);
		AttachThreadInput(dwCurID, dwForeID, TRUE);
		SetWindowPos(floating_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		SetWindowPos(floating_window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		SetForegroundWindow(floating_window);
		AttachThreadInput(dwCurID, dwForeID, FALSE);

		SetWindowPos(floating_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	*/

	while (1)
	{
		if (!SetWindowPos(floating_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)) IDTLogger->warn("[�����ö��߳�][TopWindow] �ö�����������ʱʧ�� Error" + to_string(GetLastError()));
		if (!SetWindowPos(ppt_window, floating_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)) IDTLogger->warn("[�����ö��߳�][TopWindow] �ö�PPT��ע�ؼ�����ʱʧ�� Error" + to_string(GetLastError()));

		if (!choose.select)
		{
			std::shared_lock<std::shared_mutex> lock1(StrokeImageListSm);
			bool flag = StrokeImageList.empty();
			lock1.unlock();
			if (flag)
			{
				if (!SetWindowPos(drawpad_window, ppt_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)) IDTLogger->warn("[�����ö��߳�][TopWindow] �ö����崰��ʱʧ�� Error" + to_string(GetLastError()));
				if (!SetWindowPos(freeze_window, drawpad_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)) IDTLogger->warn("[�����ö��߳�][TopWindow] �ö����񱳾�����ʱʧ�� Error" + to_string(GetLastError()));
			}
		}
		else
		{
			if (!SetWindowPos(drawpad_window, ppt_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)) IDTLogger->warn("[�����ö��߳�][TopWindow] �ö����崰��ʱʧ�� Error" + to_string(GetLastError()));
			if (!SetWindowPos(freeze_window, drawpad_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)) IDTLogger->warn("[�����ö��߳�][TopWindow] �ö����񱳾�������ʱʧ�� Error" + to_string(GetLastError()));
		}

		Sleep(1000);
	}
}