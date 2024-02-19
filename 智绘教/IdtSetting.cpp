#include "IdtSetting.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb-master/stb_image.h"

IMAGE SettingSign[5];
WNDCLASSEX ImGuiWc;
ID3D11ShaderResourceView* TextureSettingSign[5];
int SettingMainMode = 1;

int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);//��ȡ��ʾ���Ŀ�
int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);//��ȡ��ʾ���ĸ�

int SettingWindowX = (ScreenWidth - SettingWindowWidth) / 2;
int SettingWindowY = (ScreenHeight - SettingWindowHeight) / 2;
int SettingWindowWidth = 900;
int SettingWindowHeight = 700;

/*
LRESULT CALLBACK TestWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		return 0;

	default:
		return HIWINDOW_DEFAULT_PROC;
	}
	return 0;
}
void ControlTestMain()
{
	ExMessage m;
	while (!off_signal)
	{
		hiex::getmessage_win32(&m, EM_MOUSE, setting_window);

		if (SettingMainMode == 1 && IsInRect(m.x, m.y, { 220, 625, 980, 675 }))
		{
			if (m.lbutton)
			{
				int lx = m.x, ly = m.y;
				while (1)
				{
					ExMessage m = hiex::getmessage_win32(EM_MOUSE, setting_window);
					if (IsInRect(m.x, m.y, { 20, 625, 780, 675 }))
					{
						if (!m.lbutton)
						{
							if (setlist.experimental_functions) setlist.experimental_functions = false;
							else setlist.experimental_functions = true;

							{
								Json::StyledWriter outjson;
								Json::Value root;

								root["edition"] = Json::Value(edition_date);
								root["StartUp"] = Json::Value(setlist.StartUp);
								root["experimental_functions"] = Json::Value(setlist.experimental_functions);

								ofstream writejson;
								writejson.imbue(locale("zh_CN.UTF8"));
								writejson.open(wstring_to_string(string_to_wstring(global_path) + L"opt\\deploy.json").c_str());
								writejson << outjson.write(root);
								writejson.close();
							}

							break;
						}
					}
					else
					{
						hiex::flushmessage_win32(EM_MOUSE, setting_window);

						break;
					}
				}
				hiex::flushmessage_win32(EM_MOUSE, setting_window);
			}
		}

		else if (IsInRect(m.x, m.y, { 10, 15, 190, 75 }))
		{
			if (m.lbutton)
			{
				int lx = m.x, ly = m.y;
				while (1)
				{
					ExMessage m = hiex::getmessage_win32(EM_MOUSE, setting_window);
					if (IsInRect(m.x, m.y, { 10, 15, 190, 75 }))
					{
						if (!m.lbutton)
						{
							SettingMainMode = 1;

							break;
						}
					}
					else
					{
						hiex::flushmessage_win32(EM_MOUSE, setting_window);

						break;
					}
				}
				hiex::flushmessage_win32(EM_MOUSE, setting_window);
			}
		}
		else if (IsInRect(m.x, m.y, { 10, 85, 190, 145 }))
		{
			if (m.lbutton)
			{
				int lx = m.x, ly = m.y;
				while (1)
				{
					ExMessage m = hiex::getmessage_win32(EM_MOUSE, setting_window);
					if (IsInRect(m.x, m.y, { 10, 85, 190, 145 }))
					{
						if (!m.lbutton)
						{
							SettingMainMode = 2;

							break;
						}
					}
					else
					{
						hiex::flushmessage_win32(EM_MOUSE, setting_window);

						break;
					}
				}
				hiex::flushmessage_win32(EM_MOUSE, setting_window);
			}
		}
		else if (IsInRect(m.x, m.y, { 10, 155, 190, 215 }))
		{
			if (m.lbutton)
			{
				int lx = m.x, ly = m.y;
				while (1)
				{
					ExMessage m = hiex::getmessage_win32(EM_MOUSE, setting_window);
					if (IsInRect(m.x, m.y, { 10, 155, 190, 215 }))
					{
						if (!m.lbutton)
						{
							SettingMainMode = 3;

							break;
						}
					}
					else
					{
						hiex::flushmessage_win32(EM_MOUSE, setting_window);

						break;
					}
				}
				hiex::flushmessage_win32(EM_MOUSE, setting_window);
			}
		}
	}
}
int SettingMain()
{
	this_thread::sleep_for(chrono::seconds(3));
	while (!already) this_thread::sleep_for(chrono::milliseconds(50));

	thread_status[L"SettingMain"] = true;

	loadimage(&SettingSign[1], L"PNG", L"sign2");
	loadimage(&SettingSign[2], L"PNG", L"sign3");
	loadimage(&SettingSign[3], L"PNG", L"sign4");
	loadimage(&SettingSign[4], L"PNG", L"sign5");

	IMAGE test_icon[5];
	loadimage(&test_icon[1], L"PNG", L"test_icon1");
	loadimage(&test_icon[2], L"PNG", L"test_icon2");
	loadimage(&test_icon[3], L"PNG", L"test_icon3");

	DisableResizing(setting_window, true);//��ֹ��������
	DisableSystemMenu(setting_window, true);

	LONG style = GetWindowLong(setting_window, GWL_STYLE);
	style &= ~WS_SYSMENU;
	SetWindowLong(setting_window, GWL_STYLE, style);

	hiex::SetWndProcFunc(setting_window, TestWndProc);

	// ����BLENDFUNCTION�ṹ��
	BLENDFUNCTION blend;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = 255; // ����͸���ȣ�0Ϊȫ͸����255Ϊ��͸��
	blend.AlphaFormat = AC_SRC_ALPHA; // ʹ��Դͼ���alphaͨ��
	HDC hdcScreen = GetDC(NULL);
	// ����UpdateLayeredWindow�������´���
	POINT ptSrc = { 0,0 };
	SIZE sizeWnd = { 1010, 750 };
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
	LONG nRet = ::GetWindowLong(setting_window, GWL_EXSTYLE);
	nRet |= WS_EX_LAYERED;
	::SetWindowLong(setting_window, GWL_EXSTYLE, nRet);

	thread ControlTestMain_thread(ControlTestMain);
	ControlTestMain_thread.detach();

	POINT pt;
	IMAGE test_title(1010, 750), test_background(1010, 710), test_content(800, 700);

	if (gif_load && _waccess((string_to_wstring(global_path) + L"Toothless.gif").c_str(), 4) == 0)
	{
		GIF_dynamic_effect.load((string_to_wstring(global_path) + L"Toothless.gif").c_str());
		GIF_dynamic_effect.bind(GetImageHDC(&test_title));
		GIF_dynamic_effect.setPos(250, 120);
		GIF_dynamic_effect.setSize(0, 0);
	}

	magnificationWindowReady++;
	while (!off_signal)
	{
		Sleep(500);

		if (test.select == true)
		{
			wstring ppt_LinkTest = LinkTest();
			wstring ppt_IsPptDependencyLoaded = IsPptDependencyLoaded();
			Graphics graphics(GetImageHDC(&test_content));

			if (!IsWindowVisible(setting_window)) ShowWindow(setting_window, SW_SHOW);

			while (!off_signal)
			{
				if (!choose.select) SetWindowPos(setting_window, drawpad_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				else if (ppt_show != NULL) SetWindowPos(setting_window, ppt_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				else SetWindowPos(setting_window, floating_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

				SetImageColor(test_background, RGBA(0, 0, 0, 0), true);
				hiex::EasyX_Gdiplus_SolidRoundRect(0, 0, 1000, 700, 20, 20, RGBA(245, 245, 245, 255), false, SmoothingModeHighQuality, &test_background);

				if (gif_load && !GIF_dynamic_effect.IsPlaying() && SettingMainMode == 1) GIF_dynamic_effect.play();

				if (SettingMainMode == 1)
				{
					SetImageColor(test_content, RGBA(0, 0, 0, 0), true);
					hiex::EasyX_Gdiplus_SolidRoundRect(0, 0, 800, 700, 20, 20, RGBA(255, 255, 255, 255), false, SmoothingModeHighQuality, &test_content);

					{
						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 20, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(0, 0, 0, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 0;
							dwords_rect.top = 0;
							dwords_rect.right = 800;
							dwords_rect.bottom = 30;
						}
						graphics.DrawString(L"�رմ�ҳ�����ٴε�� ѡ�� ��ť", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}

					hiex::TransparentImage(&test_content, 50, 140, &SettingSign[2]);

					if (!server_feedback.empty() && server_feedback != "")
					{
						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 20, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(0, 0, 0, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 0;
							dwords_rect.top = 480;
							dwords_rect.right = 800;
							dwords_rect.bottom = 560;
						}
						graphics.DrawString(string_to_wstring(server_feedback).c_str(), -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}

					if (AutomaticUpdateStep == 0)
					{
						hiex::EasyX_Gdiplus_RoundRect(20, 560, 760, 50, 20, 20, RGBA(130, 130, 130, 255), 2, false, SmoothingModeHighQuality, &test_content);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 25, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(130, 130, 130, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 20;
							dwords_rect.top = 560;
							dwords_rect.right = 20 + 760;
							dwords_rect.bottom = 560 + 50;
						}
						graphics.DrawString(L"�����Զ����´�����", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}
					else if (AutomaticUpdateStep == 1)
					{
						hiex::EasyX_Gdiplus_RoundRect(20, 560, 760, 50, 20, 20, RGBA(106, 156, 45, 255), 2, false, SmoothingModeHighQuality, &test_content);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 25, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(106, 156, 45, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 20;
							dwords_rect.top = 560;
							dwords_rect.right = 20 + 760;
							dwords_rect.bottom = 560 + 50;
						}
						graphics.DrawString(L"����汾�Ѿ�������", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}
					else if (AutomaticUpdateStep == 2)
					{
						hiex::EasyX_Gdiplus_RoundRect(20, 560, 760, 50, 20, 20, RGBA(245, 166, 35, 255), 2, false, SmoothingModeHighQuality, &test_content);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 25, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(245, 166, 35, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 20;
							dwords_rect.top = 560;
							dwords_rect.right = 20 + 760;
							dwords_rect.bottom = 560 + 50;
						}
						graphics.DrawString(L"�°汾�Ŷ�������", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}
					else if (AutomaticUpdateStep == 3)
					{
						hiex::EasyX_Gdiplus_RoundRect(20, 560, 760, 50, 20, 20, RGBA(106, 156, 45, 255), 2, false, SmoothingModeHighQuality, &test_content);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 25, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(106, 156, 45, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 20;
							dwords_rect.top = 560;
							dwords_rect.right = 20 + 760;
							dwords_rect.bottom = 560 + 50;
						}
						graphics.DrawString(L"��������Ը��µ����°汾", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}

					if (setlist.experimental_functions)
					{
						hiex::EasyX_Gdiplus_RoundRect(20, 620, 760, 50, 20, 20, RGBA(0, 111, 225, 255), 2, false, SmoothingModeHighQuality, &test_content);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 25, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(0, 111, 225, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 20;
							dwords_rect.top = 620;
							dwords_rect.right = 20 + 760;
							dwords_rect.bottom = 620 + 50;
						}
						graphics.DrawString(L"����ʵ���Թ��� �����ã�������ã�", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}
					else
					{
						hiex::EasyX_Gdiplus_RoundRect(20, 620, 760, 50, 20, 20, RGBA(130, 130, 130, 255), 2, false, SmoothingModeHighQuality, &test_content);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 25, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(130, 130, 130, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 20;
							dwords_rect.top = 620;
							dwords_rect.right = 20 + 760;
							dwords_rect.bottom = 620 + 50;
						}
						graphics.DrawString(L"����ʵ���Թ��� �ѽ��ã�������ã�", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}
				}
				if (SettingMainMode == 2)
				{
					SetImageColor(test_content, RGBA(0, 0, 0, 0), true);
					hiex::EasyX_Gdiplus_SolidRoundRect(0, 0, 800, 700, 20, 20, RGBA(255, 255, 255, 255), false, SmoothingModeHighQuality, &test_content);

					{
						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 20, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(0, 0, 0, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 0;
							dwords_rect.top = 0;
							dwords_rect.right = 800;
							dwords_rect.bottom = 30;
						}
						graphics.DrawString(L"�رմ�ҳ�����ٴε�� ѡ�� ��ť", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}

					Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 18, FontStyleRegular, UnitPixel);
					SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGB(0, 0, 0), false));
					graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
					{
						dwords_rect.left = 0;
						dwords_rect.top = 0;
						dwords_rect.right = 800;
						dwords_rect.bottom = 700;
					}

					GetCursorPos(&pt);

					wstring text = L"\n\n���������£�";
					text += KEY_DOWN(VK_LBUTTON) ? L"��" : L"��";
					text += L"\n������� " + to_wstring(pt.x) + L"," + to_wstring(pt.y);

					text += L"\n\n�˳���ʹ�� RealTimeStylus ���ؿ⣨���Դ�������뻭��ģʽ��";
					text += L"\n����� WinXP �����ϰ汾��ϵͳ��Ч�ʽϸߣ�֧�ֶ�㴥��";

					if (uRealTimeStylus == 2) text += L"\n\nRealTimeStylus ��Ϣ������";
					else if (uRealTimeStylus == 3) text += L"\n\nRealTimeStylus ��Ϣ��̧��";
					else if (uRealTimeStylus == 4) text += L"\n\nRealTimeStylus ��Ϣ���ƶ�";
					else text += L"\n\nRealTimeStylus ��Ϣ������";

					text += L"\n���ذ��£�";
					text += touchDown ? L"��" : L"��";
					text += L"\n���´��ص�������";
					text += to_wstring(touchNum);

					for (int i = 0; i < touchNum; i++)
					{
						std::shared_lock<std::shared_mutex> lock1(PointPosSm);
						POINT pt = TouchPos[TouchList[i]].pt;
						lock1.unlock();

						std::shared_lock<std::shared_mutex> lock2(TouchSpeedSm);
						double speed = TouchSpeed[TouchList[i]];
						lock2.unlock();

						text += L"\n���ص�" + to_wstring(i + 1) + L" pid" + to_wstring(TouchList[i]) + L" ����" + to_wstring(pt.x) + L"," + to_wstring(pt.y) + L" �ٶ�" + to_wstring(speed);
					}

					text += L"\n\nTouchList ";
					for (const auto& val : TouchList)
					{
						text += to_wstring(val) + L" ";
					}
					text += L"\nTouchTemp ";
					for (size_t i = 0; i < TouchTemp.size(); ++i)
					{
						text += to_wstring(TouchTemp[i].pid) + L" ";
					}

					text += L"\n\n���ؿ⵱ǰ��С��" + to_wstring(RecallImage.size());
					text += L"\n���ؿ� recall_image_recond��" + to_wstring(recall_image_recond);
					text += L"\n���ؿ� total_record_pointer��" + to_wstring(total_record_pointer);
					text += L"\n���ؿ� practical_total_record_pointer��" + to_wstring(practical_total_record_pointer);
					//text += L"\n\n���ؿ� reference_record_pointer��" + to_wstring(reference_record_pointer);
					//text += L"\n���ؿ� current_record_pointer��" + to_wstring(current_record_pointer);
					text += L"\n�״λ���״̬��", text += (FirstDraw == true) ? L"��" : L"��";

					text += L"\n\nCOM�����ƽӿ� ������� ״̬��\n";
					text += ppt_LinkTest;
					text += L"\nPPT �������״̬��";
					text += ppt_IsPptDependencyLoaded;

					text += L"\n\nPPT ״̬��";
					text += ppt_info_stay.TotalPage != -1 ? L"���ڲ���" : L"δ����";
					text += L"\nPPT ��ҳ������";
					text += to_wstring(ppt_info_stay.TotalPage);
					text += L"\nPPT ��ǰҳ��ţ�";
					text += to_wstring(ppt_info_stay.CurrentPage);

					graphics.DrawString(text.c_str(), -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
				}
				if (SettingMainMode == 3)
				{
					SetImageColor(test_content, RGBA(0, 0, 0, 0), true);
					hiex::EasyX_Gdiplus_SolidRoundRect(0, 0, 800, 700, 20, 20, RGBA(255, 255, 255, 255), false, SmoothingModeHighQuality, &test_content);

					hiex::TransparentImage(&test_content, 100, 20, &SettingSign[1]);
					{
						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 20, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(0, 0, 0, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 0;
							dwords_rect.top = 0;
							dwords_rect.right = 800;
							dwords_rect.bottom = 30;
						}
						graphics.DrawString(L"�رմ�ҳ�����ٴε�� ѡ�� ��ť", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}

					Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 18, FontStyleRegular, UnitPixel);
					SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGB(0, 0, 0), false));
					graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
					{
						dwords_rect.left = 0;
						dwords_rect.top = 305;
						dwords_rect.right = 800;
						dwords_rect.bottom = 630;
					}

					wstring text = L"����汾��" + string_to_wstring(edition_code);
					text += L"\n���򷢲��汾��" + string_to_wstring(edition_date) + L" Ĭ�Ϸ�֧";
					text += L"\n���򹹽�ʱ�䣺" + buildTime;
					text += L"\n�û�ID��" + userid;

					if (gif_load)
					{
						text += L"\n\n����ͨ����EasterEgg ���������";
						text += L"\n�����������Ҫɾ��ͬĿ¼�µĲʵ� GIF �ļ���";
					}
					else
					{
						text += L"\n\n����ͨ����LTS";
						if (!server_code.empty() && server_code != "")
						{
							text += L"\n�����汾���ţ�" + string_to_wstring(server_code);
							if (server_code == "GWSR") text += L" ������ר�ð汾��";
						}
						if (procedure_updata_error == 1) text += L"\n�����Զ����£�������";
						else if (procedure_updata_error == 2) text += L"\n�����Զ����£������������";
						else text += L"\n�����Զ����£������У��ȴ�������������";
					}

					if (!server_feedback.empty() && server_feedback != "") text += L"\n������������Ϣ��" + string_to_wstring(server_feedback);
					if (server_updata_error)
					{
						text += L"\n\n������ͨ�Ŵ���Error" + to_wstring(server_updata_error);
						if (!server_updata_error_reason.empty()) text += L"\n" + server_updata_error_reason;
					}

					graphics.DrawString(text.c_str(), -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);

					{
						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 18, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGB(6, 39, 182), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 0;
							dwords_rect.top = 630;
							dwords_rect.right = 800;
							dwords_rect.bottom = 700;
						}
						graphics.DrawString(L"���������ϵ��ʽ\nQQ: 2685549821\ne-mail: alan-crl@foxmail.com", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}
				}

				//����
				{
					Graphics graphics(GetImageHDC(&test_background));

					if (SettingMainMode == 1)
					{
						hiex::EasyX_Gdiplus_FillRoundRect(10, 10, 180, 60, 20, 20, RGBA(0, 111, 225, 255), RGBA(230, 230, 230, 255), 3, false, SmoothingModeHighQuality, &test_background);

						ChangeColor(test_icon[1], RGBA(0, 111, 225, 255));
						hiex::TransparentImage(&test_background, 20, 20, &test_icon[1]);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 24, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(0, 111, 225, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 70;
							dwords_rect.top = 10;
							dwords_rect.right = 70 + 110;
							dwords_rect.bottom = 10 + 63;
						}
						graphics.DrawString(L"��ҳ", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}
					else
					{
						hiex::EasyX_Gdiplus_FillRoundRect(10, 10, 180, 60, 20, 20, RGBA(230, 230, 230, 255), RGBA(230, 230, 230, 255), 3, false, SmoothingModeHighQuality, &test_background);

						ChangeColor(test_icon[1], RGBA(130, 130, 130, 255));
						hiex::TransparentImage(&test_background, 20, 20, &test_icon[1]);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 24, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(130, 130, 130, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 70;
							dwords_rect.top = 10;
							dwords_rect.right = 70 + 110;
							dwords_rect.bottom = 10 + 63;
						}
						graphics.DrawString(L"��ҳ", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}

					if (SettingMainMode == 2)
					{
						hiex::EasyX_Gdiplus_FillRoundRect(10, 80, 180, 60, 20, 20, RGBA(0, 111, 225, 255), RGBA(230, 230, 230, 255), 3, false, SmoothingModeHighQuality, &test_background);

						ChangeColor(test_icon[2], RGBA(0, 111, 225, 255));
						hiex::TransparentImage(&test_background, 20, 90, &test_icon[2]);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 24, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(0, 111, 225, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 70;
							dwords_rect.top = 80;
							dwords_rect.right = 70 + 110;
							dwords_rect.bottom = 80 + 63;
						}
						graphics.DrawString(L"����", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}
					else
					{
						hiex::EasyX_Gdiplus_FillRoundRect(10, 80, 180, 60, 20, 20, RGBA(230, 230, 230, 255), RGBA(230, 230, 230, 255), 3, false, SmoothingModeHighQuality, &test_background);

						ChangeColor(test_icon[2], RGBA(130, 130, 130, 255));
						hiex::TransparentImage(&test_background, 20, 90, &test_icon[2]);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 24, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(130, 130, 130, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 70;
							dwords_rect.top = 80;
							dwords_rect.right = 70 + 110;
							dwords_rect.bottom = 80 + 63;
						}
						graphics.DrawString(L"����", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}

					if (SettingMainMode == 3)
					{
						hiex::EasyX_Gdiplus_FillRoundRect(10, 150, 180, 60, 20, 20, RGBA(0, 111, 225, 255), RGBA(230, 230, 230, 255), 3, false, SmoothingModeHighQuality, &test_background);

						ChangeColor(test_icon[3], RGBA(0, 111, 225, 255));
						hiex::TransparentImage(&test_background, 20, 160, &test_icon[3]);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 24, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(0, 111, 225, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 70;
							dwords_rect.top = 150;
							dwords_rect.right = 70 + 110;
							dwords_rect.bottom = 150 + 63;
						}
						graphics.DrawString(L"����", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}
					else
					{
						hiex::EasyX_Gdiplus_FillRoundRect(10, 150, 180, 60, 20, 20, RGBA(230, 230, 230, 255), RGBA(230, 230, 230, 255), 3, false, SmoothingModeHighQuality, &test_background);

						ChangeColor(test_icon[3], RGBA(130, 130, 130, 255));
						hiex::TransparentImage(&test_background, 20, 160, &test_icon[3]);

						Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 24, FontStyleRegular, UnitPixel);
						SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(130, 130, 130, 255), false));
						graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
						{
							dwords_rect.left = 70;
							dwords_rect.top = 150;
							dwords_rect.right = 70 + 110;
							dwords_rect.bottom = 150 + 63;
						}
						graphics.DrawString(L"����", -1, &gp_font, hiex::RECTToRectF(dwords_rect), &stringFormat, &WordBrush);
					}

					hiex::TransparentImage(&test_background, 10, 486, &SettingSign[4]);
				}
				//������
				{
					SetImageColor(test_title, RGBA(0, 0, 0, 0), true);
					hiex::EasyX_Gdiplus_SolidRoundRect(5, 5, 1000, 740, 20, 20, RGBA(0, 111, 225, 200), true, SmoothingModeHighQuality, &test_title);

					Graphics graphics(GetImageHDC(&test_title));
					Gdiplus::Font20 gp_font(&HarmonyOS_fontFamily, 22, FontStyleRegular, UnitPixel);
					SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGBA(255, 255, 255, 255), false));
					graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
					{
						words_rect.left = 5;
						words_rect.top = 5;
						words_rect.right = 1005;
						words_rect.bottom = 50;
					}
					graphics.DrawString(L"�ǻ�� ����ѡ��", -1, &gp_font, hiex::RECTToRectF(words_rect), &stringFormat, &WordBrush);
				}
				hiex::TransparentImage(&test_background, 200, 0, &test_content);
				hiex::TransparentImage(&test_title, 5, 45, &test_background);
				hiex::EasyX_Gdiplus_RoundRect(5, 5, 1000, 740, 20, 20, RGBA(0, 111, 225, 255), 3, false, SmoothingModeHighQuality, &test_title);

				if (gif_load && SettingMainMode == 1) GIF_dynamic_effect.draw();

				{
					RECT rect;
					GetWindowRect(setting_window, &rect);

					POINT ptDst = { rect.left, rect.top };
					ulwi.pptDst = &ptDst;
					ulwi.hdcSrc = GetImageHDC(&test_title);
					UpdateLayeredWindowIndirect(setting_window, &ulwi);
				}

				if (off_signal || !test.select) break;

				Sleep(20);
			}

			if (gif_load && GIF_dynamic_effect.IsPlaying())
			{
				GIF_dynamic_effect.pause();
				GIF_dynamic_effect.resetPlayState();
			}
		}
		else if (IsWindowVisible(setting_window)) ShowWindow(setting_window, SW_HIDE);
	}

	ShowWindow(setting_window, SW_HIDE);
	thread_status[L"SettingMain"] = false;

	return 0;
}
*/

