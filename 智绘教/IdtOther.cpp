#include "IdtOther.h"

wstring GetCurrentExeDirectory()
{
	wchar_t buffer[MAX_PATH];
	DWORD length = GetModuleFileNameW(NULL, buffer, sizeof(buffer) / sizeof(wchar_t));
	if (length == 0 || length == sizeof(buffer) / sizeof(wchar_t)) return L"";

	filesystem::path fullPath(buffer);
	return fullPath.parent_path().wstring();
}
//��������������
bool ModifyRegedit(bool bAutoRun)
{
	wchar_t pFileName[MAX_PATH] = { 0 };
	wcscpy(pFileName, string_to_wstring(_pgmptr).c_str());

	HKEY hKey;
	LPCTSTR lpRun = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey);
	if (lRet != ERROR_SUCCESS)
		return false;

	if (bAutoRun)
		RegSetValueEx(hKey, L"xmg_drawpad_startup", 0, REG_SZ, (const BYTE*)(LPCSTR)pFileName, MAX_PATH);
	else
		RegDeleteValueA(hKey, "xmg_drawpad_startup");
	RegCloseKey(hKey);
	return true;
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

wstring GetCPUID()
{
	wstring strCPUID;
	int CPUInfo[4] = { -1 };
	unsigned long s1, s2;
	wchar_t buffer[17];

	__cpuid(CPUInfo, 1);
	s1 = CPUInfo[3];
	s2 = CPUInfo[0];

	swprintf(buffer, 17, L"%08X%08X", s1, s2);
	strCPUID = buffer;
	return strCPUID;
}
wstring GetMotherboardUUID()
{
	std::wstring motherboardUUID;

	// ��ʼ�� COM
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr))
	{
		// ��ʼ�� WMI
		hr = CoInitializeSecurity(
			NULL,
			-1,
			NULL,
			NULL,
			RPC_C_AUTHN_LEVEL_DEFAULT,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			NULL,
			EOAC_NONE,
			NULL
		);
		if (SUCCEEDED(hr))
		{
			IWbemLocator* pWbemLocator = NULL;
			hr = CoCreateInstance(
				CLSID_WbemLocator,
				0,
				CLSCTX_INPROC_SERVER,
				IID_IWbemLocator,
				(LPVOID*)&pWbemLocator
			);

			if (SUCCEEDED(hr))
			{
				IWbemServices* pWbemServices = NULL;
				hr = pWbemLocator->ConnectServer(
					_bstr_t(L"ROOT\\CIMV2"),
					NULL,
					NULL,
					0,
					NULL,
					0,
					0,
					&pWbemServices
				);

				if (SUCCEEDED(hr))
				{
					hr = CoSetProxyBlanket(
						pWbemServices,
						RPC_C_AUTHN_WINNT,
						RPC_C_AUTHZ_NONE,
						NULL,
						RPC_C_AUTHN_LEVEL_CALL,
						RPC_C_IMP_LEVEL_IMPERSONATE,
						NULL,
						EOAC_NONE
					);

					if (SUCCEEDED(hr))
					{
						// ִ�� WMI ��ѯ
						IEnumWbemClassObject* pEnumObject = NULL;
						hr = pWbemServices->ExecQuery(
							_bstr_t("WQL"),
							_bstr_t("SELECT UUID FROM Win32_ComputerSystemProduct"),
							WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
							NULL,
							&pEnumObject
						);

						if (SUCCEEDED(hr))
						{
							IWbemClassObject* pClassObject = NULL;
							ULONG uReturned = 0;

							// ��ȡ��ѯ���
							hr = pEnumObject->Next(
								WBEM_INFINITE,
								1,
								&pClassObject,
								&uReturned
							);

							if (SUCCEEDED(hr) && uReturned == 1)
							{
								VARIANT vtProp;
								VariantInit(&vtProp);

								// ��ȡ UUID ����
								hr = pClassObject->Get(
									_bstr_t(L"UUID"),
									0,
									&vtProp,
									0,
									0
								);

								if (SUCCEEDED(hr))
								{
									// �� UUID ת��Ϊ wstring
									motherboardUUID = vtProp.bstrVal;

									VariantClear(&vtProp);
								}
								pClassObject->Release();
							}
							pEnumObject->Release();
						}
					}
					pWbemServices->Release();
				}
				pWbemLocator->Release();
			}
		}
		CoUninitialize();
	}

	return motherboardUUID;
}
wstring GetMainHardDiskSerialNumber()
{
	std::wstring serialNumber;

	// ��ʼ�� COM
	HRESULT hr = CoInitializeSecurity(
		NULL,
		-1,
		NULL,
		NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE,
		NULL
	);

	if (SUCCEEDED(hr))
	{
		IWbemLocator* pWbemLocator = NULL;
		hr = CoCreateInstance(
			CLSID_WbemLocator,
			0,
			CLSCTX_INPROC_SERVER,
			IID_IWbemLocator,
			(LPVOID*)&pWbemLocator
		);

		if (SUCCEEDED(hr))
		{
			IWbemServices* pWbemServices = NULL;
			hr = pWbemLocator->ConnectServer(
				_bstr_t(L"ROOT\\CIMV2"),
				NULL,
				NULL,
				0,
				NULL,
				0,
				0,
				&pWbemServices
			);

			if (SUCCEEDED(hr))
			{
				hr = CoSetProxyBlanket(
					pWbemServices,
					RPC_C_AUTHN_WINNT,
					RPC_C_AUTHZ_NONE,
					NULL,
					RPC_C_AUTHN_LEVEL_CALL,
					RPC_C_IMP_LEVEL_IMPERSONATE,
					NULL,
					EOAC_NONE
				);

				if (SUCCEEDED(hr))
				{
					// ִ�� WMI ��ѯ
					IEnumWbemClassObject* pEnumObject = NULL;
					hr = pWbemServices->ExecQuery(
						_bstr_t("WQL"),
						_bstr_t("SELECT SerialNumber FROM Win32_DiskDrive WHERE Index = 0"),
						WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
						NULL,
						&pEnumObject
					);

					if (SUCCEEDED(hr))
					{
						IWbemClassObject* pClassObject = NULL;
						ULONG uReturned = 0;

						// ��ȡ��ѯ���
						hr = pEnumObject->Next(
							WBEM_INFINITE,
							1,
							&pClassObject,
							&uReturned
						);

						if (SUCCEEDED(hr) && uReturned == 1)
						{
							VARIANT vtProp;
							VariantInit(&vtProp);

							// ��ȡӲ�����к�����
							hr = pClassObject->Get(
								_bstr_t(L"SerialNumber"),
								0,
								&vtProp,
								0,
								0
							);

							if (SUCCEEDED(hr))
							{
								if (vtProp.vt == VT_BSTR)
								{
									// �����к�ת��Ϊ wstring
									serialNumber = vtProp.bstrVal;
								}

								VariantClear(&vtProp);
							}
							pClassObject->Release();
						}
						pEnumObject->Release();
					}
				}
				pWbemServices->Release();
			}
			pWbemLocator->Release();
		}
	}

	return serialNumber;
}
//�ж�Ӳ�����к��Ƿ����
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

	if (_waccess_s((DesktopPath + L"�ǻ��(��Ļ��ע���幤��).lnk").c_str(), 0) == 0)
	{
		if (IsShortcutPointingToDirectory(DesktopPath + L"�ǻ��(��Ļ��ע���幤��).lnk", string_to_wstring(global_path) + L"�ǻ��.exe"))
		{
			bool flag = false;

			for (const auto& entry : std::filesystem::directory_iterator(DesktopPath))
			{
				if (std::filesystem::is_regular_file(entry) && entry.path().extension() == L".lnk")
				{
					if (entry.path().wstring() != DesktopPath + L"�ǻ��(��Ļ��ע���幤��).lnk" && IsShortcutPointingToDirectory(entry.path().wstring(), string_to_wstring(global_path) + L"�ǻ��.exe"))
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
			std::filesystem::remove(DesktopPath + L"�ǻ��(��Ļ��ע���幤��).lnk", ec);
			CreateShortcut(DesktopPath + L"�ǻ��(��Ļ��ע���幤��).lnk", string_to_wstring(global_path) + L"�ǻ��.exe");

			for (const auto& entry : std::filesystem::directory_iterator(DesktopPath))
			{
				if (std::filesystem::is_regular_file(entry) && entry.path().extension() == L".lnk")
				{
					if (entry.path().wstring() != DesktopPath + L"�ǻ��(��Ļ��ע���幤��).lnk" && IsShortcutPointingToDirectory(entry.path().wstring(), string_to_wstring(global_path) + L"�ǻ��.exe"))
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
				if (IsShortcutPointingToDirectory(entry.path().wstring(), string_to_wstring(global_path) + L"�ǻ��.exe"))
				{
					std::error_code ec;
					std::filesystem::remove(entry.path().wstring(), ec);
				}
			}
		}
		CreateShortcut(DesktopPath + L"�ǻ��(��Ļ��ע���幤��).lnk", string_to_wstring(global_path) + L"�ǻ��.exe");
	}

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return;
}