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
#include "IdtD2DPreparation.h"
#include "IdtDisplayManagement.h"
#include "IdtDrawpad.h"
#include "IdtGuid.h"
#include "IdtImage.h"
#include "IdtMagnification.h"
#include "IdtOther.h"
#include "IdtPlug-in.h"
#include "IdtRts.h"
#include "IdtSetting.h"
#include "IdtState.h"
#include "IdtSysNotifications.h"
#include "IdtText.h"
#include "IdtTime.h"
#include "IdtUpdate.h"
#include "IdtWindow.h"

#include <lm.h>
#include <shellscalingapi.h>
#include <shlobj.h>
#pragma comment(lib, "netapi32.lib")

int floating_main();
int drawpad_main();
int SettingMain();
void FreezeFrameWindow();

wstring buildTime = __DATE__ L" " __TIME__;		//����ʱ��
string editionDate = "20240605a";				//���򷢲�����
string editionChannel = "Dev";					//���򷢲�ͨ��
string editionCode = "24H2";					//����汾 (BetaH3)

wstring userId; //�û�ID���������кţ�
string globalPath; //����ǰ·��

int offSignal = false, offSignalReady = false; //�ر�ָ��
map <wstring, bool> threadStatus; //�߳�״̬����

shared_ptr<spdlog::logger> IDTLogger;

// ������ڵ�
int WINAPI wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	// ·��Ԥ����
	{
		globalPath = WstringToString(GetCurrentExeDirectory() + L"\\");

		if (!HasReadWriteAccess(StringToWstring(globalPath)))
		{
			if (IsUserAnAdmin()) MessageBox(NULL, L"��ǰĿ¼Ȩ�������޷��������У��뽫����ת��������Ŀ¼��������", L"�ǻ����ʾ", MB_SYSTEMMODAL | MB_OK);
			else ShellExecute(NULL, L"runas", GetCurrentExePath().c_str(), NULL, NULL, SW_SHOWNORMAL);
			return 0;
		}
	}
#ifdef IDT_RELEASE
	// ��ֹ�ظ�����
	{
		if (_waccess((StringToWstring(globalPath) + L"force_start.signal").c_str(), 0) == 0)
		{
			error_code ec;
			filesystem::remove(StringToWstring(globalPath) + L"force_start.signal", ec);
		}
		else if (ProcessRunningCnt(GetCurrentExePath()) > 1) return 0;
	}
