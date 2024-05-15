#include "IdtUpdate.h"

#include "IdtConfiguration.h"
#include "IdtHash.h"
#include "IdtOther.h"
#include "IdtSetting.h"
#include "IdtText.h"
#include "IdtTime.h"
#include "IdtWindow.h"

//�����������
void CrashedHandler()
{
	threadStatus[L"CrashedHandler"] = true;

	ofstream write;
	write.imbue(locale("zh_CN.UTF8"));

	write.open(WstringToString(StringToWstring(globalPath) + L"api\\open.txt").c_str());
	write << 1;
	write.close();

	// ����������������
	{
		bool start = true;

		// ��鱾���ļ�������
		{
			if (_waccess((StringToWstring(globalPath) + L"api").c_str(), 0) == -1)
			{
				error_code ec;
				filesystem::create_directory(StringToWstring(globalPath) + L"api", ec);
			}

			if (_waccess((StringToWstring(globalPath) + L"api\\�ǻ��CrashedHandler.exe").c_str(), 0) == -1)
				if (!ExtractResource((StringToWstring(globalPath) + L"api\\�ǻ��CrashedHandler.exe").c_str(), L"EXE", MAKEINTRESOURCE(201)))
					start = false;
			if (_waccess((StringToWstring(globalPath) + L"api\\api\\�ǻ��CrashedHandlerClose.exe").c_str(), 0) == -1)
				if (!ExtractResource((StringToWstring(globalPath) + L"api\\�ǻ��CrashedHandlerClose.exe").c_str(), L"EXE", MAKEINTRESOURCE(202)))
					start = false;

			{
				string hash_md5, hash_sha256;
				{
					hashwrapper* myWrapper = new md5wrapper();
					hash_md5 = myWrapper->getHashFromFileW(StringToWstring(globalPath) + L"api\\�ǻ��CrashedHandler.exe");
					delete myWrapper;
				}
				{
					hashwrapper* myWrapper = new sha256wrapper();
					hash_sha256 = myWrapper->getHashFromFileW(StringToWstring(globalPath) + L"api\\�ǻ��CrashedHandler.exe");
					delete myWrapper;
				}

				if (hash_md5 != CrashedHandlerMd5 || hash_sha256 != CrashedHandlerSHA256)
					if (!ExtractResource((StringToWstring(globalPath) + L"api\\�ǻ��CrashedHandler.exe").c_str(), L"EXE", MAKEINTRESOURCE(201)))
						start = false;
			}
			{
				string hash_md5, hash_sha256;
				{
					hashwrapper* myWrapper = new md5wrapper();
					hash_md5 = myWrapper->getHashFromFileW(StringToWstring(globalPath) + L"api\\�ǻ��CrashedHandlerClose.exe");
					delete myWrapper;
				}
				{
					hashwrapper* myWrapper = new sha256wrapper();
					hash_sha256 = myWrapper->getHashFromFileW(StringToWstring(globalPath) + L"api\\�ǻ��CrashedHandlerClose.exe");
					delete myWrapper;
				}

				if (hash_md5 != CrashedHandlerCloseMd5 || hash_sha256 != CrashedHandlerCloseSHA256)
					if (!ExtractResource((StringToWstring(globalPath) + L"api\\�ǻ��CrashedHandlerClose.exe").c_str(), L"EXE", MAKEINTRESOURCE(202)))
						start = false;
			}
		}

		// ������������
		if (start && !isProcessRunning((StringToWstring(globalPath) + L"api\\�ǻ��CrashedHandler.exe").c_str()))
		{
			wstring path = GetCurrentExePath();
			ShellExecute(NULL, NULL, (StringToWstring(globalPath) + L"api\\�ǻ��CrashedHandler.exe").c_str(), (L"/\"" + path + L"\"").c_str(), NULL, SW_SHOWNORMAL);
		}
	}

	int value = 2;
	while (!offSignal)
	{
		write.open(WstringToString(StringToWstring(globalPath) + L"api\\open.txt").c_str());
		write << value;
		write.close();

		value++;
		if (value > 100000000) value = 2;

		this_thread::sleep_for(chrono::milliseconds(1000));
	}

	if (offSignal == 2)
	{
		write.open(WstringToString(StringToWstring(globalPath) + L"api\\open.txt").c_str());
		write << -2;
		write.close();
	}
	else
	{
		error_code ec;
		filesystem::remove(StringToWstring(globalPath) + L"api\\open.txt", ec);
	}

	threadStatus[L"CrashedHandler"] = false;
	offSignalReady = true;
}

