#include "IdtOther.h"

#include <io.h>
#include <objbase.h>
#include <psapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <tlhelp32.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

wstring GetCurrentExeDirectory()
{
	wchar_t buffer[MAX_PATH];
	DWORD length = GetModuleFileNameW(NULL, buffer, sizeof(buffer) / sizeof(wchar_t));
	if (length == 0 || length == sizeof(buffer) / sizeof(wchar_t)) return L"";

	filesystem::path fullPath(buffer);
	return fullPath.parent_path().wstring();
}
wstring GetCurrentExePath()
{
	wchar_t buffer[MAX_PATH];
	DWORD length = GetModuleFileNameW(NULL, buffer, sizeof(buffer) / sizeof(wchar_t));
	if (length == 0 || length == sizeof(buffer) / sizeof(wchar_t)) return L"";

	return (wstring)buffer;
}
wstring GetCurrentExeName()
{
	wchar_t buffer[MAX_PATH];
	DWORD length = GetModuleFileNameW(NULL, buffer, sizeof(buffer) / sizeof(wchar_t));
	if (length == 0 || length == sizeof(buffer) / sizeof(wchar_t)) return L"";

	filesystem::path fullPath(buffer);
	return fullPath.filename().wstring();
}

//����״̬��ȡ
bool checkIsNetwork()
{
	//  ͨ��NLA�ӿڻ�ȡ����״̬
	IUnknown* pUnknown = NULL;
	BOOL   bOnline = TRUE;//�Ƿ�����
	HRESULT Result = CoCreateInstance(CLSID_NetworkListManager, NULL, CLSCTX_ALL,
		IID_IUnknown, (void**)&pUnknown);
	if (SUCCEEDED(Result))
	{
		INetworkListManager* pNetworkListManager = NULL;
		if (pUnknown)
			Result = pUnknown->QueryInterface(IID_INetworkListManager, (void
				**)&pNetworkListManager);
		if (SUCCEEDED(Result))
		{
			VARIANT_BOOL IsConnect = VARIANT_FALSE;
			if (pNetworkListManager)
				Result = pNetworkListManager->get_IsConnectedToInternet(&IsConnect);
			if (SUCCEEDED(Result))
			{
				bOnline = (IsConnect == VARIANT_TRUE) ? true : false;
			}
		}
		if (pNetworkListManager)
			pNetworkListManager->Release();
	}
	if (pUnknown) pUnknown->Release();

	return bOnline;
}
// ��ȡָ��ģ���е���Դ�ļ�
bool ExtractResource(LPCTSTR strDstFile, LPCTSTR strResType, LPCTSTR strResName)
{
	// �����ļ�
	HANDLE hFile = ::CreateFile(strDstFile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	// ������Դ�ļ���������Դ���ڴ桢�õ���Դ��С
	HRSRC	hRes = ::FindResource(NULL, strResName, strResType);
	HGLOBAL	hMem = ::LoadResource(NULL, hRes);
	DWORD	dwSize = ::SizeofResource(NULL, hRes);

	// д���ļ�
	DWORD dwWrite = 0;		// ����д���ֽ�
	::WriteFile(hFile, hMem, dwSize, &dwWrite, NULL);
	::CloseHandle(hFile);

	return true;
}

//�ж�id�Ƿ����
bool isValidString(const wstring& str)
{
	for (wchar_t ch : str)
	{
		// ����ַ����ǿɴ�ӡ�ģ����Ҳ��ǿո�����Ϊ������
		if (!iswprint(ch) && !iswspace(ch))  return false;
	}
	return true;
}

//��ݷ�ʽ�ж�
bool IsShortcutPointingToDirectory(const std::wstring& shortcutPath, const std::wstring& targetDirectory)
{
	IShellLink* psl;
	//CoInitialize(NULL);

	// ����һ��IShellLink����
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hres)) {
		IPersistFile* ppf;

		// ��ȡIShellLink��IPersistFile�ӿ�
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
		if (SUCCEEDED(hres)) {
			// �򿪿�ݷ�ʽ�ļ�
			hres = ppf->Load(shortcutPath.c_str(), STGM_READ);
			if (SUCCEEDED(hres)) {
				WIN32_FIND_DATAW wfd;
				ZeroMemory(&wfd, sizeof(wfd));
				// ��ȡ��ݷ�ʽ��Ŀ��·��
				hres = psl->GetPath(wfd.cFileName, MAX_PATH, NULL, SLGP_RAWPATH);
				if (SUCCEEDED(hres)) {
					// ���Ŀ��·���Ƿ���ָ��Ŀ¼��ƥ��
					if (std::wstring(wfd.cFileName).find(targetDirectory) != std::wstring::npos) {
						return true;
					}
				}
			}
			ppf->Release();
		}
		psl->Release();
	}
	//CoUninitialize();

	return false;
}
bool CreateShortcut(const std::wstring& shortcutPath, const std::wstring& targetExePath)
{
	//CoInitialize(NULL);

	// ����һ��IShellLink����
	IShellLink* psl;
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);

	if (SUCCEEDED(hres)) {
		// ���ÿ�ݷ�ʽ��Ŀ��·��
		psl->SetPath(targetExePath.c_str());

		// ��ȡ����Ŀ¼
		LPITEMIDLIST pidl;
		SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl);

		// ����һ��IShellLink�����IPersistFile�ӿ�
		IPersistFile* ppf;
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

		if (SUCCEEDED(hres)) {
			// �����ݷ�ʽ
			hres = ppf->Save(shortcutPath.c_str(), TRUE);
			ppf->Release();
		}

		psl->Release();
	}

	//CoUninitialize();

	return SUCCEEDED(hres);
}
void SetShortcut()
{
	wchar_t desktopPath[MAX_PATH];
	wstring DesktopPath;

	if (SHGetSpecialFolderPathW(0, desktopPath, CSIDL_DESKTOP, FALSE)) DesktopPath = wstring(desktopPath) + L"\\";
	else return;

	if (_waccess_s((DesktopPath + L"�ǻ��(��Ļ��ע����).lnk").c_str(), 0) == 0)
	{
		if (IsShortcutPointingToDirectory(DesktopPath + L"�ǻ��(��Ļ��ע����).lnk", GetCurrentExePath()))
		{
			bool flag = false;

			for (const auto& entry : std::filesystem::directory_iterator(DesktopPath))
			{
				if (std::filesystem::is_regular_file(entry) && entry.path().extension() == L".lnk")
				{
					if (entry.path().wstring() != DesktopPath + L"�ǻ��(��Ļ��ע����).lnk" && IsShortcutPointingToDirectory(entry.path().wstring(), GetCurrentExePath()))
					{
						std::error_code ec;
						std::filesystem::remove(entry.path().wstring(), ec);

						flag = true;
					}
				}
			}

			if (!flag) return;
		}
		else
		{
			std::error_code ec;
			std::filesystem::remove(DesktopPath + L"�ǻ��(��Ļ��ע����).lnk", ec);
			CreateShortcut(DesktopPath + L"�ǻ��(��Ļ��ע����).lnk", GetCurrentExePath());

			for (const auto& entry : std::filesystem::directory_iterator(DesktopPath))
			{
				if (std::filesystem::is_regular_file(entry) && entry.path().extension() == L".lnk")
				{
					if (entry.path().wstring() != DesktopPath + L"�ǻ��(��Ļ��ע����).lnk" && IsShortcutPointingToDirectory(entry.path().wstring(), GetCurrentExePath()))
					{
						std::error_code ec;
						std::filesystem::remove(entry.path().wstring(), ec);
					}
				}
			}
		}
	}
	else
	{
		for (const auto& entry : std::filesystem::directory_iterator(DesktopPath))
		{
			if (std::filesystem::is_regular_file(entry) && entry.path().extension() == L".lnk")
			{
				if (IsShortcutPointingToDirectory(entry.path().wstring(), GetCurrentExePath()))
				{
					std::error_code ec;
					std::filesystem::remove(entry.path().wstring(), ec);
				}
			}
		}
		CreateShortcut(DesktopPath + L"�ǻ��(��Ļ��ע����).lnk", GetCurrentExePath());
	}

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return;
}

// �������״̬��ȡ
bool isProcessRunning(const std::wstring& processPath)
{
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
					CloseHandle(process);
					CloseHandle(snapshot);
					return true;
				}
			}

			CloseHandle(process);
		} while (Process32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return false;
}
// ���̳���·����ѯ
int ProcessRunningCnt(const std::wstring& processPath)
{
	int ret = 0;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
	{
		while (Process32Next(snapshot, &entry))
		{
			// ��ȡ���̵�����·��
			wchar_t processFullPath[MAX_PATH] = L"";

			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, entry.th32ProcessID);
			if (hProcess)
			{
				HMODULE hMod;
				DWORD cbNeeded;
				if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
				{
					GetModuleFileNameExW(hProcess, hMod, processFullPath, MAX_PATH);
				}
				CloseHandle(hProcess);
			}

			// �Ƚ�·���Ƿ���ͬ
			if (wcslen(processFullPath) > 0 && wcscmp(processFullPath, processPath.c_str()) == 0) ret++;
		}
	}

	CloseHandle(snapshot);
	return ret;
}