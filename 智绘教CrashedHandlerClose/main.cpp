#include <iostream>
#include <windows.h>
#include <thread>
#include <filesystem>
#include <tlhelp32.h>

using namespace std;

#define Sleep(int) this_thread::sleep_for(chrono::milliseconds(int))
#define Test() MessageBox(NULL, L"��Ǵ�", L"���", MB_OK | MB_SYSTEMMODAL)
#define Testi(int) MessageBox(NULL, to_wstring(int).c_str(), L"��ֵ���", MB_OK | MB_SYSTEMMODAL)
#define Testw(wstring) MessageBox(NULL, wstring.c_str(), L"�ַ����", MB_OK | MB_SYSTEMMODAL)

string global_path; //����ǰ·��
string main_path; //������·��

//string ת wstring
wstring convert_to_wstring(const string s)
{
	LPCSTR pszSrc = s.c_str();
	int nLen = s.size();

	int nSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSrc, nLen, 0, 0);
	WCHAR* pwszDst = new WCHAR[nSize + 1];
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSrc, nLen, pwszDst, nSize);
	pwszDst[nSize] = 0;
	if (pwszDst[0] == 0xFEFF) // skip Oxfeff
		for (int i = 0; i < nSize; i++)
			pwszDst[i] = pwszDst[i + 1];
	wstring wcharString(pwszDst);
	delete pwszDst;
	return wcharString;
}
//wstring ת string
string convert_to_string(const wstring str)
{
	int size = WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), nullptr, 0, nullptr, nullptr);
	auto p_str(std::make_unique<char[]>(size + 1));
	WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.size(), p_str.get(), size, nullptr, nullptr);
	return std::string(p_str.get());
}

//��ֹ����
bool TerminateProcess(const std::wstring& processPath) {
	// �������̿���
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return false;
	}

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	// �������̿���
	if (Process32First(snapshot, &entry)) {
		do {
			// �ȽϽ���·��
			if (processPath == entry.szExeFile) {
				// �򿪽��̾��
				HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
				if (process == NULL) {
					CloseHandle(snapshot);
					return false;
				}

				// ��ֹ����
				TerminateProcess(process, 0);
				CloseHandle(process);
			}
		} while (Process32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return true;
}
vector<HWND> GetProcessWindows(const std::wstring& processPath) {
	std::vector<HWND> result;

	// ��ȡ���� ID
	DWORD processId = 0;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(snapshot, &entry)) {
		do {
			// �򿪽��̾��
			HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ProcessID);
			if (process == NULL) {
				continue;
			}

			// ��ȡ��������·��
			wchar_t path[MAX_PATH];
			DWORD size = MAX_PATH;
			if (QueryFullProcessImageName(process, 0, path, &size)) {
				if (processPath == path) {
					processId = entry.th32ProcessID;
					CloseHandle(process);
					break;
				}
			}

			CloseHandle(process);
		} while (Process32Next(snapshot, &entry));
	}
	CloseHandle(snapshot);

	// ö�ٴ���
	auto param = std::pair{ processId, &result };
	EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
		auto& [processId, result] = *reinterpret_cast<std::pair<DWORD, std::vector<HWND>*>*>(lParam);

		DWORD windowProcessId = 0;
		GetWindowThreadProcessId(hwnd, &windowProcessId);
		if (windowProcessId == processId) {
			result->push_back(hwnd);
		}

		return TRUE;
		}, reinterpret_cast<LPARAM>(&param));

	return result;
}

// ������ڵ�
int WINAPI wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	//ȫ��·��Ԥ����
	{
		global_path = _pgmptr;
		for (int i = int(global_path.length() - 1); i >= 0; i--)
		{
			if (global_path[i] == '\\')
			{
				global_path = global_path.substr(0, i + 1);
				break;
			}
		}

		filesystem::path directory(global_path);
		main_path = directory.parent_path().parent_path().string();
	}

	vector<HWND> windows = GetProcessWindows(convert_to_wstring(main_path) + L"\\�ǻ��.exe");
	for (HWND hwnd : windows) ShowWindow(hwnd, SW_HIDE);
	TerminateProcess(convert_to_wstring(main_path) + L"\\�ǻ��.exe");

	MessageBox(NULL, L"�ǻ��(��Ļ�������) �������ر��������ܵ���������\n\n����ȷ������ǿ�ƹر�", L"�ǻ�����ر������� �汾20240408.01", MB_OK | MB_SYSTEMMODAL | MB_ICONERROR);

	return 0;
}