#endif

	// �û�ID��ȡ
	{
		userId = StringToWstring(getDeviceGUID());
		if (userId.empty() || !isValidString(userId)) userId = L"Error";
	}
	// ��־�����ʼ��
	{
		wstring Timestamp = getTimestamp();

		error_code ec;
		if (_waccess((StringToWstring(globalPath) + L"log").c_str(), 0) == -1) filesystem::create_directory(StringToWstring(globalPath) + L"log", ec);
		else
		{
			// ��ʷ��־����

			auto getCurrentTimeStamp = []()
				{
					auto now = chrono::system_clock::now();
					auto duration = now.time_since_epoch();
					return chrono::duration_cast<chrono::milliseconds>(duration).count();
				};

			auto isLogFile = [](const string& filename)
				{
					regex pattern("idt\\d+\\.log");
					return regex_match(filename, pattern);
				};

			auto getTimeStampFromFilename = [](const string& filename)
				{
					regex pattern("idt(\\d+)\\.log");

					smatch match;
					if (regex_search(filename, match, pattern))
					{
						string timestampStr = match[1];
						return stoll(timestampStr);
					}
					return -1LL;
				};

			auto isOldLogFile = [&getCurrentTimeStamp, &getTimeStampFromFilename](const filesystem::path& filepath)
				{
					time_t currentTimeStamp = getCurrentTimeStamp();
					time_t fileTimeStamp = getTimeStampFromFilename(filepath.filename().string());

					if (fileTimeStamp == -1) return false;
					return (currentTimeStamp - fileTimeStamp) >= (7LL * 24LL * 60LL * 60LL * 1000LL) || (currentTimeStamp - fileTimeStamp) < 0; // 7��ĺ�����
				};

			auto calculateDirectorySize = [](const filesystem::path& directoryPath)
				{
					uintmax_t totalSize = 0;
					for (const auto& entry : filesystem::directory_iterator(directoryPath))
					{
						if (entry.is_regular_file()) {
							totalSize += entry.file_size();
						}
					}
					return totalSize;
				};

			auto deleteOldLogFiles = [&isLogFile, &isOldLogFile, &calculateDirectorySize](const filesystem::path& directory)
				{
					uintmax_t totalSize = calculateDirectorySize(directory);

					for (const auto& entry : filesystem::directory_iterator(directory))
					{
						if (entry.is_regular_file())
						{
							if (isLogFile(entry.path().filename().string()) && (totalSize > 10485760LL || isOldLogFile(entry.path())))
							{
								uintmax_t entrySize = entry.file_size();

								error_code ec;
								filesystem::remove(entry.path(), ec);

								if (!ec) totalSize -= entrySize;
							}
						}
					}
				};

			string directoryPath = globalPath + "log";

			filesystem::path directory(directoryPath);
			if (filesystem::exists(directory) && filesystem::is_directory(directory))
			{
				deleteOldLogFiles(directory);
			}
		}

		if (_waccess((StringToWstring(globalPath) + L"log\\idt" + Timestamp + L".log").c_str(), 0) == 0) filesystem::remove(StringToWstring(globalPath) + L"log\\idt" + Timestamp + L".log", ec);

		auto IDTLoggerFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(globalPath + "log\\idt" + WstringToString(Timestamp) + ".log");

		spdlog::init_thread_pool(8192, 64);
		IDTLogger = std::make_shared<spdlog::async_logger>("IDTLogger", IDTLoggerFileSink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);

		IDTLogger->set_level(spdlog::level::info);
		IDTLogger->set_pattern("[%l][%H:%M:%S.%e]%v");

		IDTLogger->flush_on(spdlog::level::info);
		IDTLogger->info("[���߳�][IdtMain] ��־��ʼ��¼ " + WstringToString(userId));

		//logger->info("");
		//logger->warn("");
		//logger->error("");
		//logger->critical("");
	}
	// �����Զ�����
	{
		if (_waccess((StringToWstring(globalPath) + L"update.json").c_str(), 4) == 0)
		{
			wstring tedition, representation;
			string thash_md5, thash_sha256;
			wstring old_name;

			bool flag = true;

			Json::Reader reader;
			Json::Value root;

			ifstream readjson;
			readjson.imbue(locale("zh_CN.UTF8"));
			readjson.open(WstringToString(StringToWstring(globalPath) + L"update.json").c_str());

			if (reader.parse(readjson, root))
			{
				if (root.isMember("edition")) tedition = StringToWstring(ConvertToGbk(root["edition"].asString()));
				else flag = false;

				if (root.isMember("representation")) representation = StringToWstring(ConvertToGbk(root["representation"].asString()));
				else flag = false;

				if (root.isMember("hash"))
				{
					if (root["hash"].isMember("md5")) thash_md5 = ConvertToGbk(root["hash"]["md5"].asString());
					else flag = false;
					if (root["hash"].isMember("sha256")) thash_sha256 = ConvertToGbk(root["hash"]["sha256"].asString());
					else flag = false;
				}
				else flag = false;

				if (root.isMember("old_name")) old_name = StringToWstring(ConvertToGbk(root["old_name"].asString()));
			}
			readjson.close();

			string hash_md5, hash_sha256;
			{
				hashwrapper* myWrapper = new md5wrapper();
				hash_md5 = myWrapper->getHashFromFileW(GetCurrentExePath());
				delete myWrapper;
			}
			{
				hashwrapper* myWrapper = new sha256wrapper();
				hash_sha256 = myWrapper->getHashFromFileW(GetCurrentExePath());
				delete myWrapper;
			}

			if (flag && tedition == StringToWstring(editionDate) && hash_md5 == thash_md5 && hash_sha256 == thash_sha256)
			{
				//������������ʼ�滻�汾

				this_thread::sleep_for(chrono::milliseconds(1000));

				filesystem::path directory(globalPath);
				string main_path = directory.parent_path().parent_path().string();

				error_code ec;
				if (!old_name.empty()) filesystem::remove(StringToWstring(main_path) + L"\\" + old_name, ec);
				else filesystem::remove(StringToWstring(main_path) + L"\\�ǻ��.exe", ec);
				filesystem::copy_file(StringToWstring(globalPath) + representation, StringToWstring(main_path) + L"\\�ǻ��.exe", std::filesystem::copy_options::overwrite_existing, ec);

				ShellExecute(NULL, NULL, (StringToWstring(main_path) + L"\\�ǻ��.exe").c_str(), NULL, NULL, SW_SHOWNORMAL);

				return 0;
			}
			else
			{
				error_code ec;
				filesystem::remove(StringToWstring(globalPath) + L"update.json", ec);

				filesystem::path directory(globalPath);
				string main_path = directory.parent_path().parent_path().string();
				if (!old_name.empty()) ShellExecute(NULL, NULL, (StringToWstring(main_path) + L"\\" + old_name).c_str(), NULL, NULL, SW_SHOWNORMAL);
				else ShellExecute(NULL, NULL, (StringToWstring(main_path) + L"\\�ǻ��.exe").c_str(), NULL, NULL, SW_SHOWNORMAL);

				return 0;
			}
		}
		if (_waccess((StringToWstring(globalPath) + L"installer\\update.json").c_str(), 4) == 0)
		{
			wstring tedition, path;
			string thash_md5, thash_sha256;

			bool flag = true;

			Json::Reader reader;
			Json::Value root;

			ifstream readjson;
			readjson.imbue(locale("zh_CN.UTF8"));
			readjson.open(WstringToString(StringToWstring(globalPath) + L"installer\\update.json").c_str());

			if (reader.parse(readjson, root))
			{
				if (root.isMember("edition")) tedition = StringToWstring(ConvertToGbk(root["edition"].asString()));
				else flag = false;

				if (root.isMember("path")) path = StringToWstring(ConvertToGbk(root["path"].asString()));
				else flag = false;

				if (root.isMember("hash"))
				{
					if (root["hash"].isMember("md5")) thash_md5 = ConvertToGbk(root["hash"]["md5"].asString());
					else flag = false;
					if (root["hash"].isMember("sha256")) thash_sha256 = ConvertToGbk(root["hash"]["sha256"].asString());
					else flag = false;
				}
				else flag = false;
			}

			readjson.close();

			string hash_md5, hash_sha256;
			{
				hashwrapper* myWrapper = new md5wrapper();
				hash_md5 = myWrapper->getHashFromFileW(StringToWstring(globalPath) + path);
				delete myWrapper;
			}
			{
				hashwrapper* myWrapper = new sha256wrapper();
				hash_sha256 = myWrapper->getHashFromFileW(StringToWstring(globalPath) + path);
				delete myWrapper;
			}

			if (flag && tedition > StringToWstring(editionDate) && _waccess((StringToWstring(globalPath) + path).c_str(), 0) == 0 && hash_md5 == thash_md5 && hash_sha256 == thash_sha256)
			{
				//������������ʼ�滻�汾
				{
					root["old_name"] = Json::Value(ConvertToUtf8(WstringToString(GetCurrentExeName())));

					Json::StreamWriterBuilder outjson;
					outjson.settings_["emitUTF8"] = true;
					std::unique_ptr<Json::StreamWriter> writer(outjson.newStreamWriter());
					ofstream writejson;
					writejson.imbue(locale("zh_CN.UTF8"));
					writejson.open(WstringToString(StringToWstring(globalPath) + L"installer\\update.json").c_str());
					writer->write(root, &writejson);
					writejson.close();
				}

				if (ExtractResource((StringToWstring(globalPath) + L"Inkeys.png").c_str(), L"PNG_ICON", MAKEINTRESOURCE(236)))
				{
					IdtSysNotificationsImageAndText04(L"�ǻ��", 5000, StringToWstring(globalPath) + L"Inkeys.png", L"�ǻ�������Զ����£������ĵȴ�", L"��ͨ�� MD5 �� SHA265 ������У��", L"�汾�� " + StringToWstring(editionDate) + L" -> " + tedition);

					error_code ec;
					filesystem::remove(StringToWstring(globalPath) + L"Inkeys.png", ec);
				}
				ShellExecute(NULL, NULL, (StringToWstring(globalPath) + path).c_str(), NULL, NULL, SW_SHOWNORMAL);

				return 0;
			}
			else if (tedition == StringToWstring(editionDate))
			{
				std::error_code ec;
				filesystem::remove_all(StringToWstring(globalPath) + L"installer", ec);
				filesystem::remove_all(StringToWstring(globalPath) + L"api", ec);

				filesystem::remove(StringToWstring(globalPath) + L"PptCOM.dll", ec);
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
	// �����ͼ���ʼ��
	{
		D2DStarup();
	}
	// �����ʼ��
	{
		IDTLogger->info("[���߳�][IdtMain] ��ʼ������");

		IDTLogger->info("[���߳�][IdtMain] ��������");

		INT numFound = 0;
		HRSRC hRes = ::FindResource(NULL, MAKEINTRESOURCE(198), L"TTF");
		HGLOBAL hMem = ::LoadResource(NULL, hRes);
		DWORD dwSize = ::SizeofResource(NULL, hRes);

		fontCollection.AddMemoryFont(hMem, dwSize);
		fontCollection.GetFamilies(1, &HarmonyOS_fontFamily, &numFound);

		{
			if (_waccess((StringToWstring(globalPath) + L"ttf").c_str(), 0) == -1)
			{
				error_code ec;
				filesystem::create_directory(StringToWstring(globalPath) + L"ttf", ec);
			}
			ExtractResource((StringToWstring(globalPath) + L"ttf\\hmossscr.ttf").c_str(), L"TTF", MAKEINTRESOURCE(198));

			IdtFontCollectionLoader* D2DFontCollectionLoader = new IdtFontCollectionLoader;
			D2DFontCollectionLoader->AddFont(D2DTextFactory, StringToWstring(globalPath) + L"ttf\\hmossscr.ttf");

			D2DTextFactory->RegisterFontCollectionLoader(D2DFontCollectionLoader);
			D2DTextFactory->CreateCustomFontCollection(D2DFontCollectionLoader, 0, 0, &D2DFontCollection);
			D2DTextFactory->UnregisterFontCollectionLoader(D2DFontCollectionLoader);
		}

		IDTLogger->info("[���߳�][IdtMain] �����������");

		//filesystem::create_directory(StringToWstring(globalPath) + L"ttf");
		//ExtractResource((StringToWstring(globalPath) + L"ttf\\HarmonyOS_Sans_SC_Regular.ttf").c_str(), L"TTF", MAKEINTRESOURCE(198));
		//fontCollection.AddFontFile((StringToWstring(globalPath) + L"ttf\\HarmonyOS_Sans_SC_Regular.ttf").c_str());
		//filesystem::path directory((StringToWstring(globalPath) + L"ttf").c_str());
		//filesystem::remove_all(directory);

		//AddFontResourceEx((StringToWstring(globalPath) + L"ttf\\HarmonyOS_Sans_SC_Regular.ttf").c_str(), FR_PRIVATE, NULL);
		//AddFontResourceEx((StringToWstring(globalPath) + L"ttf\\Douyu_Font.otf").c_str(), FR_PRIVATE, NULL);
		//AddFontResourceEx((StringToWstring(globalPath) + L"ttf\\SmileySans-Oblique.ttf").c_str(), FR_PRIVATE, NULL);

		//wcscpy(font.lfFaceName, L"HarmonyOS Sans SC");
		//wcscpy(font.lfFaceName, L"DOUYU Gdiplus::Font");
		//wcscpy(font.lfFaceName, L"�����");

		IDTLogger->info("[���߳�][IdtMain] ��������");

		stringFormat.SetAlignment(StringAlignmentCenter);
		stringFormat.SetLineAlignment(StringAlignmentCenter);
		stringFormat.SetFormatFlags(StringFormatFlagsNoWrap);

		stringFormat_left.SetAlignment(StringAlignmentNear);
		stringFormat_left.SetLineAlignment(StringAlignmentNear);
		stringFormat_left.SetFormatFlags(StringFormatFlagsNoWrap);

		IDTLogger->info("[���߳�][IdtMain] �����������");
		IDTLogger->info("[���߳�][IdtMain] ��ʼ���������");
	}
	// ��������Ϣ��ʼ��
	{
		IDTLogger->info("[���߳�][IdtMain] ��ʼ����������Ϣ");

		// ��ʾ�����
		IDTLogger->info("[���߳�][IdtMain] ��������Ϣ��ѯ");
		DisplayManagementMain();
		IDTLogger->info("[���߳�][IdtMain] ��������Ϣ��ѯ���");

		shared_lock<shared_mutex> DisplaysNumberLock(DisplaysNumberSm);
		int DisplaysNumberTemp = DisplaysNumber;
		DisplaysNumberLock.unlock();

		IDTLogger->info("[���߳�][IdtMain] MagnifierThread�����߳�����");
		thread MagnifierThread_thread(MagnifierThread);
		MagnifierThread_thread.detach();

		if (DisplaysNumberTemp > 1) IDTLogger->warn("[���߳�][IdtMain] ӵ�ж��������");

		IDTLogger->info("[���߳�][IdtMain] ��ʼ����������Ϣ���");
	}
	// ������Ϣ��ʼ��
	{
		IDTLogger->info("[���߳�][IdtMain] ��ʼ��������Ϣ");

		if (_waccess((StringToWstring(globalPath) + L"opt\\deploy.json").c_str(), 4) == -1)
		{
			IDTLogger->warn("[���߳�][IdtMain] ������Ϣ������");

			IDTLogger->info("[���߳�][IdtMain] ����������Ϣ");
			FirstSetting(true);
			IDTLogger->info("[���߳�][IdtMain] ����������Ϣ���");

			{
				shared_lock<shared_mutex> DisplaysNumberLock(DisplaysNumberSm);
				int DisplaysNumberTemp = DisplaysNumber;
				DisplaysNumberLock.unlock();
				if (DisplaysNumberTemp > 1)
					MessageBox(floating_window, (L"��⵽�����ӵ�� " + to_wstring(DisplaysNumberTemp) + L" ����ʾ�����ǻ��Ŀǰ��֧������չ��ʾ���ϻ�ͼ��\n����������ʾ���ϻ�ͼ��").c_str(), L"�ǻ�̾���", MB_OK | MB_SYSTEMMODAL);
			}
		}

		IDTLogger->info("[���߳�][IdtMain] ��ȡ������Ϣ");
		ReadSetting(true);
		IDTLogger->info("[���߳�][IdtMain] ��ȡ������Ϣ���");

		IDTLogger->info("[���߳�][IdtMain] ����������Ϣ");
		WriteSetting();
		IDTLogger->info("[���߳�][IdtMain] ����������Ϣ���");
	}
	// ������ó�ʼ��
	{
		// ���� DesktopDrawpadBlocker
		thread(StartDesktopDrawpadBlocker).detach();
	}
	// COM��ʼ��
	HANDLE hActCtx;
	ULONG_PTR ulCookie;
	{
		IDTLogger->info("[���߳�][IdtMain] ��ʼ��COM");

		IDTLogger->info("[���߳�][IdtMain] ��ʼ��CoInitialize");
		CoInitialize(NULL);
		IDTLogger->info("[���߳�][IdtMain] ��ʼ��CoInitialize���");

		//PptCOM �������
		{
			IDTLogger->info("[���߳�][IdtMain] ��ʼ��PptCOM.dll");
			ExtractResource((StringToWstring(globalPath) + L"PptCOM.dll").c_str(), L"DLL", MAKEINTRESOURCE(222));
			IDTLogger->info("[���߳�][IdtMain] ��ʼ��PptCOM.dll���");

			IDTLogger->info("[���߳�][IdtMain] ��ʼ��������API");

			ACTCTX actCtx = { 0 };
			actCtx.cbSize = sizeof(actCtx);
			actCtx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_HMODULE_VALID;
			actCtx.lpResourceName = MAKEINTRESOURCE(221);
			actCtx.hModule = GetModuleHandle(NULL);

			hActCtx = CreateActCtx(&actCtx);
			ActivateActCtx(hActCtx, &ulCookie);

			IDTLogger->info("[���߳�][IdtMain] ��ʼ��������API���");

			IDTLogger->info("[���߳�][IdtMain] ����PptCOM.dll");
			HMODULE hModule = LoadLibrary((StringToWstring(globalPath) + L"PptCOM.dll").c_str());
			IDTLogger->info("[���߳�][IdtMain] ����PptCOM.dll���");
		}

		IDTLogger->info("[���߳�][IdtMain] ��ʼ��COM���");
	}
	//�����ݷ�ʽ��ʼ��
	if (setlist.CreateLnk)
	{
		IDTLogger->info("[���߳�][IdtMain] ��ʼ�������ݷ�ʽ");
		SetShortcut();
		IDTLogger->info("[���߳�][IdtMain] ��ʼ�������ݷ�ʽ���");
	}

	// ����
	{
		wstring ClassName;
		if (userId == L"Error") ClassName = L"IdtHiEasyX";
		else ClassName = userId;

		IDTLogger->info("[���߳�][IdtMain] �������񱳾�����");
		hiex::PreSetWindowShowState(SW_HIDE);
		freeze_window = hiex::initgraph_win32(MainMonitor.MonitorWidth, MainMonitor.MonitorHeight, 0, L"Idt4 FreezeWindow", ClassName.c_str());
		IDTLogger->info("[���߳�][IdtMain] �������񱳾��������");

		IDTLogger->info("[���߳�][IdtMain] �������崰��");
		hiex::PreSetWindowShowState(SW_HIDE);
		drawpad_window = hiex::initgraph_win32(MainMonitor.MonitorWidth, MainMonitor.MonitorHeight, 0, L"Idt3 DrawpadWindow", ClassName.c_str(), nullptr, freeze_window);
		IDTLogger->info("[���߳�][IdtMain] �������崰�����");

		IDTLogger->info("[���߳�][IdtMain] ����PPT��ע�ؼ�����");
		hiex::PreSetWindowShowState(SW_HIDE);
		ppt_window = hiex::initgraph_win32(MainMonitor.MonitorWidth, MainMonitor.MonitorHeight, 0, L"Idt2 PptWindow", ClassName.c_str(), nullptr, drawpad_window);
		IDTLogger->info("[���߳�][IdtMain] ����PPT��ע�ؼ��������");

		IDTLogger->info("[���߳�][IdtMain] ��������������");
		hiex::PreSetWindowShowState(SW_HIDE);
		floating_window = hiex::initgraph_win32(background.getwidth(), background.getheight(), 0, L"Idt1 FloatingWindow", ClassName.c_str(), nullptr, ppt_window);
		IDTLogger->info("[���߳�][IdtMain] �����������������");

		// ���崰����ע�� RTS ǰ����ӵ���ö����ԣ�����ʾǰ�Ƚ���һ��ȫ���ö�
		SetWindowPos(freeze_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		// ���崰����ǰ�����䲻��ӵ�н������ʽ����ע�� RTS
		{
			while (!(GetWindowLong(drawpad_window, GWL_EXSTYLE) & WS_EX_NOACTIVATE))
			{
				SetWindowLong(drawpad_window, GWL_EXSTYLE, GetWindowLong(drawpad_window, GWL_EXSTYLE) | WS_EX_NOACTIVATE);
				if (GetWindowLong(drawpad_window, GWL_EXSTYLE) & WS_EX_NOACTIVATE) break;

				this_thread::sleep_for(chrono::milliseconds(10));
			}

			SetWindowPos(drawpad_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}

		IDTLogger->info("[���߳�][IdtMain] TopWindow�����߳�����");
		thread TopWindowThread(TopWindow);
		TopWindowThread.detach();
	}
	// RealTimeStylus���ؿ�
	{
		IDTLogger->info("[���߳�][IdtMain] ��ʼ��RTS���ؿ�");
		// Create RTS object
		g_pRealTimeStylus = CreateRealTimeStylus(drawpad_window);
		if (g_pRealTimeStylus == NULL)
		{
			IDTLogger->warn("[���߳�][IdtMain] RealTimeStylus Ϊ NULL");

			uRealTimeStylus = -1;
			goto RealTimeStylusEnd;
		}

		// Create EventHandler object
		g_pSyncEventHandlerRTS = CSyncEventHandlerRTS::Create(g_pRealTimeStylus);
		if (g_pSyncEventHandlerRTS == NULL)
		{
			IDTLogger->warn("[���߳�][IdtMain] SyncEventHandlerRTS Ϊ NULL");

			g_pRealTimeStylus->Release();
			g_pRealTimeStylus = NULL;

			uRealTimeStylus = -2;
			goto RealTimeStylusEnd;
		}

		// Enable RTS
		if (!EnableRealTimeStylus(g_pRealTimeStylus))
		{
			IDTLogger->warn("[���߳�][IdtMain] ���� RTS ʧ��");

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

			offSignal = true;

			// ����ʼ�� COM ����
			CoUninitialize();

			return 0;
		}

		IDTLogger->info("[���߳�][IdtMain] RTSSpeed�����߳�����");
		thread RTSSpeed_thread(RTSSpeed);
		RTSSpeed_thread.detach();

		IDTLogger->info("[���߳�][IdtMain] ��ʼ��RTS���ؿ����");
	}
	// �߳�
	{
		IDTLogger->info("[���߳�][IdtMain] floating_main�����߳�����");
		thread floating_main_thread(floating_main);
		floating_main_thread.detach();

		IDTLogger->info("[���߳�][IdtMain] drawpad_main�����߳�����");
		thread drawpad_main_thread(drawpad_main);
		drawpad_main_thread.detach();

		IDTLogger->info("[���߳�][IdtMain] SettingMain�����߳�����");
		thread test_main_thread(SettingMain);
		test_main_thread.detach();

		IDTLogger->info("[���߳�][IdtMain] FreezeFrameWindow�����߳�����");
		thread FreezeFrameWindow_thread(FreezeFrameWindow);
		FreezeFrameWindow_thread.detach();

		IDTLogger->info("[���߳�][IdtMain] StateMonitoring�����߳�����");
		thread(StateMonitoring).detach();
	}

	while (!offSignal) this_thread::sleep_for(chrono::milliseconds(500));

	IDTLogger->info("[���߳�][IdtMain] �ȴ��������߳̽���");

	int WaitingCount = 0;
	for (; WaitingCount < 20; WaitingCount++)
	{
		if (!threadStatus[L"floating_main"] && !threadStatus[L"drawpad_main"] && !threadStatus[L"SettingMain"] && !threadStatus[L"FreezeFrameWindow"] && !threadStatus[L"NetUpdate"]) break;
		this_thread::sleep_for(chrono::milliseconds(500));
	}
	if (WaitingCount >= 20) IDTLogger->warn("[���߳�][IdtMain] ���������̳߳�ʱ��ǿ�ƽ����߳�");

	// ����ʼ�� COM ����
	{
		IDTLogger->info("[���߳�][IdtMain] ����ʼ�� COM ����");

		IDTLogger->info("[���߳�][IdtMain] ��ʼ��CoUninitialize");
		CoUninitialize();
		IDTLogger->info("[���߳�][IdtMain] ��ʼ��CoUninitialize���");

		IDTLogger->info("[���߳�][IdtMain] �ͷ�������API");
		DeactivateActCtx(0, ulCookie);
		ReleaseActCtx(hActCtx);
		IDTLogger->info("[���߳�][IdtMain] �ͷ�������API���");

		IDTLogger->info("[���߳�][IdtMain] ����ʼ�� COM �������");
	}

#ifdef IDT_RELEASE
	IDTLogger->info("[���߳�][IdtMain] �ȴ������������ֽ���");
	while (!offSignalReady) this_thread::sleep_for(chrono::milliseconds(500));
	IDTLogger->info("[���߳�][IdtMain] �����������ֽ���");
#endif

	IDTLogger->info("[���߳�][IdtMain] �ѽ����ǻ�������̲߳��رճ���");
	return 0;
}

// ����ר��
#ifndef IDT_RELEASE
void Test()
{
	MessageBox(NULL, L"��Ǵ�", L"���", MB_OK | MB_SYSTEMMODAL);
}
void Testi(long long t)
{
	MessageBox(NULL, to_wstring(t).c_str(), L"��ֵ���", MB_OK | MB_SYSTEMMODAL);
}
void Testw(wstring t)
{
	MessageBoxW(NULL, t.c_str(), L"�ַ����", MB_OK | MB_SYSTEMMODAL);
}
void Testa(string t)
{
	MessageBoxA(NULL, t.c_str(), "�ַ����", MB_OK | MB_SYSTEMMODAL);
}
#endif