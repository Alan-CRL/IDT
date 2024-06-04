#include "IdtTime.h"

SYSTEMTIME sys_time;
//ʱ���
wstring getTimestamp()
{
	auto now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	std::wstringstream wss;
	wss << millis;
	return wss.str();
}
wstring getCurrentDate()
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::wstringstream wss;
	wss << std::put_time(&tm, L"%Y%m%d");
	return wss.str();
}
//��ȡ����
wstring CurrentDate()
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::wostringstream woss;
	woss << std::put_time(&tm, L"%Y-%m-%d");
	return woss.str();
}
//��ȡʱ��
wstring CurrentTime()
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::wostringstream woss;
	woss << std::put_time(&tm, L"%H-%M-%S");
	return woss.str();
}

void GetTime()
{
	threadStatus[L"GetTime"] = true;
	while (!offSignal)
	{
		GetLocalTime(&sys_time);
		this_thread::sleep_for(chrono::milliseconds(1000));
	}
	threadStatus[L"GetTime"] = false;
}
string GetCurrentTimeAll()
{
	auto now = std::chrono::system_clock::now(); // ��ȡ��ǰʱ���
	auto in_time_t = std::chrono::system_clock::to_time_t(now); // ת��Ϊtime_t

	std::tm buf; // ����ʱ��ṹ��
	localtime_s(&buf, &in_time_t); // ת��Ϊ����ʱ��

	std::stringstream ss;
	ss << std::put_time(&buf, "%Y/%m/%d %H:%M:%S"); // ��ʽ��ʱ��
	return ss.str();
}

tm GetCurrentLocalTime()
{
	time_t currentTime = time(nullptr);
	tm localTime = {};

	localtime_s(&localTime, &currentTime);

	return localTime;
}