void SettingSeekBar()
{
	if (!KEY_DOWN(VK_LBUTTON)) return;

	POINT p;
	GetCursorPos(&p);

	int pop_x = p.x - SettingWindowX;
	int pop_y = p.y - SettingWindowY;

	while (1)
	{
		if (!KEY_DOWN(VK_LBUTTON)) break;

		POINT p;
		GetCursorPos(&p);

		SetWindowPos(setting_window,
			NULL,
			SettingWindowX = p.x - pop_x,
			SettingWindowY = p.y - pop_y,
			0,
			0,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	}

	return;
}
int SettingMain()
{
	while (!already) this_thread::sleep_for(chrono::milliseconds(50));
	thread_status[L"SettingMain"] = true;

	ImGuiWc = { sizeof(WNDCLASSEX), CS_CLASSDC, ImGuiWndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Idt ImGui Tool"), NULL };
	::RegisterClassEx(&ImGuiWc);
	setting_window = ::CreateWindow(ImGuiWc.lpszClassName, _T("Idt ImGui Tool"), WS_OVERLAPPEDWINDOW, SettingWindowX, SettingWindowY, SettingWindowWidth, SettingWindowHeight, NULL, NULL, ImGuiWc.hInstance, NULL);
	SetWindowLong(setting_window, GWL_STYLE, GetWindowLong(setting_window, GWL_STYLE) & ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME));
	SetWindowPos(setting_window, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);

	::ShowWindow(setting_window, SW_HIDE);
	::UpdateWindow(setting_window);
	CreateDeviceD3D(setting_window);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // ���ü��̿���

	ImGui::StyleColorsLight();

	ImGui_ImplWin32_Init(setting_window);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	io.IniFilename = nullptr;

	HMODULE hModule = GetModuleHandle(NULL);
	HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(198), L"TTF");
	HGLOBAL hMemory = LoadResource(hModule, hResource);
	PVOID pResourceData = LockResource(hMemory);
	DWORD dwResourceSize = SizeofResource(hModule, hResource);
	ImFont* Font = io.Fonts->AddFontFromMemoryTTF(pResourceData, dwResourceSize, 26.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	ImGuiStyle& style = ImGui::GetStyle();
	auto Color = style.Colors;

	style.ChildRounding = 8.0f;
	style.FrameRounding = 8.0f;
	style.GrabRounding = 8.0f;

	style.Colors[ImGuiCol_WindowBg] = ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f);

	loadimage(&SettingSign[1], L"PNG", L"sign2");
	{
		int width = SettingSign[1].getwidth();
		int height = SettingSign[1].getheight();
		DWORD* pMem = GetImageBuffer(&SettingSign[1]);

		unsigned char* data = new unsigned char[width * height * 4];
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				DWORD color = pMem[y * width + x];
				unsigned char alpha = (color & 0xFF000000) >> 24;
				if (alpha != 0)
				{
					data[(y * width + x) * 4 + 0] = ((color & 0x00FF0000) >> 16) * 255 / alpha;
					data[(y * width + x) * 4 + 1] = ((color & 0x0000FF00) >> 8) * 255 / alpha;
					data[(y * width + x) * 4 + 2] = ((color & 0x000000FF) >> 0) * 255 / alpha;
				}
				else
				{
					data[(y * width + x) * 4 + 0] = 0;
					data[(y * width + x) * 4 + 1] = 0;
					data[(y * width + x) * 4 + 2] = 0;
				}
				data[(y * width + x) * 4 + 3] = alpha;
			}
		}

		bool ret = LoadTextureFromFile(data, width, height, &TextureSettingSign[1]);
		delete[] data;

		IM_ASSERT(ret);
	}
	loadimage(&SettingSign[2], L"PNG", L"sign3");
	{
		int width = SettingSign[2].getwidth();
		int height = SettingSign[2].getheight();
		DWORD* pMem = GetImageBuffer(&SettingSign[2]);

		unsigned char* data = new unsigned char[width * height * 4];
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				DWORD color = pMem[y * width + x];
				unsigned char alpha = (color & 0xFF000000) >> 24;
				if (alpha != 0)
				{
					data[(y * width + x) * 4 + 0] = ((color & 0x00FF0000) >> 16) * 255 / alpha;
					data[(y * width + x) * 4 + 1] = ((color & 0x0000FF00) >> 8) * 255 / alpha;
					data[(y * width + x) * 4 + 2] = ((color & 0x000000FF) >> 0) * 255 / alpha;
				}
				else
				{
					data[(y * width + x) * 4 + 0] = 0;
					data[(y * width + x) * 4 + 1] = 0;
					data[(y * width + x) * 4 + 2] = 0;
				}
				data[(y * width + x) * 4 + 3] = alpha;
			}
		}

		bool ret = LoadTextureFromFile(data, width, height, &TextureSettingSign[2]);
		delete[] data;

		IM_ASSERT(ret);
	}
	loadimage(&SettingSign[3], L"PNG", L"sign4");
	loadimage(&SettingSign[4], L"PNG", L"sign5");
	{
		int width = SettingSign[4].getwidth();
		int height = SettingSign[4].getheight();
		DWORD* pMem = GetImageBuffer(&SettingSign[4]);

		unsigned char* data = new unsigned char[width * height * 4];
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				DWORD color = pMem[y * width + x];
				unsigned char alpha = (color & 0xFF000000) >> 24;
				if (alpha != 0)
				{
					data[(y * width + x) * 4 + 0] = ((color & 0x00FF0000) >> 16) * 255 / alpha;
					data[(y * width + x) * 4 + 1] = ((color & 0x0000FF00) >> 8) * 255 / alpha;
					data[(y * width + x) * 4 + 2] = ((color & 0x000000FF) >> 0) * 255 / alpha;
				}
				else
				{
					data[(y * width + x) * 4 + 0] = 0;
					data[(y * width + x) * 4 + 1] = 0;
					data[(y * width + x) * 4 + 2] = 0;
				}
				data[(y * width + x) * 4 + 3] = alpha;
			}
		}

		bool ret = LoadTextureFromFile(data, width, height, &TextureSettingSign[4]);
		delete[] data;

		IM_ASSERT(ret);
	}

	//��ʼ���������
	hiex::tDelayFPS recond;
	POINT MoushPos = { 0,0 };
	bool ShowWindow = false;
	ImGuiToggleConfig config;
	config.FrameRounding = 0.3f;
	config.KnobRounding = 0.5f;
	config.Size = { 60.0f,30.0f };
	config.Flags = ImGuiToggleFlags_Animated;

	int PushStyleColorNum = 0, PushFontNum = 0;
	int QueryWaitingTime = 5;

	bool StartUp = false, CreateLnk = true;
	bool BrushRecover = setlist.BrushRecover, RubberRecover = setlist.RubberRecover;

	wstring ppt_LinkTest = LinkTest();
	wstring ppt_IsPptDependencyLoaded = L"���Ӧ��û����(״̬��ʾ�������⣬�������޸�)";// = IsPptDependencyLoaded(); //TODO ��������
	wstring receivedData;
	POINT pt;

	magnificationWindowReady++;
	while (!off_signal)
	{
		MSG msg;
		while (::PeekMessage(&msg, setting_window, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);

			switch (msg.message)
			{
			case WM_MOUSEMOVE:
				MoushPos.x = GET_X_LPARAM(msg.lParam);
				MoushPos.y = GET_Y_LPARAM(msg.lParam);

				break;

			case WM_LBUTTONDOWN:
				if (IsInRect(MoushPos.x, MoushPos.y, { 0,0,870,30 })) SettingSeekBar();

				break;
			}
		}
		hiex::DelayFPS(recond, 10);

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		{
			//����������
			static int Tab = 0;
			enum Tab
			{
				tab1,
				tab2,
				tab3,
				tab4,
				tab5
			};

			ImGui::SetNextWindowPos({ 0,0 });//���ô���λ��
			ImGui::SetNextWindowSize({ static_cast<float>(SettingWindowWidth),static_cast<float>(SettingWindowHeight) });//���ô��ڴ�С

			Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
			ImGui::Begin(u8"�ǻ��ѡ��", &test.select, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);//��ʼ���ƴ���

			{
				ImGui::SetCursorPos({ 10.0f,44.0f });

				PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab1 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab1 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
				if (Tab == Tab::tab1) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
				if (Tab == Tab::tab1) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
				if (ImGui::Button(u8"��ҳ", { 100.0f,40.0f }))
				{
					Tab = Tab::tab1;
				}
				while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

				ImGui::SetCursorPos({ 10.0f,94.0f });

				PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab2 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab2 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
				if (Tab == Tab::tab2) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
				if (Tab == Tab::tab2) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
				if (ImGui::Button(u8"����", { 100.0f,40.0f }))
				{
					Tab = Tab::tab2;
				}
				while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

				ImGui::SetCursorPos({ 10.0f,144.0f });

				PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab3 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab3 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
				if (Tab == Tab::tab3) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
				if (Tab == Tab::tab3) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
				if (ImGui::Button(u8"����", { 100.0f,40.0f }))
				{
					Tab = Tab::tab3;
				}
				while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

				ImGui::SetCursorPos({ 10.0f,194.0f });

				PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab4 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab4 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
				if (Tab == Tab::tab4) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
				if (Tab == Tab::tab4) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
				if (ImGui::Button(u8"����", { 100.0f,40.0f }))
				{
					Tab = Tab::tab4;
				}
				while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

				ImGui::SetCursorPos({ 10.0f,244.0f });

				PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab5 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab5 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
				if (Tab == Tab::tab5) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
				if (Tab == Tab::tab5) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
				if (ImGui::Button(u8"��������", { 100.0f,40.0f }))
				{
					Tab = Tab::tab5;
				}
				while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();
			}

			ImGui::SetCursorPos({ 120.0f,44.0f });
			ImGui::BeginChild(u8"������", { 770.0f,616.0f }, true);
			switch (Tab)
			{
			case Tab::tab1:
				// ��ҳ
				Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);

				ImGui::SetCursorPos({ 35.0f,ImGui::GetCursorPosY() + 40.0f });
				ImGui::Image((void*)TextureSettingSign[2], ImVec2((float)SettingSign[2].getwidth(), (float)SettingSign[2].getheight()));

				/*
				if (AutomaticUpdateStep == 0)
				{
					ImGui::SetCursorPos({ 185.0f - (float)SettingSign[4].getwidth() / 2.0f,ImGui::GetCursorPosY() + 20 });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(195 / 255.0f, 195 / 255.0f, 195 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
					if (ImGui::Button(u8"�����Զ����´�����", { 400.0f,50.0f }))
					{
						//TODO
					}
				}
				else if (AutomaticUpdateStep == 1)
				{
					ImGui::SetCursorPos({ 185.0f - (float)SettingSign[4].getwidth() / 2.0f,ImGui::GetCursorPosY() + 20 });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(106 / 255.0f, 156 / 255.0f, 45 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(93 / 255.0f, 136 / 255.0f, 39 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(79 / 255.0f, 116 / 255.0f, 34 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
					if (ImGui::Button(u8"����汾�Ѿ�������", { 400.0f,50.0f }))
					{
						//TODO
					}
				}
				else if (AutomaticUpdateStep == 2)
				{
					ImGui::SetCursorPos({ 185.0f - (float)SettingSign[4].getwidth() / 2.0f,ImGui::GetCursorPosY() + 20 });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(245 / 255.0f, 166 / 255.0f, 35 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(225 / 255.0f, 153 / 255.0f, 34 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(205 / 255.0f, 140 / 255.0f, 33 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
					if (ImGui::Button(u8"�����°汾�Ŷ�������", { 400.0f,50.0f }))
					{
						//TODO
					}
				}
				else if (AutomaticUpdateStep == 3)
				{
					ImGui::SetCursorPos({ 185.0f - (float)SettingSign[4].getwidth() / 2.0f,ImGui::GetCursorPosY() + 20 });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 153 / 255.0f, 255 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 141 / 255.0f, 235 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 129 / 255.0f, 215 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
					if (ImGui::Button(u8"���������Ը��µ����°汾", { 400.0f,50.0f }))
					{
						//TODO
					}
				}
				while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

				{
					float CursorPosY = ImGui::GetCursorPosY();

					ImGui::SetCursorPos({ 185.0f - (float)SettingSign[4].getwidth() / 2.0f,CursorPosY + 10 });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 153 / 255.0f, 255 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 141 / 255.0f, 235 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 129 / 255.0f, 215 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
					if (ImGui::Button(u8"ʵ�����¹���", { 400.0f,50.0f }))
					{
						//TODO
					}
					while (PushStyleColorNum) PushStyleColorNum--,ImGui::PopStyleColor();
				}*/

				{
					ImGui::SetCursorPosY(616.0f - 168.0f);
					std::wstring author = L"���������ϵ��ʽ\nQQ: 2685549821\nEmail: alan-crl@foxmail.com";
					int left_x = 10, right_x = 570;

					std::vector<std::string> lines;
					std::wstring line, temp;
					std::wstringstream ss(author);

					while (getline(ss, temp, L'\n'))
					{
						bool flag = false;
						line = L"";

						for (wchar_t ch : temp)
						{
							flag = false;

							float text_width = ImGui::CalcTextSize(convert_to_utf8(wstring_to_string(line + ch)).c_str()).x;
							if (text_width > (right_x - left_x))
							{
								lines.emplace_back(convert_to_utf8(wstring_to_string(line)));
								line = L"", flag = true;
							}

							line += ch;
						}

						if (!flag) lines.emplace_back(convert_to_utf8(wstring_to_string(line)));
					}

					for (const auto& temp : lines)
					{
						float text_width = ImGui::CalcTextSize(temp.c_str()).x;
						float text_indentation = ((right_x - left_x) - text_width) * 0.5f;
						if (text_indentation < 0)  text_indentation = 0;
						ImGui::SetCursorPosX(left_x + text_indentation);
						ImGui::TextUnformatted(temp.c_str());
					}
				}
				{
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));

					std::wstring author = L"�ٷ�QȺ��618720802\n��л��λһֱ�������ǻ�̵�֧��~";
					int left_x = 10, right_x = 570;

					std::vector<std::string> lines;
					std::wstring line, temp;
					std::wstringstream ss(author);

					while (getline(ss, temp, L'\n'))
					{
						bool flag = false;
						line = L"";

						for (wchar_t ch : temp)
						{
							flag = false;

							float text_width = ImGui::CalcTextSize(convert_to_utf8(wstring_to_string(line + ch)).c_str()).x;
							if (text_width > (right_x - left_x))
							{
								lines.emplace_back(convert_to_utf8(wstring_to_string(line)));
								line = L"", flag = true;
							}

							line += ch;
						}

						if (!flag) lines.emplace_back(convert_to_utf8(wstring_to_string(line)));
					}

					for (const auto& temp : lines)
					{
						float text_width = ImGui::CalcTextSize(temp.c_str()).x;
						float text_indentation = ((right_x - left_x) - text_width) * 0.5f;
						if (text_indentation < 0)  text_indentation = 0;
						ImGui::SetCursorPosX(left_x + text_indentation);
						ImGui::TextUnformatted(temp.c_str());
					}
				}

				ImGui::SetCursorPos({ 760.0f - (float)SettingSign[4].getwidth(),606.0f - (float)SettingSign[4].getheight() });
				ImGui::Image((void*)TextureSettingSign[4], ImVec2((float)SettingSign[4].getwidth(), (float)SettingSign[4].getheight()));

				break;

			case Tab::tab2:
				// ͨ��
				ImGui::SetCursorPosY(20.0f);

				{
					ImGui::SetCursorPosX(20.0f);
					ImGui::BeginChild(u8"���򻷾�", { 730.0f,125.0f }, true, ImGuiWindowFlags_NoScrollbar);

					{
						ImGui::SetCursorPosY(10.0f);

						Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
						CenteredText(u8" ��ѯ��������״̬", 4.0f);

						Font->Scale = 0.7f, PushFontNum++, ImGui::PushFont(Font);
						ImGui::SameLine(); HelpMarker(u8"���������Զ���������ǰ��Ҫ��ѯ��ǰ״̬�������������ԱȨ�ޣ�", ImGui::GetStyleColorVec4(ImGuiCol_Text));

						if (!receivedData.empty())
						{
							string temp, helptemp;
							if (receivedData.length() >= 5 && receivedData.substr(0, 5) == L"Succe") temp = u8"��ѯ״̬�ɹ�", helptemp = u8"���Ե���������������";
							else if (receivedData.length() >= 5 && receivedData.substr(0, 5) == L"Error") temp = u8"��ѯ״̬���� " + wstring_to_string(receivedData), helptemp = u8"�ٴε����ѯ���ԣ������������Թ���Ա�������";
							else if (receivedData.length() >= 5 && receivedData.substr(0, 5) == L"TimeO") temp = u8"��ѯ״̬��ʱ", helptemp = u8"�ٴε����ѯ����\nͬʱ��ʱʱ�佫�� 5 �����Ϊ 15 ��", QueryWaitingTime = 15;
							else if (receivedData.length() >= 5 && receivedData.substr(0, 5) == L"Renew") temp = u8"��Ҫ���²�ѯ״̬", helptemp = u8"���������Զ���������ʱ��ʱ�����ٴε����ѯ\nͬʱ��ʱʱ�佫�� 5 �����Ϊ 15 ��", QueryWaitingTime = 15;
							else temp = u8"δ֪����", helptemp = u8"�ٴε����ѯ���ԣ������������Թ���Ա�������";

							Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
							ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 80.0f - ImGui::CalcTextSize(temp.c_str()).x - 30.0f);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							CenteredText(temp.c_str(), 4.0f);

							Font->Scale = 0.7f, PushFontNum++, ImGui::PushFont(Font);
							ImGui::SameLine(); HelpMarker(helptemp.c_str(), ImGui::GetStyleColorVec4(ImGuiCol_Text));
						}

						Font->Scale = 0.7f, PushFontNum++, ImGui::PushFont(Font);
						ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 70.0f);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(195 / 255.0f, 195 / 255.0f, 195 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
						if (ImGui::Button(u8"��ѯ", { 60.0f,30.0f }))
						{
							if (_waccess((string_to_wstring(global_path) + L"api").c_str(), 0) == -1) filesystem::create_directory(string_to_wstring(global_path) + L"api");
							ExtractResource((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), L"EXE", MAKEINTRESOURCE(229));

							ShellExecute(NULL, L"runas", (string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), (L"/\"query\" /\"" + GetCurrentExePath() + L"\\�ǻ��.exe\"").c_str(), NULL, SW_SHOWNORMAL);

							DWORD dwBytesRead;
							WCHAR buffer[4096];
							HANDLE hPipe = INVALID_HANDLE_VALUE;

							int for_i;
							for (for_i = 0; for_i <= QueryWaitingTime * 10; for_i++)
							{
								if (WaitNamedPipe(TEXT("\\\\.\\pipe\\IDTPipe1"), 100)) break;
								else Sleep(100);
							}

							if (for_i > QueryWaitingTime * 10) receivedData = L"TimeOut";
							else
							{
								hPipe = CreateFile(TEXT("\\\\.\\pipe\\IDTPipe1"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
								if (ReadFile(hPipe, buffer, sizeof(buffer), &dwBytesRead, NULL))
								{
									receivedData.assign(buffer, dwBytesRead / sizeof(WCHAR));
									if (receivedData == L"fail")
									{
										receivedData = L"Success";
										setlist.StartUp = 1, StartUp = false;
									}
									else if (receivedData == L"success")
									{
										receivedData = L"Success";
										setlist.StartUp = 2, StartUp = true;
									}
									else receivedData = L"Error unknown";
								}
								else receivedData = L"Error" + to_wstring(GetLastError());
							}

							CloseHandle(hPipe);
						}
					}
					{
						ImGui::SetCursorPosY(45.0f);

						Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
						CenteredText(u8" �����Զ�����", 4.0f);

						Font->Scale = 0.7f, PushFontNum++, ImGui::PushFont(Font);
						ImGui::SameLine();
						if (setlist.StartUp) HelpMarker(u8"�����������ԱȨ��", ImGui::GetStyleColorVec4(ImGuiCol_Text));
						else HelpMarker(u8"���������Զ���������ǰ��Ҫ��ѯ��ǰ״̬�������������ԱȨ�ޣ�\n�����Ϸ���ť��ѯ��ǰ״̬", ImGui::GetStyleColorVec4(ImGuiCol_Text));

						if (setlist.StartUp)
						{
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 101 / 255.0f, 205 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
							ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 70.0f);
							ImGui::Toggle(u8"##�����Զ�����", &StartUp, config);

							if (setlist.StartUp - 1 != (int)StartUp)
							{
								if (_waccess((string_to_wstring(global_path) + L"api").c_str(), 0) == -1) filesystem::create_directory(string_to_wstring(global_path) + L"api");
								ExtractResource((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), L"EXE", MAKEINTRESOURCE(229));

								if (StartUp) ShellExecute(NULL, L"runas", (string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), (L"/\"set\" /\"" + GetCurrentExePath() + L"\"").c_str(), NULL, SW_SHOWNORMAL);
								else ShellExecute(NULL, L"runas", (string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), (L"/\"delete\" /\"" + GetCurrentExePath() + L"\"").c_str(), NULL, SW_SHOWNORMAL);

								DWORD dwBytesRead;
								WCHAR buffer[4096];
								HANDLE hPipe = INVALID_HANDLE_VALUE;
								wstring treceivedData;

								int for_i;
								for (for_i = 0; for_i <= QueryWaitingTime * 10; for_i++)
								{
									if (WaitNamedPipe(TEXT("\\\\.\\pipe\\IDTPipe1"), 100)) break;
									else Sleep(100);
								}

								if (for_i <= QueryWaitingTime * 10)
								{
									hPipe = CreateFile(TEXT("\\\\.\\pipe\\IDTPipe1"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
									if (ReadFile(hPipe, buffer, sizeof(buffer), &dwBytesRead, NULL))
									{
										treceivedData.assign(buffer, dwBytesRead / sizeof(WCHAR));
										if (treceivedData == L"fail") setlist.StartUp = 1, StartUp = false;
										else if (treceivedData == L"success") setlist.StartUp = 2, StartUp = true;
									}
								}
								else setlist.StartUp = 0, receivedData = L"Renew";

								CloseHandle(hPipe);
							}
						}
					}
					{
						ImGui::SetCursorPosY(80.0f);

						Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
						CenteredText(u8" ����ʱ���������ݷ�ʽ", 4.0f);

						Font->Scale = 0.7f, PushFontNum++, ImGui::PushFont(Font);
						ImGui::SameLine();
						HelpMarker(u8"������ÿ������ʱ�����洴����ݷ�ʽ\n��������ܽ������Ϊ�������ӵ�и�����Զ��幦��", ImGui::GetStyleColorVec4(ImGuiCol_Text));

						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 101 / 255.0f, 205 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
						ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 70.0f);
						ImGui::Toggle(u8"##����ʱ���������ݷ�ʽ", &CreateLnk, config);

						if (setlist.CreateLnk != CreateLnk)
						{
							setlist.CreateLnk = CreateLnk;
							WriteSetting();
						}
					}

					ImGui::EndChild();
				}
				{
					ImGui::SetCursorPosX(20.0f);
					ImGui::BeginChild(u8"���ʵ���", { 730.0f,90.0f }, true, ImGuiWindowFlags_NoScrollbar);

					{
						ImGui::SetCursorPosY(10.0f);

						Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
						CenteredText(u8" ���ʻ���ʱ�Զ���������", 4.0f);

						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 101 / 255.0f, 205 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
						ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 70.0f);
						ImGui::Toggle(u8"##���ʻ���ʱ�Զ���������", &BrushRecover, config);

						if (setlist.BrushRecover != BrushRecover)
						{
							setlist.BrushRecover = BrushRecover;
							WriteSetting();
						}
					}
					{
						ImGui::SetCursorPosY(45.0f);

						Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
						CenteredText(u8" ��Ƥ����ʱ�Զ���������", 4.0f);

						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 101 / 255.0f, 205 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
						ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 70.0f);
						ImGui::Toggle(u8"##��Ƥ����ʱ�Զ���������", &RubberRecover, config);

						if (setlist.RubberRecover != RubberRecover)
						{
							setlist.RubberRecover = RubberRecover;
							WriteSetting();
						}
					}

					ImGui::EndChild();
				}

				break;
			case Tab::tab3:
				//����
				Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
				{
					ImGui::SetCursorPosY(10.0f);
					wstring text;
					{
						GetCursorPos(&pt);

						text = L"���������£�";
						text += KEY_DOWN(VK_LBUTTON) ? L"��" : L"��";
						text += L"\n������� " + to_wstring(pt.x) + L"," + to_wstring(pt.y);

						text += L"\n\n�˳���ʹ�� RealTimeStylus ���ؿ⣨���Դ�������뻭��ģʽ��";
						text += L"\n����� WinXP �����ϰ汾��ϵͳ��Ч�ʽϸߣ�֧�ֶ�㴥��";

						if (uRealTimeStylus == 2) text += L"\n\nRealTimeStylus ��Ϣ������";
						else if (uRealTimeStylus == 3) text += L"\n\nRealTimeStylus ��Ϣ��̧��";
						else if (uRealTimeStylus == 4) text += L"\n\nRealTimeStylus ��Ϣ���ƶ�";
						else text += L"\n\nRealTimeStylus ��Ϣ������";

						text += L"\n���ذ��£�";
						text += touchDown ? L"��" : L"��";
						text += L"\n���´��ص�������";
						text += to_wstring(touchNum);

						for (int i = 0; i < touchNum; i++)
						{
							std::shared_lock<std::shared_mutex> lock1(PointPosSm);
							POINT pt = TouchPos[TouchList[i]].pt;
							lock1.unlock();

							std::shared_lock<std::shared_mutex> lock2(TouchSpeedSm);
							double speed = TouchSpeed[TouchList[i]];
							lock2.unlock();

							text += L"\n���ص�" + to_wstring(i + 1) + L" pid" + to_wstring(TouchList[i]) + L" ����" + to_wstring(pt.x) + L"," + to_wstring(pt.y) + L" �ٶ�" + to_wstring(speed);
						}

						text += L"\n\nTouchList ";
						for (const auto& val : TouchList)
						{
							text += to_wstring(val) + L" ";
						}
						text += L"\nTouchTemp ";
						for (size_t i = 0; i < TouchTemp.size(); ++i)
						{
							text += to_wstring(TouchTemp[i].pid) + L" ";
						}

						text += L"\n\n���ؿ⵱ǰ��С��" + to_wstring(RecallImage.size()) + L"(��ֵ" + to_wstring(RecallImagePeak) + L")";
						text += L"\n���ؿ� recall_image_recond��" + to_wstring(recall_image_recond);
						text += L"\n���ؿ� reference_record_pointer��" + to_wstring(reference_record_pointer);
						text += L"\n���ؿ� practical_total_record_pointer��" + to_wstring(practical_total_record_pointer);
						text += L"\n���ؿ� total_record_pointer��" + to_wstring(total_record_pointer);
						text += L"\n���ؿ� current_record_pointer��" + to_wstring(current_record_pointer);
						text += L"\n\n�״λ���״̬��", text += (FirstDraw == true) ? L"��" : L"��";

						text += L"\n\nCOM�����ƽӿ� ������� ״̬��\n";
						text += ppt_LinkTest;
						text += L"\nPPT �������״̬��";
						text += ppt_IsPptDependencyLoaded;

						text += L"\n\nPPT ״̬��";
						text += ppt_info_stay.TotalPage != -1 ? L"���ڲ���" : L"δ����";
						text += L"\nPPT ��ҳ������";
						text += to_wstring(ppt_info_stay.TotalPage);
						text += L"\nPPT ��ǰҳ��ţ�";
						text += to_wstring(ppt_info_stay.CurrentPage);
					}

					int left_x = 10, right_x = 760;

					std::vector<std::string> lines;
					std::wstring line, temp;
					std::wstringstream ss(text);

					while (getline(ss, temp, L'\n'))
					{
						bool flag = false;
						line = L"";

						for (wchar_t ch : temp)
						{
							flag = false;

							float text_width = ImGui::CalcTextSize(convert_to_utf8(wstring_to_string(line + ch)).c_str()).x;
							if (text_width > (right_x - left_x))
							{
								lines.emplace_back(convert_to_utf8(wstring_to_string(line)));
								line = L"", flag = true;
							}

							line += ch;
						}

						if (!flag) lines.emplace_back(convert_to_utf8(wstring_to_string(line)));
					}

					for (const auto& temp : lines)
					{
						float text_width = ImGui::CalcTextSize(temp.c_str()).x;
						float text_indentation = ((right_x - left_x) - text_width) * 0.5f;
						if (text_indentation < 0)  text_indentation = 0;
						ImGui::SetCursorPosX(left_x + text_indentation);
						ImGui::TextUnformatted(temp.c_str());
					}
				}
				break;

			case Tab::tab4:
				// ����
				ImGui::SetCursorPos({ 35.0f,ImGui::GetCursorPosY() + 40.0f });
				ImGui::Image((void*)TextureSettingSign[1], ImVec2((float)SettingSign[1].getwidth(), (float)SettingSign[1].getheight()));

				Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
					wstring text;
					{
						text = L"����汾��" + string_to_wstring(edition_code);
						text += L"\n���򷢲��汾��" + string_to_wstring(edition_date) + L" Ĭ�Ϸ�֧";
						text += L"\n���򹹽�ʱ�䣺" + buildTime;
						text += L"\n�û�ID��" + userid;

						text += L"\n\n����ͨ����LTS";
						if (!server_code.empty() && server_code != "")
						{
							text += L"\n�����汾���ţ�" + string_to_wstring(server_code);
							if (server_code == "GWSR") text += L" ������ר�ð汾��";
						}
						if (procedure_updata_error == 1) text += L"\n�����Զ����£�������";
						else if (procedure_updata_error == 2) text += L"\n�����Զ����£������������";
						else text += L"\n�����Զ����£������У��ȴ�������������";

						if (!server_feedback.empty() && server_feedback != "") text += L"\n������������Ϣ��" + string_to_wstring(server_feedback);
						if (server_updata_error)
						{
							text += L"\n\n������ͨ�Ŵ���Error" + to_wstring(server_updata_error);
							if (!server_updata_error_reason.empty()) text += L"\n" + server_updata_error_reason;
						}
					}

					int left_x = 10, right_x = 760;

					std::vector<std::string> lines;
					std::wstring line, temp;
					std::wstringstream ss(text);

					while (getline(ss, temp, L'\n'))
					{
						bool flag = false;
						line = L"";

						for (wchar_t ch : temp)
						{
							flag = false;

							float text_width = ImGui::CalcTextSize(convert_to_utf8(wstring_to_string(line + ch)).c_str()).x;
							if (text_width > (right_x - left_x))
							{
								lines.emplace_back(convert_to_utf8(wstring_to_string(line)));
								line = L"", flag = true;
							}

							line += ch;
						}

						if (!flag) lines.emplace_back(convert_to_utf8(wstring_to_string(line)));
					}

					for (const auto& temp : lines)
					{
						float text_width = ImGui::CalcTextSize(temp.c_str()).x;
						float text_indentation = ((right_x - left_x) - text_width) * 0.5f;
						if (text_indentation < 0)  text_indentation = 0;
						ImGui::SetCursorPosX(left_x + text_indentation);
						ImGui::TextUnformatted(temp.c_str());
					}
				}

				break;
			case Tab::tab5:
				// ��������
				Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
				{
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
					wstring text;
					{
						text = L"��������\n\n";
						text += L"���ȷǳ���л��ʹ���ǻ�̣�Ҳ��л�����ǻ�̵�֧��~\n\n";
						text += L"�ǻ���� GPLv3 ��Դ����������ӭ��Ҽ����ǻ�̵Ŀ���\n";
						text += L"Ŀǰ���������ǳ�����ڴ�ռ�����⣬�ǻ��ռ�õ��ڴ�����̫���ˣ�\n�¸��汾���ƾͻ���������\n\n";
						text += L"�����ҷ����ǻ�̵��û���������ӣ��ҷǳ���л��λ���ǻ�̵�֧��\nҲ��л�����к�վ�����ǻ�̵��ƹ���϶�\n��������Ҳ�õ�������û�����������汾����ר���޸�CPUռ���ʸߣ�ͬʱ�������һЩС����\n\n";
						text += L"�һᾡ�����ܿ����ǻ�̣����Ǹ��µ��ٶȺ��¹��ܵ�ʵ���ٶȿ϶��Ȳ�����ҵ���\n������ѧ��ʱ����ȣ���д��ô";
					}

					int left_x = 10, right_x = 760;

					std::vector<std::string> lines;
					std::wstring line, temp;
					std::wstringstream ss(text);

					while (getline(ss, temp, L'\n'))
					{
						bool flag = false;
						line = L"";

						for (wchar_t ch : temp)
						{
							flag = false;

							float text_width = ImGui::CalcTextSize(convert_to_utf8(wstring_to_string(line + ch)).c_str()).x;
							if (text_width > (right_x - left_x))
							{
								lines.emplace_back(convert_to_utf8(wstring_to_string(line)));
								line = L"", flag = true;
							}

							line += ch;
						}

						if (!flag) lines.emplace_back(convert_to_utf8(wstring_to_string(line)));
					}

					for (const auto& temp : lines)
					{
						float text_width = ImGui::CalcTextSize(temp.c_str()).x;
						float text_indentation = ((right_x - left_x) - text_width) * 0.5f;
						if (text_indentation < 0)  text_indentation = 0;
						ImGui::SetCursorPosX(left_x + text_indentation);
						ImGui::TextUnformatted(temp.c_str());
					}
				}

				break;
			}

			ImGui::EndChild();

			ImGui::SetCursorPos({ 120.0f,44.0f + 616.0f + 5.0f });
			Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
			PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
			CenteredText(u8"�������ö����Զ����沢������Ч", 4.0f);

			ImGui::End();

			ImGui::PopStyleColor(PushStyleColorNum);
			while (PushFontNum) PushFontNum--, ImGui::PopFont();
		}

		// ��Ⱦ
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		g_pSwapChain->Present(1, 0); // ͬ����ʾ
		//g_pSwapChain->Present(0, 0); // ��ͬ����ʾ

		if (!test.select)
		{
			if (ShowWindow && IsWindowVisible(setting_window)) ::ShowWindow(setting_window, SW_HIDE), ShowWindow = false;
			while (!test.select) Sleep(100);
		}
		else if (!ShowWindow && !IsWindowVisible(setting_window))
		{
			::ShowWindow(setting_window, SW_SHOW);
			::SetForegroundWindow(setting_window);
			ShowWindow = true;
		}
	}
	::ShowWindow(setting_window, SW_HIDE);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	//ImGui::DestroyContext(); //��֪��Ϊɶ���˾ͱ���

	CleanupDeviceD3D();
	::DestroyWindow(setting_window);
	::UnregisterClass(ImGuiWc.lpszClassName, ImGuiWc.hInstance);

	thread_status[L"SettingMain"] = false;

	return 0;
}

