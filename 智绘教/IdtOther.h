#pragma once
#include "IdtMain.h"
#include "IdtText.h"

#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <io.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

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