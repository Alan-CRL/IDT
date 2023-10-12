#include "IdtWindow.h"

HWND floating_window = NULL; //����������
HWND drawpad_window = NULL; //���崰��
HWND ppt_window = NULL; //PPT�ؼ�����
HWND test_window = NULL; //������ⴰ��
HWND freeze_window = NULL; //���񱳾�����

bool FreezePPT;
HWND ppt_show;

//����ǿ���ö�
BOOL OnForceShow(HWND hWnd)
{
	HWND hForeWnd = NULL;
	DWORD dwForeID = 0;
	DWORD dwCurID = 0;

	hForeWnd = ::GetForegroundWindow();
	dwCurID = ::GetCurrentThreadId();
	dwForeID = ::GetWindowThreadProcessId(hForeWnd, NULL);
	::AttachThreadInput(dwCurID, dwForeID, TRUE);
	//::ShowWindow(hWnd, SW_SHOWNORMAL);
	::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	::SetForegroundWindow(hWnd);
	// ��ǰ̨�����߳���������ǰ�̣߳�Ҳ���ǳ���A�еĵ����̣߳�
	::AttachThreadInput(dwCurID, dwForeID, FALSE);

	return TRUE;
}
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