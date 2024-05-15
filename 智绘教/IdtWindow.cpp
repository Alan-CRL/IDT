#include "IdtWindow.h"

#include "IdtDraw.h"
#include "IdtDrawpad.h"
#include "IdtRts.h"
#include "IdtText.h"

HWND floating_window = NULL; //����������
HWND drawpad_window = NULL; //���崰��
HWND ppt_window = NULL; //PPT�ؼ�����
HWND freeze_window = NULL; //���񱳾�����
HWND setting_window = NULL; //������ⴰ��

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

IdtWindowsIsVisibleStruct IdtWindowsIsVisible;

//�ö����򴰿�
void TopWindow()
{
	// �ȴ����ڻ���
	IDTLogger->info("[�����ö��߳�][TopWindow] �ȴ����ڳ��λ���");
	for (int i = 1; i <= 20; i++)
	{
		this_thread::sleep_for(chrono::milliseconds(500));

		bool flag = true;
		if (flag && !IdtWindowsIsVisible.floatingWindow) flag = false;
		if (flag && !IdtWindowsIsVisible.pptWindow) flag = false;
		if (flag && !IdtWindowsIsVisible.drawpadWindow) flag = false;
		if (flag && !IdtWindowsIsVisible.freezeWindow) flag = false;

		if (flag)
		{
			IdtWindowsIsVisible.allCompleted = true;
			break;
		}
	}

	// ״̬��ʱ -> ��������
	if (!IdtWindowsIsVisible.allCompleted)
	{
		IDTLogger->warn("[�����ö��߳�][TopWindow] �ȴ����ڳ��λ��Ƴ�ʱ");

		offSignal = true;
		return;
	}
	IDTLogger->info("[�����ö��߳�][TopWindow] �ȴ����ڳ��λ������");

	IDTLogger->info("[�����ö��߳�][TopWindow] ��ʾ����");
	ShowWindow(floating_window, SW_SHOW);
	ShowWindow(ppt_window, SW_SHOW);
	ShowWindow(drawpad_window, SW_SHOW);
	ShowWindow(freeze_window, SW_SHOW);
	IDTLogger->info("[�����ö��߳�][TopWindow] ��ʾ�������");

	while (uRealTimeStylus != 1) this_thread::sleep_for(chrono::milliseconds(500));
	while (!offSignal)
	{
		// ��鴰����ʾ״̬
		{
			if (!IsWindowVisible(floating_window)) ShowWindow(floating_window, SW_SHOW);
			if (!IsWindowVisible(ppt_window)) ShowWindow(ppt_window, SW_SHOW);
			if (!IsWindowVisible(drawpad_window)) ShowWindow(drawpad_window, SW_SHOW);
			if (!IsWindowVisible(freeze_window)) ShowWindow(freeze_window, SW_SHOW);
		}

		// �ö�����
		if (!choose.select)
		{
			std::shared_lock<std::shared_mutex> lock1(StrokeImageListSm);
			bool flag = StrokeImageList.empty();
			lock1.unlock();
			if (flag)
			{
				if (!SetWindowPos(freeze_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE))
					IDTLogger->warn("[�����ö��߳�][TopWindow] �ö�����ʱʧ�� Error" + to_string(GetLastError()));
			}
		}
		else
		{
			if (!SetWindowPos(freeze_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE))
				IDTLogger->warn("[�����ö��߳�][TopWindow] �ö�����ʱʧ�� Error" + to_string(GetLastError()));
		}

		// ����ö����
		if (!(GetWindowLong(freeze_window, GWL_EXSTYLE) & WS_EX_TOPMOST))
		{
			IDTLogger->warn("[�����ö��߳�][TopWindow] �ö�����ʧ��");
			IDTLogger->info("[�����ö��߳�][TopWindow] ǿ���ö�����");

			HWND hForeWnd = GetForegroundWindow();
			DWORD dwForeID = GetCurrentThreadId();
			DWORD dwCurID = GetWindowThreadProcessId(hForeWnd, NULL);
			AttachThreadInput(dwCurID, dwForeID, TRUE);
			SetWindowPos(freeze_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			SetWindowPos(freeze_window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			SetForegroundWindow(freeze_window);
			AttachThreadInput(dwCurID, dwForeID, FALSE);

			SetWindowPos(freeze_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

			IDTLogger->info("[�����ö��߳�][TopWindow] ǿ���ö��������");
		}

		// �ӳٵȴ�
		for (int i = 1; i <= 10; i++)
		{
			if (offSignal) break;
			this_thread::sleep_for(chrono::milliseconds(100));
		}
	}

	IDTLogger->info("[�����ö��߳�][TopWindow] ���ش���");
	ShowWindow(floating_window, SW_HIDE);
	ShowWindow(ppt_window, SW_HIDE);
	ShowWindow(drawpad_window, SW_HIDE);
	ShowWindow(freeze_window, SW_HIDE);
	IDTLogger->info("[�����ö��߳�][TopWindow] ���ش������");

	return;
}