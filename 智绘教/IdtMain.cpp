/*
 * @file		IdtMain.cpp
 * @brief		�ǻ����Ŀ����Դ�ļ�
 * @note		���ڳ�ʼ���ǻ�̲��������ģ��
 *
 * @envir		VisualStudio 2022 | MSVC 143 | .NET Framework 4.0 | EasyX_20230723 | Windows 11
 * @site		https://github.com/Alan-CRL/Intelligent-Drawing-Teaching
 *
 * @author		Alan-CRL
 * @qq			2685549821
 * @email		alan-crl@foxmail.com
*/

#include "IdtMain.h"

#include "IdtConfiguration.h"
#include "IdtDisplayManagement.h"
#include "IdtDrawpad.h"
#include "IdtImage.h"
#include "IdtMagnification.h"
#include "IdtOther.h"
#include "IdtRts.h"
#include "IdtSetting.h"
#include "IdtText.h"
#include "IdtUpdate.h"
#include "IdtWindow.h"

#include <shellscalingapi.h>
#include <lm.h>
#pragma comment(lib, "netapi32.lib")

int floating_main();
int drawpad_main();
int SettingMain();
void FreezeFrameWindow();

bool already = false;

wstring buildTime = __DATE__ L" " __TIME__; //����ʱ��
string edition_date = "20240324a"; //���򷢲�����
string edition_code = "24H1(BetaH2)"; //����汾

wstring userid; //�û�ID���������кţ�
string global_path; //����ǰ·��

double server_updata_error, procedure_updata_error;
wstring server_updata_error_reason;

bool off_signal = false; //�ر�ָ��
map <wstring, bool> thread_status; //�߳�״̬����

//����ר��
#ifndef IDT_RELEASE
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
#endif

