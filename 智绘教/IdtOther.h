#pragma once
#include "IdtMain.h"

#include "IdtText.h"

wstring GetCurrentExeDirectory();
//��������������
bool ModifyRegedit(bool bAutoRun);
//����״̬��ȡ
bool checkIsNetwork();
// ��ȡָ��ģ���е���Դ�ļ�
bool ExtractResource(LPCTSTR strDstFile, LPCTSTR strResType, LPCTSTR strResName);

wstring GetCPUID();
wstring GetMotherboardUUID();
wstring GetMainHardDiskSerialNumber();
//�ж�Ӳ�����к��Ƿ����
bool isValidString(const wstring& str);