#pragma once
#include "IdtMain.h"

//�����������
void CrashedHandler();

//�����Զ�����
extern int AutomaticUpdateStep;
wstring get_domain_name(wstring url);
wstring convertToHttp(const wstring& url);
void AutomaticUpdate();