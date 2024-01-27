#pragma once
#include "IdtMain.h"

#include <tlhelp32.h> // �ṩ���̡�ģ�顢�̵߳ı����ȹ���

extern HWND floating_window; //����������
extern HWND drawpad_window; //���崰��
extern HWND ppt_window; //PPT�ؼ�����
extern HWND test_window; //������ⴰ��
extern HWND freeze_window; //���񱳾�����

extern bool FreezePPT;
extern HWND ppt_show;
extern wstring ppt_title;
extern map<wstring, bool> ppt_title_recond;

//����ǿ���ö�
BOOL OnForceShow(HWND hWnd);
//�����Ƿ��ö�
bool IsWindowFocused(HWND hWnd);
//�������״̬��ȡ
bool isProcessRunning(const std::wstring& processPath);
HWND GetLastFocusWindow();
wstring GetWindowText(HWND hWnd);