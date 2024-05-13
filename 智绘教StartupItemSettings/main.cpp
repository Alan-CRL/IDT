#include <windows.h>
#include <filesystem>
#include <iostream>
#include <sddl.h>
#include <sstream>
#include <thread>

using namespace std;

#define Sleep(int) this_thread::sleep_for(chrono::milliseconds(int))

void Test()
{
	MessageBox(NULL, L"��Ǵ�", L"���", MB_OK | MB_SYSTEMMODAL);
}
void Testi(int t)
{
	MessageBox(NULL, to_wstring(t).c_str(), L"��ֵ���", MB_OK | MB_SYSTEMMODAL);
}
void Testw(wstring t)
{
	MessageBox(NULL, t.c_str(), L"�ַ����", MB_OK | MB_SYSTEMMODAL);
}

string edition_date = "20240428.01";
wstring DataToSend = L"nullptr";

//string to wstring
wstring string_to_wstring(const string& s)
{
	if (s.empty()) return L"";

	int sizeRequired = MultiByteToWideChar(936, 0, s.c_str(), -1, NULL, 0);
	if (sizeRequired == 0) return L"";

	wstring ws(sizeRequired - 1, L'\0');
	MultiByteToWideChar(936, 0, s.c_str(), -1, &ws[0], sizeRequired);

	return ws;
}
//wstring to string
string wstring_to_string(const wstring& ws)
{
	if (ws.empty()) return "";

	int sizeRequired = WideCharToMultiByte(936, 0, ws.c_str(), -1, NULL, 0, NULL, NULL);
	if (sizeRequired == 0) return "";

	string s(sizeRequired - 1, '\0');
	WideCharToMultiByte(936, 0, ws.c_str(), -1, &s[0], sizeRequired, NULL, NULL);

	return s;
}

wstring GetCurrentExeDirectory()
{
	wchar_t buffer[MAX_PATH];
	DWORD length = GetModuleFileNameW(NULL, buffer, sizeof(buffer) / sizeof(wchar_t));
	if (length == 0 || length == sizeof(buffer) / sizeof(wchar_t)) return L"";

	filesystem::path fullPath(buffer);
	return fullPath.parent_path().wstring();
}
bool SetStartupAndSelfStart(bool bAutoRun, wstring path, wstring nameclass)
{
	path = L"\"" + path + L"\"";

	wchar_t pFileName[MAX_PATH] = { 0 };
	wcscpy_s(pFileName, path.c_str());

	HKEY hKey;
	LPCTSTR lpRun = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey);
	if (lRet != ERROR_SUCCESS) return false;

	if (bAutoRun) RegSetValueEx(hKey, nameclass.c_str(), 0, REG_SZ, (const BYTE*)(LPCSTR)pFileName, MAX_PATH);
	else RegDeleteValue(hKey, nameclass.c_str());

	RegCloseKey(hKey);
	return true;
}
bool QueryStartupSelfStart(wstring path, wstring nameclass)
{
	wchar_t pFileName[MAX_PATH] = { 0 };
	wcscpy_s(pFileName, path.c_str());

	path = L"\"" + path + L"\"";
	wchar_t pFileName2[MAX_PATH] = { 0 };
	wcscpy_s(pFileName2, path.c_str());

	HKEY hKey;
	LPCTSTR lpRun = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_READ, &hKey); // �޸Ĵ˴�ΪKEY_READ
	if (lRet != ERROR_SUCCESS) return false;

	wchar_t regValue[MAX_PATH] = { 0 };
	DWORD dwType;
	DWORD dwSize = sizeof(regValue);
	lRet = RegQueryValueEx(hKey, nameclass.c_str(), 0, &dwType, (LPBYTE)regValue, &dwSize);
	if (lRet != ERROR_SUCCESS || dwType != REG_SZ || (wcscmp(regValue, pFileName) != 0 && wcscmp(regValue, pFileName2) != 0)) // �����ѯ����ֵ����ֵ�����ַ�������ֵ������ָ��·�����򷵻�false
	{
		RegCloseKey(hKey);
		return false;
	}

	RegCloseKey(hKey);
	return true;
}

// ������ڵ�
int WINAPI wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	wstring parameters, path, NameClass;
	wstringstream wss(GetCommandLineW());

	getline(wss, parameters, L'/');
	getline(wss, parameters, L'\"');
	getline(wss, parameters, L'\"');

	getline(wss, path, L'/');
	getline(wss, path, L'\"');
	getline(wss, path, L'\"');

	getline(wss, NameClass, L'/');
	getline(wss, NameClass, L'\"');
	getline(wss, NameClass, L'\"');
	if (NameClass.empty()) NameClass = L"xmg_drawpad_startup";

	if (parameters.length() >= 4 && parameters.substr(0, 3) == L"set")
	{
		SetStartupAndSelfStart(true, path, NameClass);
		if (QueryStartupSelfStart(path, NameClass)) DataToSend = parameters.substr(3, 1) + L"success";
		else DataToSend = parameters.substr(3, 1) + L"fail";
	}
	else if (parameters.length() >= 7 && parameters.substr(0, 6) == L"delete")
	{
		SetStartupAndSelfStart(false, path, NameClass);
		if (QueryStartupSelfStart(path, NameClass)) DataToSend = parameters.substr(6, 1) + L"success";
		else DataToSend = parameters.substr(6, 1) + L"fail";
	}
	else if (parameters.length() >= 6 && parameters.substr(0, 5) == L"query")
	{
		if (QueryStartupSelfStart(path, NameClass)) DataToSend = parameters.substr(5, 1) + L"success";
		else DataToSend = parameters.substr(5, 1) + L"fail";
	}
	else return 0;

	/*
		DWORD dwBytesWritten;
		wstring dataToSend = L"�ܵ����ò���";

		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
		HANDLE hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\IDTPipe1"), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 0, 0, NMPWAIT_WAIT_FOREVER, &sa);
		ConnectNamedPipe(hPipe, NULL);
		WriteFile(hPipe, dataToSend.data(), dataToSend.size() * sizeof(wchar_t), &dwBytesWritten, NULL);

		CloseHandle(hPipe);
	*/

	HANDLE hPipe;
	DWORD dwBytesWritten;

	PSECURITY_DESCRIPTOR pSD;
	SECURITY_ATTRIBUTES sa;
	ConvertStringSecurityDescriptorToSecurityDescriptor(L"D:(A;OICI;GRGW;;;WD)", SDDL_REVISION_1, &pSD, NULL);
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;
	hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\IDTPipe1"), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 0, 0, NMPWAIT_WAIT_FOREVER, &sa);
	LocalFree(pSD);

	OVERLAPPED oOverlap;
	oOverlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	oOverlap.Internal = 0;
	oOverlap.InternalHigh = 0;
	oOverlap.Offset = 0;
	oOverlap.OffsetHigh = 0;

	int for_i;
	for (for_i = 0; for_i <= 5; for_i++)
	{
		if (ConnectNamedPipe(hPipe, &oOverlap)) break;
		else Sleep(100);
	}

	if (for_i <= 5) WriteFile(hPipe, DataToSend.data(), DataToSend.size() * sizeof(wchar_t), &dwBytesWritten, NULL);

	CloseHandle(hPipe);
	CloseHandle(oOverlap.hEvent);

	return 0;
}