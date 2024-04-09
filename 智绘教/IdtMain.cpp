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
string edition_date = "20240409a(Beta)"; //���򷢲�����
string edition_code = "24H1(BetaH2)"; //����汾

wstring userid; //�û�ID���������кţ�
string global_path; //����ǰ·��

double server_updata_error, procedure_updata_error;
wstring server_updata_error_reason;

int off_signal = false, off_signal_ready = false; //�ر�ָ��
map <wstring, bool> thread_status; //�߳�״̬����

shared_ptr<spdlog::logger> IDTLogger;

// ������ڵ�
int main()
{
	// ·��Ԥ����
	{
		global_path = wstring_to_string(GetCurrentExeDirectory() + L"\\");

		if (!HasReadWriteAccess(string_to_wstring(global_path)))
		{
			if (IsUserAnAdmin()) MessageBox(NULL, L"��ǰĿ¼Ȩ�������޷��������У��뽫����ת��������Ŀ¼", L"�ǻ����ʾ", MB_OK);
			else ShellExecute(NULL, L"runas", GetCurrentExePath().c_str(), NULL, NULL, SW_SHOWNORMAL);
			return 0;
		}
	}
	// �û�ID��ȡ
	{
		userid = GetMainHardDiskSerialNumber();
		if (userid.empty() || !isValidString(userid)) userid = L"ID�޷���ȷʶ��";
	}
	// ��־�����ʼ��
	{
		error_code ec;
		if (_waccess((string_to_wstring(global_path) + L"log").c_str(), 0) == -1) filesystem::create_directory(string_to_wstring(global_path) + L"log", ec);
		if (_waccess((string_to_wstring(global_path) + L"log\\idt.log").c_str(), 0) == 0) filesystem::remove(string_to_wstring(global_path) + L"log\\idt.log", ec);

		auto IDTLoggerFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(global_path + "log\\idt.log");

		spdlog::init_thread_pool(8192, 64);
		IDTLogger = std::make_shared<spdlog::async_logger>("IDTLogger", IDTLoggerFileSink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);

		IDTLogger->set_level(spdlog::level::info);
		IDTLogger->set_pattern("[%l][%H:%M:%S.%e]%v");

		IDTLogger->flush_on(spdlog::level::info);
		IDTLogger->info("[���߳�][IdtMain] ��־��ʼ��¼ " + wstring_to_string(userid));

		//logger->info("");
		//logger->warn("");
		//logger->error("");
		//logger->critical("");
	}
	// �����Զ�����
	{
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

	// DPI��ʼ��
	{
		IDTLogger->info("[���߳�][IdtMain] ��ʼ��DPI");

		IDTLogger->info("[���߳�][IdtMain] ����Shcore.dll");
		HMODULE hShcore = LoadLibrary(L"Shcore.dll");
		if (hShcore != NULL)
		{
			IDTLogger->info("[���߳�][IdtMain] ����Shcore.dll�ɹ�");

			IDTLogger->info("[���߳�][IdtMain] ��ѯ�ӿ�SetProcessDpiAwareness");
			typedef HRESULT(WINAPI* LPFNSPDPIA)(PROCESS_DPI_AWARENESS);
			LPFNSPDPIA lSetProcessDpiAwareness = (LPFNSPDPIA)GetProcAddress(hShcore, "SetProcessDpiAwareness");
			if (lSetProcessDpiAwareness != NULL)
			{
				IDTLogger->info("[���߳�][IdtMain] ��ѯ�ӿ�SetProcessDpiAwareness�ɹ�");

				IDTLogger->info("[���߳�][IdtMain] ִ��SetProcessDpiAwareness");
				HRESULT hr = lSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
				if (!SUCCEEDED(hr))
				{
					IDTLogger->warn("[���߳�][IdtMain] ִ��SetProcessDpiAwarenessʧ��");
					if (!SetProcessDPIAware()) IDTLogger->error("[���߳�][IdtMain] ����SetProcessDPIAwareʧ��");
				}
				else IDTLogger->info("[���߳�][IdtMain] ִ��SetProcessDpiAwareness�ɹ�");
			}
			else
			{
				IDTLogger->warn("[���߳�][IdtMain] ��ѯ�ӿ�SetProcessDpiAwarenessʧ��");
				if (!SetProcessDPIAware()) IDTLogger->error("[���߳�][IdtMain] ����SetProcessDPIAwareʧ��");
			}

			FreeLibrary(hShcore);
		}
		else
		{
			IDTLogger->warn("[���߳�][IdtMain] ����Shcore.dllʧ��");
			if (!SetProcessDPIAware()) IDTLogger->error("[���߳�][IdtMain] ����SetProcessDPIAwareʧ��");
		}

		//ͼ��DPIת��
		{
			alpha_drawpad.Resize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
			tester.Resize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
			pptdrawpad.Resize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		}
		IDTLogger->info("[���߳�][IdtMain] ��ʼ��DPI���");
	}
	// �����ʼ��
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
	// ������Ϣ��ʼ��
	{
		if (_waccess((string_to_wstring(global_path) + L"opt\\deploy.json").c_str(), 4) == -1) FirstSetting(true);

		ReadSetting(true);
		WriteSetting();
	}
	// COM��ʼ��
	HANDLE hActCtx;
	ULONG_PTR ulCookie;
	{
		CoInitialize(NULL);

		//PptCOM �������
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
	}
	// ��������Ϣ��ʼ��
	{
		// ��ʾ�����
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
	}
	//�����ݷ�ʽ��ʼ��
	if (setlist.CreateLnk) SetShortcut();

	// ����
	{
		hiex::PreSetWindowShowState(SW_HIDE);
		floating_window = initgraph(background.getwidth(), background.getheight());
		hiex::PreSetWindowShowState(SW_HIDE);
		ppt_window = initgraph(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		hiex::PreSetWindowShowState(SW_HIDE);
		drawpad_window = initgraph(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		hiex::PreSetWindowShowState(SW_HIDE);
		freeze_window = initgraph(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

		SetWindowPos(floating_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		SetWindowPos(ppt_window, floating_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		SetWindowPos(drawpad_window, ppt_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		SetWindowPos(freeze_window, drawpad_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		SetWindowPos(setting_window, freeze_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		thread TopWindowThread(TopWindow);
		TopWindowThread.detach();
	}
	// RealTimeStylus���ؿ�
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
	// �߳�
	{
		thread floating_main_thread(floating_main);
		floating_main_thread.detach();
		thread drawpad_main_thread(drawpad_main);
		drawpad_main_thread.detach();
		thread test_main_thread(SettingMain);
		test_main_thread.detach();
		thread FreezeFrameWindow_thread(FreezeFrameWindow);
		FreezeFrameWindow_thread.detach();
	}

	while (!off_signal) Sleep(500);
	int WaitingCount = 0;
	for (; WaitingCount < 20; WaitingCount++)
	{
		if (!thread_status[L"floating_main"] && !thread_status[L"drawpad_main"] && !thread_status[L"SettingMain"] && !thread_status[L"FreezeFrameWindow"] && !thread_status[L"NetUpdate"]) break;
		Sleep(500);
	}

	// ����ʼ�� COM ����
	CoUninitialize();
	DeactivateActCtx(0, ulCookie);
	ReleaseActCtx(hActCtx);
	while (!off_signal_ready) Sleep(500);

	return 0;
}
// ·��Ȩ�޼��
bool HasReadWriteAccess(const std::wstring& directoryPath)
{
	DWORD attributes = GetFileAttributesW(directoryPath.c_str());
	if (attributes == INVALID_FILE_ATTRIBUTES) return false;
	if (!(attributes & FILE_ATTRIBUTE_DIRECTORY)) return false;
	if (attributes & FILE_ATTRIBUTE_READONLY) return false;
	if (attributes & FILE_ATTRIBUTE_READONLY) return false;

	return true;
}

// ����ר��
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