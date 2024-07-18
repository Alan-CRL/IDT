#pragma once
#include "IdtMain.h"

#include <tlhelp32.h> // �ṩ���̡�ģ�顢�̵߳ı����ȹ���

extern HWND floating_window; //����������
extern HWND drawpad_window; //���崰��
extern HWND ppt_window; //PPT�ؼ�����
extern HWND freeze_window; //���񱳾�����
extern HWND setting_window; //������ⴰ��

extern bool FreezePPT;
extern HWND ppt_show;
extern wstring ppt_title, ppt_software;
extern map<wstring, bool> ppt_title_recond;

//�����Ƿ��ö�
bool IsWindowFocused(HWND hWnd);

HWND GetLastFocusWindow();
wstring GetWindowText(HWND hWnd);

struct IdtWindowsIsVisibleStruct
{
	IdtWindowsIsVisibleStruct()
	{
		floatingWindow = false;
		drawpadWindow = false;
		pptWindow = false;
		freezeWindow = false;

		allCompleted = false;
	}

	bool floatingWindow;
	bool drawpadWindow;
	bool pptWindow;
	bool freezeWindow;

	bool allCompleted;
};
extern IdtWindowsIsVisibleStruct IdtWindowsIsVisible;

//�ö����򴰿�
void TopWindow();