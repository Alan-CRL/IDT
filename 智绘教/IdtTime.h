#pragma once
#include "IdtMain.h"
#include <ctime>

extern SYSTEMTIME sys_time;
//ʱ���
wstring getTimestamp();
wstring getCurrentDate();
//��ȡ����
wstring CurrentDate();
//��ȡʱ��
wstring CurrentTime();

void GetTime();
string GetCurrentTimeAll();

tm GetCurrentLocalTime();