//�����Զ�����
int AutomaticUpdateStep = 0;
wstring get_domain_name(wstring url) {
	wregex pattern(L"([a-zA-z]+://[^/]+)");
	wsmatch match;
	if (regex_search(url, match, pattern)) return match[0].str();
	else return L"";
}
wstring convertToHttp(const wstring& url)
{
	//���±���
	//if (getCurrentDate() >= L"20231020") return url;

	wstring httpPrefix = L"http://";
	if (url.length() >= 7 && url.compare(0, 7, httpPrefix) == 0) return url;
	else if (url.length() >= 8 && url.compare(0, 8, L"https://") == 0) return httpPrefix + url.substr(8);
	else return httpPrefix + url;
}
void AutomaticUpdate()
{
	/*
	AutomaticUpdateStep ����
	0 �����Զ�����δ����
	1 �����Զ�����������
	2 �����Զ������������Ӵ���
	3 �����Զ������������°汾��Ϣʱʧ��
	4 �����Զ��������ص����°汾��Ϣ�����Ϲ淶
	5 �����Զ��������ص����°汾��Ϣ�в��������õ�ͨ��
	6 �°汾������������
	7 �����Զ������������°汾����ʧ��
	8 �����Զ��������ص����°汾������
	9 �������µ����°汾
	10 �����Ѿ������°汾
	*/

	//this_thread::sleep_for(chrono::seconds(3));
	//while (!already) this_thread::sleep_for(chrono::milliseconds(50));

	struct
	{
		wstring editionDate;
		wstring editionCode;
		wstring explain;
		wstring representation;
		wstring path[10];
		int path_size;

		string hash_md5, hash_sha256;
	}info;

	string channel = setlist.UpdateChannel;

	bool state = true;
	bool update = true;

	bool against = false;

	while (!offSignal)
	{
		AutomaticUpdateStep = 1;

		state = true;
		against = false;

		//��ȡ���°汾��Ϣ
		if (state)
		{
			error_code ec;
			filesystem::create_directory(StringToWstring(globalPath) + L"installer", ec); //����·��
			filesystem::remove(StringToWstring(globalPath) + L"installer\\new_download.json", ec);

			info.editionDate = L"";
			info.editionCode = L"";
			info.explain = L"";
			info.representation = L"";
			info.hash_md5 = "";
			info.hash_sha256 = "";
			info.path_size = 0;

			HRESULT download1;
			{
				DeleteUrlCacheEntry(L"http://home.alan-crl.top");
				download1 = URLDownloadToFileW( // ���������������ݵ������ļ�
					nullptr,                  // �����д nullptr ����
					(L"http://home.alan-crl.top/version_identification/official_version.json?timestamp=" + getTimestamp()).c_str(), // ������д����ַ
					StringToLPCWSTR(globalPath + "installer\\new_download.json"),            // �ļ���д����
					0,                        // д 0 �Ͷ���
					nullptr                   // Ҳ�ǣ�������д nullptr ����
				);
			}
			if (download1 != S_OK)
			{
				//�������ص�ַ
				DeleteUrlCacheEntry(L"https://vip.123pan.cn");
				download1 = URLDownloadToFileW( // ���������������ݵ������ļ�
					nullptr,                  // �����д nullptr ����
					(L"https://vip.123pan.cn/1709404/version_identification/official_version.json?timestamp=" + getTimestamp()).c_str(), // ������д����ַ
					StringToLPCWSTR(globalPath + "installer\\new_download.json"),            // �ļ���д����
					0,                        // д 0 �Ͷ���
					nullptr                   // Ҳ�ǣ�������д nullptr ����
				);
			}

			if (download1 == S_OK)
			{
				Json::Reader reader;
				Json::Value root;

				ifstream injson;
				injson.imbue(locale("zh_CN.UTF8"));
				injson.open(WstringToString(StringToWstring(globalPath) + L"installer\\new_download.json").c_str());

				if (reader.parse(injson, root))
				{
					int flag = 0;

					if (root.isMember(channel))
					{
						if (root[channel].isMember("edition_date")) info.editionDate = StringToWstring(ConvertToGbk(root[channel]["edition_date"].asString()));
						else flag = 1;

						if (root[channel].isMember("edition_code")) info.editionCode = StringToWstring(ConvertToGbk(root[channel]["edition_code"].asString()));
						else flag = 1;

						if (root[channel].isMember("explain")) info.explain = StringToWstring(ConvertToGbk(root[channel]["explain"].asString()));

						if (root[channel].isMember("hash"))
						{
							if (root[channel]["hash"].isMember("md5")) info.hash_md5 = ConvertToGbk(root[channel]["hash"]["md5"].asString());
							else flag = 1;

							if (root[channel]["hash"].isMember("sha256")) info.hash_sha256 = ConvertToGbk(root[channel]["hash"]["sha256"].asString());
							else flag = 1;
						}
						else flag = 1;

						if (root[channel].isMember("path"))
						{
							info.path_size = root[channel]["path"].size();
							for (int i = 0; i < min(info.path_size, 10); i++)
							{
								info.path[i] = StringToWstring(ConvertToGbk(root[channel]["path"][i].asString()));
							}

							if (info.path_size <= 0) flag = 1;
						}
						else flag = 1;

						if (root[channel].isMember("representation")) info.representation = StringToWstring(ConvertToGbk(root[channel]["representation"].asString()));
						else flag = 1;
					}
					if (!root.isMember(channel) && root.size() >= 1)
					{
						Json::Value::Members members = root.getMemberNames();
						channel = members[0];

						if (root[channel].isMember("edition_date")) info.editionDate = StringToWstring(ConvertToGbk(root[channel]["edition_date"].asString()));
						else flag = 1;

						if (root[channel].isMember("edition_code")) info.editionCode = StringToWstring(ConvertToGbk(root[channel]["edition_code"].asString()));
						else flag = 1;

						if (root[channel].isMember("explain")) info.explain = StringToWstring(ConvertToGbk(root[channel]["explain"].asString()));

						if (root[channel].isMember("hash"))
						{
							if (root[channel]["hash"].isMember("md5")) info.hash_md5 = ConvertToGbk(root[channel]["hash"]["md5"].asString());
							else flag = 1;

							if (root[channel]["hash"].isMember("sha256")) info.hash_sha256 = ConvertToGbk(root[channel]["hash"]["sha256"].asString());
							else flag = 1;
						}
						else flag = 1;

						if (root[channel].isMember("path"))
						{
							info.path_size = root[channel]["path"].size();
							for (int i = 0; i < min(info.path_size, 10); i++)
							{
								info.path[i] = StringToWstring(ConvertToGbk(root[channel]["path"][i].asString()));
							}

							if (info.path_size <= 0) flag = 2;
						}
						else flag = 1;

						if (root[channel].isMember("representation")) info.representation = StringToWstring(ConvertToGbk(root[channel]["representation"].asString()));
						else flag = 1;

						if (!flag)
						{
							setlist.UpdateChannel = channel;
							WriteSetting();
						}
					}

					if (flag)
					{
						AutomaticUpdateStep = 5;
						state = false;
					}
				}
				else
				{
					AutomaticUpdateStep = 4;
					state = false;
				}

				injson.close();
			}
			else
			{
				state = false;
				if (!checkIsNetwork()) AutomaticUpdateStep = 2;
				else AutomaticUpdateStep = 3;
			}

			filesystem::remove(StringToWstring(globalPath) + L"installer\\new_download.json", ec);
		}

		//�������°汾
		if (state && info.editionDate != L"" && info.editionDate > StringToWstring(editionDate))
		{
			update = true;
			if (_waccess((StringToWstring(globalPath) + L"installer\\update.json").c_str(), 4) == 0)
			{
				wstring tedition, tpath;
				string thash_md5, thash_sha256;

				Json::Reader reader;
				Json::Value root;

				ifstream readjson;
				readjson.imbue(locale("zh_CN.UTF8"));
				readjson.open(WstringToString(StringToWstring(globalPath) + L"installer\\update.json").c_str());

				if (reader.parse(readjson, root))
				{
					tedition = StringToWstring(ConvertToGbk(root["edition"].asString()));
					tpath = StringToWstring(ConvertToGbk(root["path"].asString()));

					thash_md5 = ConvertToGbk(root["hash"]["md5"].asString());
					thash_sha256 = ConvertToGbk(root["hash"]["sha256"].asString());
				}

				readjson.close();

				string hash_md5, hash_sha256;
				{
					hashwrapper* myWrapper = new md5wrapper();
					hash_md5 = myWrapper->getHashFromFileW(StringToWstring(globalPath) + tpath);
					delete myWrapper;
				}
				{
					hashwrapper* myWrapper = new sha256wrapper();
					hash_sha256 = myWrapper->getHashFromFileW(StringToWstring(globalPath) + tpath);
					delete myWrapper;
				}

				if (tedition >= info.editionDate && _waccess((StringToWstring(globalPath) + tpath).c_str(), 0) == 0 && hash_md5 == thash_md5 && hash_sha256 == thash_sha256)
				{
					update = false;
				}
			}

			if (update)
			{
				AutomaticUpdateStep = 6;

				error_code ec;
				filesystem::create_directory(StringToWstring(globalPath) + L"installer", ec); //����·��
				filesystem::remove(StringToWstring(globalPath) + L"installer\\new_procedure.tmp", ec);

				against = true;
				for (int i = 0; i < info.path_size; i++)
				{
					DeleteUrlCacheEntry(convertToHttp(get_domain_name(info.path[i])).c_str());

					HRESULT download2;
					{
						download2 = URLDownloadToFileW( // ���������������ݵ������ļ�
							nullptr,                  // �����д nullptr ����
							(convertToHttp(info.path[i]) + L"?timestamp=" + getTimestamp()).c_str(), // ������д����ַ
							StringToLPCWSTR(globalPath + "installer\\new_procedure.tmp"),            // �ļ���д����
							0,                        // д 0 �Ͷ���
							nullptr                   // Ҳ�ǣ�������д nullptr ����
						);
					}

					if (download2 == S_OK)
					{
						wstring timestamp = getTimestamp();

						error_code ec;
						filesystem::remove(StringToWstring(globalPath) + L"installer\\new_procedure" + timestamp + L".exe", ec);
						filesystem::remove(StringToWstring(globalPath) + L"installer\\" + info.representation + L".exe", ec);

						filesystem::rename(StringToWstring(globalPath) + L"installer\\new_procedure.tmp", StringToWstring(globalPath) + L"installer\\new_procedure" + timestamp + L".zip", ec);
						if (ec) continue;

						{
							HZIP hz = OpenZip((StringToWstring(globalPath) + L"installer\\new_procedure" + timestamp + L".zip").c_str(), 0);
							SetUnzipBaseDir(hz, (StringToWstring(globalPath) + L"installer\\").c_str());
							ZIPENTRY ze;
							GetZipItem(hz, -1, &ze);
							int numitems = ze.index;
							for (int i = 0; i < numitems; i++)
							{
								GetZipItem(hz, i, &ze);
								UnzipItem(hz, i, ze.name);
							}
							CloseZip(hz);
						}

						filesystem::remove(StringToWstring(globalPath) + L"installer\\new_procedure" + timestamp + L".zip", ec);
						filesystem::rename(StringToWstring(globalPath) + L"installer\\" + info.representation, StringToWstring(globalPath) + L"installer\\new_procedure" + timestamp + L".exe", ec);
						if (ec)
						{
							AutomaticUpdateStep = 8;
							continue;
						}

						string hash_md5, hash_sha256;
						{
							hashwrapper* myWrapper = new md5wrapper();
							hash_md5 = myWrapper->getHashFromFileW(StringToWstring(globalPath) + L"installer\\new_procedure" + timestamp + L".exe");
							delete myWrapper;
						}
						{
							hashwrapper* myWrapper = new sha256wrapper();
							hash_sha256 = myWrapper->getHashFromFileW(StringToWstring(globalPath) + L"installer\\new_procedure" + timestamp + L".exe");
							delete myWrapper;
						}

						//���� update.json �ļ���ָʾ����
						if (info.hash_md5 == hash_md5 && info.hash_sha256 == hash_sha256)
						{
							Json::Value root;

							root["edition"] = Json::Value(WstringToString(info.editionDate));
							root["path"] = Json::Value("installer\\new_procedure" + WstringToString(timestamp) + ".exe");
							root["representation"] = Json::Value("new_procedure" + WstringToString(timestamp) + ".exe");

							root["hash"]["md5"] = Json::Value(info.hash_md5);
							root["hash"]["sha256"] = Json::Value(info.hash_sha256);

							root["old_name"] = Json::Value(ConvertToUtf8("�ǻ��.exe"));

							Json::StreamWriterBuilder outjson;
							outjson.settings_["emitUTF8"] = true;
							std::unique_ptr<Json::StreamWriter> writer(outjson.newStreamWriter());
							ofstream writejson;
							writejson.imbue(locale("zh_CN.UTF8"));
							writejson.open(WstringToString(StringToWstring(globalPath) + L"installer\\update.json").c_str());
							writer->write(root, &writejson);
							writejson.close();

							against = false;
							AutomaticUpdateStep = 9;

							break;
						}
						else
						{
							AutomaticUpdateStep = 8;

							error_code ec;
							filesystem::remove(StringToWstring(globalPath) + L"installer\\new_procedure" + timestamp + L".exe", ec);
						}
					}
					else AutomaticUpdateStep = 7;
				}
			}
		}
		else if (state && info.editionDate != L"" && info.editionDate <= StringToWstring(editionDate)) AutomaticUpdateStep = 10;

		if (against)
		{
			for (int i = 1; i <= 10; i++)
			{
				if (offSignal) break;
				if (channel != setlist.UpdateChannel)
				{
					channel = setlist.UpdateChannel;
					break;
				}

				this_thread::sleep_for(chrono::seconds(1));
			}
		}
		else
		{
			for (int i = 1; i <= 1800; i++)
			{
				if (offSignal) break;
				if (channel != setlist.UpdateChannel)
				{
					channel = setlist.UpdateChannel;
					break;
				}

				this_thread::sleep_for(chrono::seconds(1));
			}
		}
	}
}