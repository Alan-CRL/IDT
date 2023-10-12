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
								// �����к�ת��Ϊ wstring
								serialNumber = vtProp.bstrVal;

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