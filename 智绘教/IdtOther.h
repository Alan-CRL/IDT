#pragma once
#include "IdtMain.h"
#include "IdtText.h"

wstring GetCurrentExeDirectory();
wstring GetCurrentExePath();
wstring GetCurrentExeName();

//����״̬��ȡ
bool checkIsNetwork();
// ��ȡָ��ģ���е���Դ�ļ�
bool ExtractResource(LPCTSTR strDstFile, LPCTSTR strResType, LPCTSTR strResName);

//�ж�id�Ƿ����
bool isValidString(const wstring& str);

//��ݷ�ʽ�ж�
bool IsShortcutPointingToDirectory(const std::wstring& shortcutPath, const std::wstring& targetDirectory);
bool CreateShortcut(const std::wstring& shortcutPath, const std::wstring& targetExePath);
void SetShortcut();

// �������״̬��ȡ
bool isProcessRunning(const std::wstring& processPath);
// ���̳���·����ѯ
int ProcessRunningCnt(const std::wstring& processPath);

// ·��Ȩ�޼��
bool HasReadWriteAccess(const std::wstring& directoryPath);