// ������ڵ�
int main()
{
	//ȫ��·��Ԥ����
	{
		global_path = wstring_to_string(GetCurrentExeDirectory() + L"\\");
	}
	//������־��ʼ��
	//{
	//	std::string logFilePath = global_path + "IDT.log";
	//
	//	// Create a logger with a specified file name, and a maximum size of 3MB per file
	//	auto logger = spdlog::basic_logger_st("logger", logFilePath, 1024 * 1024 * 3);
	//	// Set the logger to only output to the file, not to the console
	//	logger->set_level(spdlog::level::trace);
	//	logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%L%$] [thread %t] %v");
	//}
	//logger->info("��ɳ�����־��ʼ��");

	//LOG(INFO) << "���Գ�ʼ��DPI";
	//DPI ��ʼ��
	{
		HMODULE hShcore = LoadLibrary(L"Shcore.dll");
		if (hShcore != NULL)
		{
			typedef HRESULT(WINAPI* LPFNSPDPIA)(PROCESS_DPI_AWARENESS);
			LPFNSPDPIA lSetProcessDpiAwareness = (LPFNSPDPIA)GetProcAddress(hShcore, "SetProcessDpiAwareness");
			if (lSetProcessDpiAwareness != NULL)
			{
				HRESULT hr = lSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
				if (!SUCCEEDED(hr)) SetProcessDPIAware();
			}
			else SetProcessDPIAware();

			FreeLibrary(hShcore);
		}
		else SetProcessDPIAware();

		//ͼ��DPIת��
		{
			alpha_drawpad.Resize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
			tester.Resize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
			pptdrawpad.Resize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		}
	}
	//LOG(INFO) << "�ɹ���ʼ��DPI";

	//LOG(INFO) << "���Գ�ʼ������";
	//�����ʼ������
	{
		HMODULE hModule = GetModuleHandle(NULL);
		HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(198), L"TTF");
		HGLOBAL hMemory = LoadResource(hModule, hResource);
		PVOID pResourceData = LockResource(hMemory);
		DWORD dwResourceSize = SizeofResource(hModule, hResource);
		fontCollection.AddMemoryFont(pResourceData, dwResourceSize);

		INT numFound = 0;
		fontCollection.GetFamilies(1, &HarmonyOS_fontFamily, &numFound);

		//filesystem::create_directory(string_to_wstring(global_path) + L"ttf");
		//ExtractResource((string_to_wstring(global_path) + L"ttf\\HarmonyOS_Sans_SC_Regular.ttf").c_str(), L"TTF", MAKEINTRESOURCE(198));
		//fontCollection.AddFontFile((string_to_wstring(global_path) + L"ttf\\HarmonyOS_Sans_SC_Regular.ttf").c_str());
		//filesystem::path directory((string_to_wstring(global_path) + L"ttf").c_str());
		//filesystem::remove_all(directory);

		//AddFontResourceEx((string_to_wstring(global_path) + L"ttf\\HarmonyOS_Sans_SC_Regular.ttf").c_str(), FR_PRIVATE, NULL);
		//AddFontResourceEx((string_to_wstring(global_path) + L"ttf\\Douyu_Font.otf").c_str(), FR_PRIVATE, NULL);
		//AddFontResourceEx((string_to_wstring(global_path) + L"ttf\\SmileySans-Oblique.ttf").c_str(), FR_PRIVATE, NULL);

		//wcscpy(font.lfFaceName, L"HarmonyOS Sans SC");
		//wcscpy(font.lfFaceName, L"DOUYU Gdiplus::Font");
		//wcscpy(font.lfFaceName, L"�����");

		stringFormat.SetAlignment(StringAlignmentCenter);
		stringFormat.SetLineAlignment(StringAlignmentCenter);
		stringFormat.SetFormatFlags(StringFormatFlagsNoWrap);

		stringFormat_left.SetAlignment(StringAlignmentNear);
		stringFormat_left.SetLineAlignment(StringAlignmentNear);
		stringFormat_left.SetFormatFlags(StringFormatFlagsNoWrap);
	}
	//LOG(INFO) << "�ɹ���ʼ������";

	//LOG(INFO) << "���Լ��������";
	//��������ж�
	{
		//��ǰ����Ϊ�°汾
		if (_waccess((string_to_wstring(global_path) + L"update.json").c_str(), 4) == 0)
		{
			wstring tedition, representation;

			Json::Reader reader;
			Json::Value root;

			ifstream readjson;
			readjson.imbue(locale("zh_CN.UTF8"));
			readjson.open(wstring_to_string(string_to_wstring(global_path) + L"update.json").c_str());

			if (reader.parse(readjson, root))
			{
				tedition = string_to_wstring(convert_to_gbk(root["edition"].asString()));
				representation = string_to_wstring(convert_to_gbk(root["representation"].asString()));
			}

			readjson.close();

			if (tedition == string_to_wstring(edition_date))
			{
				//������������ʼ�滻�汾

				Sleep(1000);

				filesystem::path directory(global_path);
				string main_path = directory.parent_path().parent_path().string();

				error_code ec;
				filesystem::remove(string_to_wstring(main_path) + L"\\�ǻ��.exe", ec);
				filesystem::copy_file(string_to_wstring(global_path) + representation, string_to_wstring(main_path) + L"\\�ǻ��.exe", std::filesystem::copy_options::overwrite_existing, ec);

				STARTUPINFOA si = { 0 };
				si.cb = sizeof(si);
				PROCESS_INFORMATION pi = { 0 };
				CreateProcessA(NULL, (main_path + "\\�ǻ��.exe").data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);

				return 0;
			}
		}
		//��ǰ����Ϊ�ɰ汾
		if (_waccess((string_to_wstring(global_path) + L"installer\\update.json").c_str(), 4) == 0)
		{
			wstring tedition, path;
			string thash_md5, thash_sha256;

			Json::Reader reader;
			Json::Value root;

			ifstream readjson;
			readjson.imbue(locale("zh_CN.UTF8"));
			readjson.open(wstring_to_string(string_to_wstring(global_path) + L"installer\\update.json").c_str());

			if (reader.parse(readjson, root))
			{
				tedition = string_to_wstring(convert_to_gbk(root["edition"].asString()));
				path = string_to_wstring(convert_to_gbk(root["path"].asString()));

				thash_md5 = convert_to_gbk(root["hash"]["md5"].asString());
				thash_sha256 = convert_to_gbk(root["hash"]["sha256"].asString());
			}

			readjson.close();

			string hash_md5, hash_sha256;
			{
				hashwrapper* myWrapper = new md5wrapper();
				hash_md5 = myWrapper->getHashFromFile(global_path + wstring_to_string(path));
				delete myWrapper;
			}
			{
				hashwrapper* myWrapper = new sha256wrapper();
				hash_sha256 = myWrapper->getHashFromFile(global_path + wstring_to_string(path));
				delete myWrapper;
			}

			if (tedition > string_to_wstring(edition_date) && _waccess((string_to_wstring(global_path) + path).c_str(), 0) == 0 && hash_md5 == thash_md5 && hash_sha256 == thash_sha256)
			{
				//������������ʼ�滻�汾

				STARTUPINFOA si = { 0 };
				si.cb = sizeof(si);
				PROCESS_INFORMATION pi = { 0 };
				CreateProcessA(NULL, (global_path + wstring_to_string(path)).data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);

				return 0;
			}
			else if (tedition == string_to_wstring(edition_date))
			{
				std::error_code ec;
				filesystem::remove_all(string_to_wstring(global_path) + L"installer", ec);
				filesystem::remove_all(string_to_wstring(global_path) + L"api", ec);

				filesystem::remove(string_to_wstring(global_path) + L"PptCOM.dll", ec);
			}
		}
	}
	//LOG(INFO) << "�ɹ����������";

	//LOG(INFO) << "���Ի�ȡ������Ϣ";
	//��ʼ����Ϣ��ȡ
	{
		if (_waccess((string_to_wstring(global_path) + L"opt\\deploy.json").c_str(), 4) == -1) FirstSetting(true);

		ReadSetting(true);
		WriteSetting();
	}
	//LOG(INFO) << "�ɹ���ȡ������Ϣ";

	//LOG(INFO) << "���Գ�ʼ��COM";
	CoInitialize(NULL);
	//LOG(INFO) << "�ɹ���ʼ��COM";

	//LOG(INFO) << "���Ի�ȡ�û�ID";
	userid = GetMainHardDiskSerialNumber();
	if (userid.empty() || !isValidString(userid)) userid = L"�޷���ȷʶ��";
	//LOG(INFO) << "�ɹ���ȡ�û�ID";

	//��ʾ�����
	DisplayManagementMain();

	shared_lock<shared_mutex> DisplaysNumberLock(DisplaysNumberSm);
	int DisplaysNumberTemp = DisplaysNumber;
	DisplaysNumberLock.unlock();

	if (DisplaysNumberTemp)
	{
		thread MagnifierThread_thread(MagnifierThread);
		MagnifierThread_thread.detach();
	}
	else MessageBox(floating_window, (L"��⵽�����ӵ�� " + to_wstring(DisplaysNumberTemp) + L" ����ʾ�����ǻ��Ŀǰ��֧��ӵ����չ��ʾ�����ԣ�\n\n���򽫼��������������ڶ�����ʷ���屣�棬�����ָ����ܽ�ʧЧ��\n�ҽ���������ʾ���ϻ�ͼ��").c_str(), L"�ǻ�̾���", MB_OK | MB_SYSTEMMODAL);

	//�����ݷ�ʽע��
	if (setlist.CreateLnk) SetShortcut();
	// ��ݼ�ע��
	// thread RegisterHotkeyThread(RegisterHotkey);
	// RegisterHotkeyThread.detach();

	// ��������

	//LOG(INFO) << "���Դ�������������";
	hiex::PreSetWindowShowState(SW_HIDE);
	floating_window = initgraph(background.getwidth(), background.getheight());
	//LOG(INFO) << "�ɹ���������������";

	//LOG(INFO) << "���Դ������ʴ���";
	hiex::PreSetWindowShowState(SW_HIDE);
	drawpad_window = initgraph(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	//LOG(INFO) << "�ɹ��������ʴ���";

	//LOG(INFO) << "���Դ���PPT�ؼ�";
	hiex::PreSetWindowShowState(SW_HIDE);
	ppt_window = initgraph(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	//LOG(INFO) << "�ɹ�����PPT�ؼ�";

	//LOG(INFO) << "���Դ�����������";
	hiex::PreSetWindowShowState(SW_HIDE);
	freeze_window = initgraph(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	//LOG(INFO) << "�ɹ�������������";

	SetWindowPos(floating_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(ppt_window, floating_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(drawpad_window, ppt_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(freeze_window, drawpad_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(setting_window, freeze_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	thread TopWindowThread(TopWindow);
	TopWindowThread.detach();

	//LOG(INFO) << "���Լ���PptCOM���";
	//PptCOM �������
	HANDLE hActCtx;
	ULONG_PTR ulCookie;
	{
		ExtractResource((string_to_wstring(global_path) + L"PptCOM.dll").c_str(), L"DLL", MAKEINTRESOURCE(222));

		ACTCTX actCtx = { 0 };
		actCtx.cbSize = sizeof(actCtx);
		actCtx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_HMODULE_VALID;
		actCtx.lpResourceName = MAKEINTRESOURCE(221);
		actCtx.hModule = GetModuleHandle(NULL);

		hActCtx = CreateActCtx(&actCtx);
		ActivateActCtx(hActCtx, &ulCookie);

		HMODULE hModule = LoadLibrary((string_to_wstring(global_path) + L"PptCOM.dll").c_str());
	}
	//LOG(INFO) << "�ɹ�����PptCOM���";

	//LOG(INFO) << "���Գ�ʼ��RTS���ؿ�";
	// ��ʼ�� RealTimeStylus ���ؿ�
	{
		// Create RTS object
		g_pRealTimeStylus = CreateRealTimeStylus(drawpad_window);
		if (g_pRealTimeStylus == NULL)
		{
			uRealTimeStylus = -1;
			goto RealTimeStylusEnd;
		}

		// Create EventHandler object
		g_pSyncEventHandlerRTS = CSyncEventHandlerRTS::Create(g_pRealTimeStylus);
		if (g_pSyncEventHandlerRTS == NULL)
		{
			g_pRealTimeStylus->Release();
			g_pRealTimeStylus = NULL;

			uRealTimeStylus = -2;
			goto RealTimeStylusEnd;
		}

		// Enable RTS
		if (!EnableRealTimeStylus(g_pRealTimeStylus))
		{
			g_pSyncEventHandlerRTS->Release();
			g_pSyncEventHandlerRTS = NULL;

			g_pRealTimeStylus->Release();
			g_pRealTimeStylus = NULL;

			uRealTimeStylus = -3;
			goto RealTimeStylusEnd;
		}

		if (uRealTimeStylus == 0) uRealTimeStylus = 1;

	RealTimeStylusEnd:

		if (uRealTimeStylus <= 0)
		{
			MessageBox(NULL, (L"���ؿ� RTS ��ʼ��ʧ�ܣ�����ֹͣ���У�\nRTS_Err" + to_wstring(-uRealTimeStylus)).c_str(), L"����", MB_OK | MB_SYSTEMMODAL);

			off_signal = true;

			// ����ʼ�� COM ����
			CoUninitialize();

			return 0;
		}

		thread RTSSpeed_thread(RTSSpeed);
		RTSSpeed_thread.detach();
	}
	//LOG(INFO) << "�ɹ���ʼ��RTS���ؿ�";

	//LOG(INFO) << "�������������������߳�";
	thread floating_main_thread(floating_main);
	floating_main_thread.detach();
	//LOG(INFO) << "�ɹ����������������߳�";
	//LOG(INFO) << "�����������ʴ����߳�";
	thread drawpad_main_thread(drawpad_main);
	drawpad_main_thread.detach();
	//LOG(INFO) << "�ɹ��������ʴ����߳�";

	//LOG(INFO) << "��������ѡ����߳�";
	thread test_main_thread(SettingMain);
	test_main_thread.detach();

	//LOG(INFO) << "�ɹ�����ѡ����߳�";
	//LOG(INFO) << "�����������������߳�";
	thread FreezeFrameWindow_thread(FreezeFrameWindow);
	FreezeFrameWindow_thread.detach();
	//LOG(INFO) << "�ɹ��������������߳�";

	//LOG(INFO) << "����������������ע���߳�";
	//thread NetUpdate_thread(NetUpdate);
	//NetUpdate_thread.detach();
	//LOG(INFO) << "�ɹ�������������ע���߳�";

	while (!off_signal) Sleep(500);

	//LOG(INFO) << "���Խ������̣߳����رճ���";

	int i = 1;
	for (; i <= 20; i++)
	{
		if (!thread_status[L"floating_main"] && !thread_status[L"drawpad_main"] && !thread_status[L"SettingMain"] && !thread_status[L"FreezeFrameWindow"] && !thread_status[L"NetUpdate"]) break;
		Sleep(500);
	}

	// ����ʼ�� COM ����
	CoUninitialize();
	DeactivateActCtx(0, ulCookie);
	ReleaseActCtx(hActCtx);

	//if (i > 10) LOG(ERROR) << "ʧ�ܽ������̣߳�����ǿ���˳�";
	//else LOG(INFO) << "�ɹ��������̣߳����رճ���";

	return 0;
}