void FirstSetting(bool info)
{
	if (info && MessageBox(floating_window, L"��ӭʹ���ǻ��~\n���򵼻��ڽ����У�Ŀǰ��Ϊ��ª�����½�\n\n�Ƿ��������ǻ�̿����Զ�������", L"�ǻ���״�ʹ����alpha", MB_YESNO | MB_SYSTEMMODAL) == 6)
	{
		if (_waccess((string_to_wstring(global_path) + L"api").c_str(), 0) == -1) filesystem::create_directory(string_to_wstring(global_path) + L"api");
		ExtractResource((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), L"EXE", MAKEINTRESOURCE(229));

		ShellExecute(NULL, L"runas", (string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), (L"/\"set\" /\"" + GetCurrentExePath() + L"\"").c_str(), NULL, SW_SHOWNORMAL);

		DWORD dwBytesRead;
		WCHAR buffer[4096];
		HANDLE hPipe = INVALID_HANDLE_VALUE;
		wstring treceivedData;

		int for_i;
		for (for_i = 0; for_i <= 5000; for_i++)
		{
			if (WaitNamedPipe(TEXT("\\\\.\\pipe\\IDTPipe1"), 100)) break;
			else Sleep(100);
		}

		if (for_i <= 5000)
		{
			hPipe = CreateFile(TEXT("\\\\.\\pipe\\IDTPipe1"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (ReadFile(hPipe, buffer, sizeof(buffer), &dwBytesRead, NULL)) treceivedData.assign(buffer, dwBytesRead / sizeof(WCHAR));
		}

		CloseHandle(hPipe);
	}

	if (_waccess((string_to_wstring(global_path) + L"opt").c_str(), 0) == -1) filesystem::create_directory(string_to_wstring(global_path) + L"opt");
	Json::StyledWriter outjson;
	Json::Value root;

	root["edition"] = Json::Value(edition_date);
	root["CreateLnk"] = Json::Value(true);
	root["BrushRecover"] = Json::Value(true);
	root["RubberRecover"] = Json::Value(false);
	root["SetSkinMode"] = Json::Value(0);

	ofstream writejson;
	writejson.imbue(locale("zh_CN.UTF8"));
	writejson.open(wstring_to_string(string_to_wstring(global_path) + L"opt\\deploy.json").c_str());
	writejson << outjson.write(root);
	writejson.close();
}
bool ReadSetting(bool first)
{
	Json::Reader reader;
	Json::Value root;

	ifstream readjson;
	readjson.imbue(locale("zh_CN.UTF8"));
	readjson.open(wstring_to_string(string_to_wstring(global_path) + L"opt\\deploy.json").c_str());

	if (reader.parse(readjson, root))
	{
		if (root.isMember("CreateLnk")) setlist.CreateLnk = root["CreateLnk"].asBool();
		if (root.isMember("BrushRecover")) setlist.BrushRecover = root["BrushRecover"].asBool();
		if (root.isMember("RubberRecover")) setlist.RubberRecover = root["RubberRecover"].asBool();
		if (root.isMember("SetSkinMode")) setlist.SetSkinMode = root["SetSkinMode"].asInt();

		//Ԥ����
		if (first)
		{
			wstring time = CurrentDate();

			if (setlist.SetSkinMode == 0)
			{
				if (L"2024-01-22" <= time && time <= L"2024-02-23") setlist.SkinMode = 3;
				else setlist.SkinMode = 1;
			}
			else setlist.SkinMode = setlist.SetSkinMode;
		}
	}

	readjson.close();

	return true;
}
bool WriteSetting()
{
	Json::StyledWriter outjson;
	Json::Value root;

	root["edition"] = Json::Value(edition_date);
	root["CreateLnk"] = Json::Value(setlist.CreateLnk);
	root["BrushRecover"] = Json::Value(setlist.BrushRecover);
	root["RubberRecover"] = Json::Value(setlist.RubberRecover);
	root["SetSkinMode"] = Json::Value(setlist.SetSkinMode);

	ofstream writejson;
	writejson.imbue(locale("zh_CN.UTF8"));
	writejson.open(wstring_to_string(string_to_wstring(global_path) + L"opt\\deploy.json").c_str());
	writejson << outjson.write(root);
	writejson.close();

	return true;
}

// ��������
bool CreateDeviceD3D(HWND hWnd)
{
	// ���ý�����
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	// createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // ���Ӳ�������ã��볢��ʹ�ø����ܵ� WARP �����������
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}
void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}
void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

bool LoadTextureFromFile(unsigned char* image_data, int width, int height, ID3D11ShaderResourceView** out_srv)
{
	if (image_data == NULL) return false;

	// Create texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = image_data;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
	pTexture->Release();

	return true;
}

// �� imgui_impl_win32.cpp ��ǰ��������Ϣ������
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// Win32 ��Ϣ������
// �������Ķ� io.WantCaptureMouse��io.WantCaptureKeyboard ��־�����˽� dear imgui �Ƿ���ʹ���������롣
// - �� io.WantCaptureMouse Ϊ true ʱ����������������ݷ��͵���Ӧ�ó��򣬻������/����������ݵĸ�����
// - �� io.WantCaptureKeyboard Ϊ true ʱ�����𽫼����������ݷ��͵���Ӧ�ó��򣬻������/���Ǽ������ݵĸ�����
// ͨ����������ʼ�ս��������봫�ݸ� dear imgui����������������־��Ӧ�ó������������ǡ�
LRESULT WINAPI ImGuiWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		g_ResizeWidth = (UINT)LOWORD(lParam); // �Ŷӵ�����С
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // ���� ALT Ӧ�ó���˵�
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}