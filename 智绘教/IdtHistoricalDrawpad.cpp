#include "IdtHistoricalDrawpad.h"

#include "IdtConfiguration.h"
#include "IdtDisplayManagement.h"
#include "IdtDraw.h"
#include "IdtDrawpad.h"
#include "IdtFloating.h"
#include "IdtFreezeFrame.h"
#include "IdtImage.h"
#include "IdtMagnification.h"
#include "IdtText.h"
#include "IdtTime.h"
#include "IdtWindow.h"

void removeEmptyFolders(std::wstring path)
{
	for (const auto& entry : filesystem::directory_iterator(path)) {
		if (entry.is_directory()) {
			removeEmptyFolders(entry.path());
			if (filesystem::is_empty(entry)) {
				filesystem::remove(entry);
			}
		}
	}
}
void removeUnknownFiles(std::wstring path, std::deque<std::wstring> knownFiles)
{
	for (const auto& entry : filesystem::recursive_directory_iterator(path)) {
		if (entry.is_regular_file()) {
			auto it = std::find(knownFiles.begin(), knownFiles.end(), entry.path().wstring());
			if (it == knownFiles.end()) {
				filesystem::remove(entry);
			}
		}
	}
}
deque<wstring> getPrevTwoDays(const std::wstring& date, int day)
{
	deque<wstring> ret;

	std::wistringstream ss(date);
	std::tm t = {};
	ss >> std::get_time(&t, L"%Y-%m-%d");

	for (int i = 1; i <= day; i++)
	{
		std::mktime(&t);
		std::wostringstream os1;
		os1 << std::put_time(&t, L"%Y-%m-%d");
		ret.push_back(os1.str());

		t.tm_mday -= 1;
	}

	return ret;
}

int current_record_pointer, total_record_pointer;
int reference_record_pointer, practical_total_record_pointer;
Json::Value record_value;

