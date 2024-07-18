/*
 * @file		IdtDisplayManagement.h
 * @brief		IDT Display management | ��ʾ������
 * @note		Obtain and manage display status | ��ȡ��������ʾ��״̬
 *
 * @author		AlanCRL
 * @qq			2685549821
 * @email		alan-crl@foxmail.com
*/

// All function and variable descriptions should be in the corresponding cpp file.
// ���еĺ����ͱ���˵��Ӧ���ڶ�Ӧ�� cpp �ļ��С�

#pragma once
#include "IdtMain.h"

extern int DisplaysNumber;
extern shared_mutex DisplaysNumberSm;
extern vector<tuple<HMONITOR, RECT, bool>> DisplaysInfo; // ��ʾ���������ʾ����Χ���Ƿ�������ʾ��
extern shared_mutex DisplaysInfoSm;

extern bool enableAppBarAutoHide;

struct MainMonitorStruct
{
	HMONITOR Monitor;
	RECT rcMonitor;

	int MonitorWidth;
	int MonitorHeight;

	bool operator==(const MainMonitorStruct& other) const
	{
		return Monitor == other.Monitor &&
			EqualRect(&rcMonitor, &other.rcMonitor) &&
			MonitorWidth == other.MonitorWidth &&
			MonitorHeight == other.MonitorHeight;
	}
	bool operator!=(const MainMonitorStruct& other) const
	{
		return !(*this == other);
	}
};
extern MainMonitorStruct MainMonitor;

void DisplayManagementMain();