//�����¼
void LoadDrawpad()
{
	if (_waccess((StringToWstring(globalPath) + L"ScreenShot\\attribute_directory.json").c_str(), 4) == 0)
	{
		Json::Reader reader;

		ifstream readjson;
		readjson.imbue(locale("zh_CN.UTF8"));
		readjson.open(WstringToString(StringToWstring(globalPath) + L"ScreenShot\\attribute_directory.json").c_str());

		deque<wstring> authenticated_file;
		authenticated_file.push_back(StringToWstring(globalPath) + L"ScreenShot\\attribute_directory.json");
		if (reader.parse(readjson, record_value))
		{
			readjson.close();

			for (int i = 0; i < (int)record_value["Image_Properties"].size(); i++)
			{
				deque<wstring> date = getPrevTwoDays(CurrentDate(), 5);

				auto it = find(date.begin(), date.end(), StringToWstring(record_value["Image_Properties"][i]["date"].asString()));
				if (it == date.end())
				{
					remove(ConvertToGbk(record_value["Image_Properties"][i]["drawpad"].asString()).c_str());
					remove(ConvertToGbk(record_value["Image_Properties"][i]["background"].asString()).c_str());
					remove(ConvertToGbk(record_value["Image_Properties"][i]["blending"].asString()).c_str());

					record_value["Image_Properties"].removeIndex(i, nullptr);
					i--;
				}
				else if (_waccess(StringToWstring(ConvertToGbk(record_value["Image_Properties"][i]["drawpad"].asString())).c_str(), 0) == -1)
				{
					remove(ConvertToGbk(record_value["Image_Properties"][i]["drawpad"].asString()).c_str());
					remove(ConvertToGbk(record_value["Image_Properties"][i]["background"].asString()).c_str());
					remove(ConvertToGbk(record_value["Image_Properties"][i]["blending"].asString()).c_str());

					record_value["Image_Properties"].removeIndex(i, nullptr);
					i--;
				}
				else
				{
					authenticated_file.push_back(StringToWstring(ConvertToGbk(record_value["Image_Properties"][i]["drawpad"].asString())));
					authenticated_file.push_back(StringToWstring(ConvertToGbk(record_value["Image_Properties"][i]["background"].asString())));
					authenticated_file.push_back(StringToWstring(ConvertToGbk(record_value["Image_Properties"][i]["blending"].asString())));
				}
			}
			removeUnknownFiles(StringToWstring(globalPath) + L"ScreenShot", authenticated_file);
			removeEmptyFolders(StringToWstring(globalPath) + L"ScreenShot");

			Json::StreamWriterBuilder OutjsonBuilder;
			OutjsonBuilder.settings_["emitUTF8"] = true;
			std::unique_ptr<Json::StreamWriter> writer(OutjsonBuilder.newStreamWriter());
			ofstream writejson;
			writejson.imbue(locale("zh_CN.UTF8"));
			writejson.open(WstringToString(StringToWstring(globalPath) + L"ScreenShot\\attribute_directory.json").c_str());
			writer->write(record_value, &writejson);
			writejson.close();
		}
		else readjson.close();
	}
	else
	{
		//����·��
		filesystem::create_directory(StringToWstring(globalPath) + L"ScreenShot");

		deque<wstring> authenticated_file;
		authenticated_file.push_back(StringToWstring(globalPath) + L"ScreenShot\\attribute_directory.json");

		removeUnknownFiles(StringToWstring(globalPath) + L"ScreenShot", authenticated_file);
		removeEmptyFolders(StringToWstring(globalPath) + L"ScreenShot");

		record_value["Image_Properties"].append(Json::Value("nullptr"));
		record_value["Image_Properties"].removeIndex(0, nullptr);

		Json::StreamWriterBuilder OutjsonBuilder;
		OutjsonBuilder.settings_["emitUTF8"] = true;
		std::unique_ptr<Json::StreamWriter> writer(OutjsonBuilder.newStreamWriter());
		ofstream writejson;
		writejson.imbue(locale("zh_CN.UTF8"));
		writejson.open(WstringToString(StringToWstring(globalPath) + L"ScreenShot\\attribute_directory.json").c_str());
		writer->write(record_value, &writejson);
		writejson.close();
	}

	current_record_pointer = reference_record_pointer = 1;
	total_record_pointer = practical_total_record_pointer = (int)record_value["Image_Properties"].size();
}
//����ͼ��ָ��Ŀ¼
void SaveScreenShot(IMAGE img, bool record_pointer_add)
{
	shared_lock<shared_mutex> DisplaysNumberLock(DisplaysNumberSm);

	wstring date = CurrentDate(), time = CurrentTime(), stamp = getTimestamp();
	if (_waccess((StringToWstring(globalPath) + L"ScreenShot").c_str(), 0 == -1)) CreateDirectory((StringToWstring(globalPath) + L"ScreenShot").c_str(), NULL);
	if (_waccess((StringToWstring(globalPath) + L"ScreenShot\\" + date).c_str(), 0 == -1)) CreateDirectory((StringToWstring(globalPath) + L"ScreenShot\\" + date).c_str(), NULL);

	saveImageToPNG(img, WstringToString(StringToWstring(globalPath) + L"ScreenShot\\" + date + L"\\" + stamp + L".png").c_str(), true, 10);

	/*
	if (magnificationReady)
	{
		RequestUpdateMagWindow = true;
		while (RequestUpdateMagWindow) this_thread::sleep_for(chrono::milliseconds(100));

		std::shared_lock<std::shared_mutex> lock1(MagnificationBackgroundSm);
		IMAGE blending = MagnificationBackground;
		lock1.unlock();
		saveImageToPNG(blending, WstringToString(StringToWstring(globalPath) + L"ScreenShot\\" + date + L"\\" + stamp + L"_background.png").c_str(), false, 10);
	}*/

	//hiex::TransparentImage(&blending, 0, 0, &img);
	//saveImageToJPG(blending, WstringToString(StringToWstring(globalPath) + L"ScreenShot\\" + date + L"\\" + stamp + L"_blending.jpg").c_str(),50);
	//saveimage((StringToWstring(globalPath) + L"ScreenShot\\" + date + L"\\" + stamp + L"_blending.jpg").c_str(), &blending);

	//ͼ��Ŀ¼��д
	if (_waccess((StringToWstring(globalPath) + L"ScreenShot\\attribute_directory.json").c_str(), 4) == 0)
	{
		{
			Json::Value set;
			set["date"] = Json::Value(ConvertToUtf8(WstringToString(date)));
			set["time"] = Json::Value(ConvertToUtf8(WstringToString(time)));
			set["drawpad"] = Json::Value(ConvertToUtf8(WstringToString(StringToWstring(globalPath) + L"ScreenShot\\" + date + L"\\" + stamp + L".png")));
			//if (magnificationReady) set["background"] = Json::Value(ConvertToUtf8(WstringToString(StringToWstring(globalPath) + L"ScreenShot\\" + date + L"\\" + stamp + L"_background.png")));
			//set["blending"] = Json::Value(ConvertToUtf8(WstringToString(StringToWstring(globalPath) + L"ScreenShot\\" + date + L"\\" + stamp + L"_blending.jpg")));

			record_value["Image_Properties"].insert(0, set);
		}

		Json::StreamWriterBuilder OutjsonBuilder;
		OutjsonBuilder.settings_["emitUTF8"] = true;
		std::unique_ptr<Json::StreamWriter> writer(OutjsonBuilder.newStreamWriter());
		ofstream writejson;
		writejson.imbue(locale("zh_CN.UTF8"));
		writejson.open(WstringToString(StringToWstring(globalPath) + L"ScreenShot\\attribute_directory.json").c_str());
		writer->write(record_value, &writejson);
		writejson.close();
	}
	else
	{
		{
			Json::Value set;
			set["date"] = Json::Value(ConvertToUtf8(WstringToString(date)));
			set["time"] = Json::Value(ConvertToUtf8(WstringToString(time)));
			set["drawpad"] = Json::Value(ConvertToUtf8(WstringToString(StringToWstring(globalPath) + L"ScreenShot\\" + date + L"\\" + stamp + L".png")));
			//if (magnificationReady) set["background"] = Json::Value(ConvertToUtf8(WstringToString(StringToWstring(globalPath) + L"ScreenShot\\" + date + L"\\" + stamp + L"_background.png")));
			//set["blending"] = Json::Value(ConvertToUtf8(WstringToString(StringToWstring(globalPath) + L"ScreenShot\\" + date + L"\\" + stamp + L"_blending.jpg")));

			record_value["Image_Properties"].append(set);
		}

		Json::StreamWriterBuilder OutjsonBuilder;
		OutjsonBuilder.settings_["emitUTF8"] = true;
		std::unique_ptr<Json::StreamWriter> writer(OutjsonBuilder.newStreamWriter());
		ofstream writejson;
		writejson.imbue(locale("zh_CN.UTF8"));
		writejson.open(WstringToString(StringToWstring(globalPath) + L"ScreenShot\\attribute_directory.json").c_str());
		writer->write(record_value, &writejson);
		writejson.close();
	}

	if (record_pointer_add) current_record_pointer++, reference_record_pointer++;
	total_record_pointer = practical_total_record_pointer = (int)record_value["Image_Properties"].size();
}

// ���ز���
void IdtRecall()
{
	// ��ʶ���Ƶȴ�
	{
		unique_lock<shared_mutex> LockdrawWaitingSm(drawWaitingSm);
		drawWaiting = true;
		LockdrawWaitingSm.unlock();
	}
	// ��ֹ��ͼʱ���س�ͻ
	{
		std::shared_lock<std::shared_mutex> LockStrokeImageListSm(StrokeImageListSm);
		bool start = !StrokeImageList.empty();
		LockStrokeImageListSm.unlock();

		//���ڻ�����ȡ������
		if (start)
		{
			// ȡ����ʶ���Ƶȴ�
			{
				unique_lock<shared_mutex> LockdrawWaitingSm(drawWaitingSm);
				drawWaiting = false;
				LockdrawWaitingSm.unlock();
			}
			return;
		}
	}

	pair<int, int> tmp_recond = make_pair(0, 0);
	int tmp_recall_image_type = 0;
	if (!RecallImage.empty())
	{
		tmp_recond = RecallImage.back().recond;
		tmp_recall_image_type = RecallImage.back().type;

		if (RecallImage.back().type == 2 && drawMode.DrawModeSelect != DrawModeSelectEnum::IdtSelection && !CompareImagesWithBuffer(&drawpad, &RecallImage.back().img));
		else RecallImage.pop_back();
		deque<RecallStruct>(RecallImage).swap(RecallImage); // ʹ��swap�������ͷ�δʹ�õ��ڴ�
	}

	if (!RecallImage.empty())
	{
		drawpad = RecallImage.back().img;
		extreme_point = RecallImage.back().extreme_point;
		recall_image_recond = RecallImage.back().recond.first;
	}
	else if (tmp_recond.first > 10)
	{
		IdtRecovery();
		return;
	}
	else
	{
		if (tmp_recall_image_type == 2)
		{
			IdtRecovery();
			return;
		}

		SetImageColor(drawpad, RGBA(0, 0, 0, 0), true);
		extreme_point.clear();
		recall_image_recond = 0;
		FirstDraw = true;
	}
	SetImageColor(window_background, RGBA(0, 0, 0, 1), true);
	hiex::TransparentImage(&window_background, 0, 0, &drawpad);

	if (drawMode.DrawModeSelect != DrawModeSelectEnum::IdtSelection)
	{
		// ����BLENDFUNCTION�ṹ��
		BLENDFUNCTION blend;
		blend.BlendOp = AC_SRC_OVER;
		blend.BlendFlags = 0;
		blend.SourceConstantAlpha = 255; // ����͸���ȣ�0Ϊȫ͸����255Ϊ��͸��
		blend.AlphaFormat = AC_SRC_ALPHA; // ʹ��Դͼ���alphaͨ��
		HDC hdcScreen = GetDC(NULL);
		// ����UpdateLayeredWindow�������´���
		POINT ptSrc = { 0,0 };
		SIZE sizeWnd = { drawpad.getwidth(),drawpad.getheight() };
		POINT ptDst = { 0,0 }; // ���ô���λ��
		UPDATELAYEREDWINDOWINFO ulwi = { 0 };
		ulwi.cbSize = sizeof(ulwi);
		ulwi.hdcDst = hdcScreen;
		ulwi.pptDst = &ptDst;
		ulwi.psize = &sizeWnd;
		ulwi.pptSrc = &ptSrc;
		ulwi.crKey = RGB(255, 255, 255);
		ulwi.pblend = &blend;
		ulwi.dwFlags = ULW_ALPHA;

		// ����Ҫ���µľ�������
		ulwi.hdcSrc = GetImageHDC(&window_background);
		UpdateLayeredWindowIndirect(drawpad_window, &ulwi);
	}
	else
	{
		reserve_drawpad = true;

		//brush.select = true;
		//rubber.select = false;
		//choose.select = false;
		drawMode.DrawModeSelect = DrawModeSelectEnum::IdtPen;
	}

	// ȡ����ʶ���Ƶȴ�
	{
		unique_lock<shared_mutex> LockdrawWaitingSm(drawWaitingSm);
		drawWaiting = false;
		LockdrawWaitingSm.unlock();
	}
	return;
}
// �����ָ�����
void IdtRecovery()
{
	if (current_record_pointer == total_record_pointer + 1)
	{
		drawMode.DrawModeSelect = DrawModeSelectEnum::IdtSelection;
		//choose.select = true;
		//brush.select = false;
		//rubber.select = false;

		reference_record_pointer = 1;
		return;
	}

	// ��ʶ���Ƶȴ�
	{
		unique_lock<shared_mutex> LockdrawWaitingSm(drawWaitingSm);
		drawWaiting = true;
		LockdrawWaitingSm.unlock();
	}
	// ��ֹ��ͼʱ���س�ͻ
	{
		std::shared_lock<std::shared_mutex> LockStrokeImageListSm(StrokeImageListSm);
		bool start = !StrokeImageList.empty();
		LockStrokeImageListSm.unlock();

		//���ڻ�����ȡ������
		if (start)
		{
			// ȡ����ʶ���Ƶȴ�
			{
				unique_lock<shared_mutex> LockdrawWaitingSm(drawWaitingSm);
				drawWaiting = false;
				LockdrawWaitingSm.unlock();
			}
			return;
		}
	}

	if (_access(ConvertToGbk(record_value["Image_Properties"][current_record_pointer - 1]["drawpad"].asString()).c_str(), 4) == -1)
	{
		// ȡ����ʶ���Ƶȴ�
		{
			unique_lock<shared_mutex> LockdrawWaitingSm(drawWaitingSm);
			drawWaiting = false;
			LockdrawWaitingSm.unlock();
		}
		return;
	}

	filesystem::path pathObj(ConvertToGbk(record_value["Image_Properties"][current_record_pointer - 1]["drawpad"].asString()));
	wstring file_name1 = pathObj.parent_path().filename().wstring();
	wstring file_name2 = pathObj.stem().wstring();

	// �������µĵ���ʱ��������к��룩��ʽ
	if (regex_match(file_name2, wregex(L"\\d+")))
	{
		chrono::milliseconds ms(_wtoll(file_name2.c_str()));
		time_t tt = std::chrono::system_clock::to_time_t(chrono::time_point<chrono::system_clock>(ms));
		RecallImageTm = *std::localtime(&tt);
	}

	// ���Ͼɰ� ʱ-��-�� ��ʽ
	else if (regex_match(file_name1 + L" " + file_name2, wregex(L"\\d{4}-\\d{2}-\\d{2} \\d{2}-\\d{2}-\\d{2}")))
	{
		std::wistringstream temp_wiss(file_name1 + L" " + file_name2);
		temp_wiss >> std::get_time(&RecallImageTm, L"%Y-%m-%d %H-%M-%S");
	}

	// ���ڴ���ʧ��
	else RecallImageTm = (tm)(NULL);
	FreezeRecall = 500;

	IMAGE temp;
	loadimage(&temp, StringToWstring(ConvertToGbk(record_value["Image_Properties"][current_record_pointer - 1]["drawpad"].asString())).c_str(), drawpad.getwidth(), drawpad.getheight(), true);
	drawpad = temp, extreme_point = map<pair<int, int>, bool>();

	current_record_pointer++;

	SetImageColor(window_background, RGBA(0, 0, 0, 1), true);
	hiex::TransparentImage(&window_background, 0, 0, &drawpad);

	if (drawMode.DrawModeSelect == DrawModeSelectEnum::IdtPen)
	{
		// ����BLENDFUNCTION�ṹ��
		BLENDFUNCTION blend;
		blend.BlendOp = AC_SRC_OVER;
		blend.BlendFlags = 0;
		blend.SourceConstantAlpha = 255; // ����͸���ȣ�0Ϊȫ͸����255Ϊ��͸��
		blend.AlphaFormat = AC_SRC_ALPHA; // ʹ��Դͼ���alphaͨ��
		HDC hdcScreen = GetDC(NULL);
		// ����UpdateLayeredWindow�������´���
		POINT ptSrc = { 0,0 };
		SIZE sizeWnd = { drawpad.getwidth(),drawpad.getheight() };
		POINT ptDst = { 0,0 }; // ���ô���λ��
		UPDATELAYEREDWINDOWINFO ulwi = { 0 };
		ulwi.cbSize = sizeof(ulwi);
		ulwi.hdcDst = hdcScreen;
		ulwi.pptDst = &ptDst;
		ulwi.psize = &sizeWnd;
		ulwi.pptSrc = &ptSrc;
		ulwi.crKey = RGB(255, 255, 255);
		ulwi.pblend = &blend;
		ulwi.dwFlags = ULW_ALPHA;

		// ����Ҫ���µľ�������
		ulwi.hdcSrc = GetImageHDC(&window_background);
		UpdateLayeredWindowIndirect(drawpad_window, &ulwi);
	}
	else
	{
		reserve_drawpad = true;

		//brush.select = true;
		//rubber.select = false;
		//choose.select = false;
		drawMode.DrawModeSelect = DrawModeSelectEnum::IdtPen;
	}

	// ȡ����ʶ���Ƶȴ�
	{
		unique_lock<shared_mutex> LockdrawWaitingSm(drawWaitingSm);
		drawWaiting = false;
		LockdrawWaitingSm.unlock();
	}
	return;
}