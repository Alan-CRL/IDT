#include "IdtSetting.h"

#include "IdtConfiguration.h"
#include "IdtDraw.h"
#include "IdtDrawpad.h"
#include "IdtHistoricalDrawpad.h"
#include "IdtImage.h"
#include "IdtMagnification.h"
#include "IdtOther.h"
#include "IdtPlug-in.h"
#include "IdtRts.h"
#include "IdtText.h"
#include "IdtUpdate.h"
#include "IdtWindow.h"

#include "imgui/imconfig.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_toggle.h"
#include "imgui/imgui_toggle_presets.h"
#include "imgui/imstb_rectpack.h"
#include "imgui/imstb_textedit.h"
#include "imgui/imstb_truetype.h"
#include <tchar.h>

// ʾ��
static void HelpMarker(const char* desc, ImVec4 Color);
static void CenteredText(const char* desc, float displacement);

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

void SettingSeekBar()
{
	if (!KeyBoradDown[VK_LBUTTON]) return;

	POINT p;
	GetCursorPos(&p);

	int pop_x = p.x - SettingWindowX;
	int pop_y = p.y - SettingWindowY;

	while (1)
	{
		if (!KeyBoradDown[VK_LBUTTON]) break;

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

	// ��ʼ������
	{
		ImGuiWc = { sizeof(WNDCLASSEX), CS_CLASSDC, ImGuiWndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Idt ImGui Tool"), NULL };
		::RegisterClassEx(&ImGuiWc);
		setting_window = ::CreateWindow(ImGuiWc.lpszClassName, _T("Idt ImGui Tool"), WS_OVERLAPPEDWINDOW, SettingWindowX, SettingWindowY, SettingWindowWidth, SettingWindowHeight, NULL, NULL, ImGuiWc.hInstance, NULL);

		SetWindowLong(setting_window, GWL_STYLE, GetWindowLong(setting_window, GWL_STYLE) & ~(WS_CAPTION | WS_BORDER | WS_THICKFRAME));
		SetWindowPos(setting_window, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);

		::ShowWindow(setting_window, SW_HIDE);
		::UpdateWindow(setting_window);

		magnificationWindowReady++;

		CreateDeviceD3D(setting_window);

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
						data[(y * width + x) * 4 + 0] = unsigned char(((color & 0x00FF0000) >> 16) * 255 / alpha);
						data[(y * width + x) * 4 + 1] = unsigned char(((color & 0x0000FF00) >> 8) * 255 / alpha);
						data[(y * width + x) * 4 + 2] = unsigned char(((color & 0x000000FF) >> 0) * 255 / alpha);
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
						data[(y * width + x) * 4 + 0] = unsigned char(((color & 0x00FF0000) >> 16) * 255 / alpha);
						data[(y * width + x) * 4 + 1] = unsigned char(((color & 0x0000FF00) >> 8) * 255 / alpha);
						data[(y * width + x) * 4 + 2] = unsigned char(((color & 0x000000FF) >> 0) * 255 / alpha);
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
						data[(y * width + x) * 4 + 0] = unsigned char(((color & 0x00FF0000) >> 16) * 255 / alpha);
						data[(y * width + x) * 4 + 1] = unsigned char(((color & 0x0000FF00) >> 8) * 255 / alpha);
						data[(y * width + x) * 4 + 2] = unsigned char(((color & 0x000000FF) >> 0) * 255 / alpha);
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
	}

	bool ShowWindow = false;
	while (!off_signal)
	{
		::ShowWindow(setting_window, SW_HIDE);
		ShowWindow = false;

		while (!test.select && !off_signal) Sleep(100);
		if (off_signal) break;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.IniFilename = nullptr;

		ImGui::StyleColorsLight();

		ImGui_ImplWin32_Init(setting_window);
		ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

		if (_waccess((string_to_wstring(global_path) + L"ttf\\hmossscr.ttf").c_str(), 0) == -1)
		{
			if (_waccess((string_to_wstring(global_path) + L"ttf").c_str(), 0) == -1)
			{
				error_code ec;
				filesystem::create_directory(string_to_wstring(global_path) + L"ttf", ec);
			}
			ExtractResource((string_to_wstring(global_path) + L"ttf\\hmossscr.ttf").c_str(), L"TTF", MAKEINTRESOURCE(198));
		}

		ImFont* Font = io.Fonts->AddFontFromFileTTF(convert_to_utf8(global_path + "ttf\\hmossscr.ttf").c_str(), 28.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		ImGuiStyle& style = ImGui::GetStyle();
		auto Color = style.Colors;

		style.ChildRounding = 8.0f;
		style.FrameRounding = 8.0f;
		style.GrabRounding = 8.0f;

		style.Colors[ImGuiCol_WindowBg] = ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f);

		//��ʼ���������
		hiex::tDelayFPS recond;
		POINT MoushPos = { 0,0 };
		ImGuiToggleConfig config;
		config.FrameRounding = 0.3f;
		config.KnobRounding = 0.5f;
		config.Size = { 60.0f,30.0f };
		config.Flags = ImGuiToggleFlags_Animated;

		int QuestNumbers = 0;
		int PushStyleColorNum = 0, PushFontNum = 0;
		int QueryWaitingTime = 5;

		bool StartUp = false, CreateLnk = setlist.CreateLnk;
		bool BrushRecover = setlist.BrushRecover, RubberRecover = setlist.RubberRecover;
		int RubberMode = setlist.RubberMode;
		string UpdateChannel = setlist.UpdateChannel;
		bool IntelligentDrawing = setlist.IntelligentDrawing, SmoothWriting = setlist.SmoothWriting;
		int SetSkinMode = setlist.SetSkinMode;

		wstring ppt_LinkTest = LinkTest();
		wstring ppt_IsPptDependencyLoaded = L"���Ӧ��û����(״̬��ʾ�������⣬�������޸�)";// = IsPptDependencyLoaded(); //TODO ��������
		wstring receivedData;
		POINT pt;

		while (!off_signal)
		{
			MSG msg;
			while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
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

			hiex::DelayFPS(recond, 24);

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
					tab5,
					tab6,
					tab7,
					tab8
				};

				ImGui::SetNextWindowPos({ 0,0 });//���ô���λ��
				ImGui::SetNextWindowSize({ static_cast<float>(SettingWindowWidth),static_cast<float>(SettingWindowHeight) });//���ô��ڴ�С

				Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
				ImGui::Begin(reinterpret_cast<const char*>(u8"�ǻ��ѡ��"), &test.select, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);//��ʼ���ƴ���

				{
					ImGui::SetCursorPos({ 10.0f,44.0f });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab1 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab1 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
					if (Tab == Tab::tab1) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (Tab == Tab::tab1) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (ImGui::Button(reinterpret_cast<const char*>(u8"��ҳ"), { 100.0f,40.0f }))
					{
						Tab = Tab::tab1;
					}
					while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

					ImGui::SetCursorPos({ 10.0f,94.0f });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab2 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab2 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
					if (Tab == Tab::tab2) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (Tab == Tab::tab2) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (ImGui::Button(reinterpret_cast<const char*>(u8"ͨ��"), { 100.0f,40.0f }))
					{
						Tab = Tab::tab2;
					}
					while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

					ImGui::SetCursorPos({ 10.0f,144.0f });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab3 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab3 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
					if (Tab == Tab::tab3) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (Tab == Tab::tab3) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (ImGui::Button(reinterpret_cast<const char*>(u8"����"), { 100.0f,40.0f }))
					{
						Tab = Tab::tab3;
					}
					while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

					ImGui::SetCursorPos({ 10.0f,194.0f });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab4 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab4 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
					if (Tab == Tab::tab4) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (Tab == Tab::tab4) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (ImGui::Button(reinterpret_cast<const char*>(u8"��ݼ�"), { 100.0f,40.0f }))
					{
						Tab = Tab::tab4;
					}
					while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

					ImGui::SetCursorPos({ 10.0f,244.0f });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab5 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab5 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
					if (Tab == Tab::tab5) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (Tab == Tab::tab5) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (ImGui::Button(reinterpret_cast<const char*>(u8"ʵ����"), { 100.0f,40.0f }))
					{
						Tab = Tab::tab5;
					}
					while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

					ImGui::SetCursorPos({ 10.0f,294.0f });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab6 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab6 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
					if (Tab == Tab::tab6) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (Tab == Tab::tab6) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (ImGui::Button(reinterpret_cast<const char*>(u8"�������"), { 100.0f,40.0f }))
					{
						Tab = Tab::tab6;
					}
					while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

					ImGui::SetCursorPos({ 10.0f,344.0f });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::tab8 ? ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, Tab == Tab::tab8 ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
					if (Tab == Tab::tab8) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (Tab == Tab::tab8) PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
					if (ImGui::Button(reinterpret_cast<const char*>(u8"����"), { 100.0f,40.0f }))
					{
						Tab = Tab::tab8;
					}
					while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();

					Font->Scale = 0.65f, ImGui::PushFont(Font);
					ImGui::SetCursorPos({ 10.0f,44.0f + 616.0f - 110.0f });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(195 / 255.0f, 195 / 255.0f, 195 / 255.0f, 1.0f));
					if (ImGui::Button(reinterpret_cast<const char*>(u8"��ʱͣ������"), { 100.0f,40.0f }))
					{
						Tab = Tab::tab7;
					}
					while (PushStyleColorNum) PushStyleColorNum--, ImGui::PopStyleColor();
					ImGui::PopFont();

					ImGui::SetCursorPos({ 10.0f,44.0f + 616.0f - 65.0f });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(195 / 255.0f, 195 / 255.0f, 195 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
					if (ImGui::Button(reinterpret_cast<const char*>(u8"��������"), { 100.0f,30.0f }))
					{
						test.select = false;
						off_signal = 2;
					}

					ImGui::SetCursorPos({ 10.0f,44.0f + 616.0f - 30.0f });
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(195 / 255.0f, 195 / 255.0f, 195 / 255.0f, 1.0f));
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
					if (ImGui::Button(reinterpret_cast<const char*>(u8"�رճ���"), { 100.0f,30.0f }))
					{
						test.select = false;
						off_signal = true;
					}
				}

				ImGui::SetCursorPos({ 120.0f,44.0f });
				ImGui::BeginChild(reinterpret_cast<const char*>(u8"������"), { 770.0f,616.0f }, true);
				switch (Tab)
				{
				case Tab::tab1:
					// ��ҳ
					Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);

					{
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(235 / 255.0f, 10 / 255.0f, 20 / 255.0f, 1.0f));

						ImGui::SetCursorPosY(10.0f);
						int left_x = 10, right_x = 760;

						std::vector<std::string> lines;
						std::wstring line, temp;
						std::wstringstream ss(L"�Ƽ�ʹ�� 1080P �ֱ��ʣ����ڴ˷ֱ��ʿ��ܻ��������ⲢӰ������\n�߷ֱ�����Ļ�����佫�� UI3.0 ���Ժ�ͬ�����п���");

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

					ImGui::SetCursorPos({ 35.0f,60.0f });
					ImGui::Image((void*)TextureSettingSign[2], ImVec2((float)SettingSign[2].getwidth(), (float)SettingSign[2].getheight()));

					{
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));

						ImGui::SetCursorPosY(616.0f - 168.0f);
						std::wstring author = L"���������ϵ��ʽ\nQQ: 2685549821\nEmail: alan-crl@foxmail.com";
						int left_x = 10, right_x = 370;

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
						int left_x = 10, right_x = 370;

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
						ImGui::BeginChild(reinterpret_cast<const char*>(u8"���򻷾�"), { 730.0f,125.0f }, true, ImGuiWindowFlags_NoScrollbar);

						{
							ImGui::SetCursorPosY(10.0f);

							Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							CenteredText(reinterpret_cast<const char*>(u8" ��ѯ��������״̬"), 4.0f);

							Font->Scale = 0.7f, PushFontNum++, ImGui::PushFont(Font);
							ImGui::SameLine(); HelpMarker(reinterpret_cast<const char*>(u8"���������Զ���������ǰ��Ҫ��ѯ��ǰ״̬�������������ԱȨ�ޣ�"), ImGui::GetStyleColorVec4(ImGuiCol_Text));

							if (!receivedData.empty())
							{
								string temp, helptemp;
								if (receivedData.length() >= 5 && receivedData.substr(0, 5) == L"Succe") temp = reinterpret_cast<const char*>(u8"��ѯ״̬�ɹ�"), helptemp = reinterpret_cast<const char*>(u8"���Ե���������������");
								else if (receivedData.length() >= 5 && receivedData.substr(0, 5) == L"Error") temp = reinterpret_cast<const char*>(u8"��ѯ״̬���� ") + wstring_to_string(receivedData), helptemp = reinterpret_cast<const char*>(u8"�ٴε����ѯ���ԣ������������Թ���Ա�������");
								else if (receivedData.length() >= 5 && receivedData.substr(0, 5) == L"TimeO") temp = reinterpret_cast<const char*>(u8"��ѯ״̬��ʱ"), helptemp = reinterpret_cast<const char*>(u8"�ٴε����ѯ����\nͬʱ��ʱʱ�佫�� 5 �����Ϊ 15 ��"), QueryWaitingTime = 15;
								else if (receivedData.length() >= 5 && receivedData.substr(0, 5) == L"Renew") temp = reinterpret_cast<const char*>(u8"��Ҫ���²�ѯ״̬"), helptemp = reinterpret_cast<const char*>(u8"���������Զ���������ʱ��ʱ�����ٴε����ѯ\nͬʱ��ʱʱ�佫�� 5 �����Ϊ 15 ��"), QueryWaitingTime = 15;
								else temp = reinterpret_cast<const char*>(u8"δ֪����"), helptemp = reinterpret_cast<const char*>(u8"�ٴε����ѯ���ԣ������������Թ���Ա�������");

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
							if (ImGui::Button(reinterpret_cast<const char*>(u8"��ѯ"), { 60.0f,30.0f }))
							{
								{
									// ��鱾���ļ�������
									{
										if (_waccess((string_to_wstring(global_path) + L"api").c_str(), 0) == -1)
										{
											error_code ec;
											filesystem::create_directory(string_to_wstring(global_path) + L"api", ec);
										}

										string StartupItemSettingsMd5 = "abaadf527bc925a85507a8361d2d9d44";
										string StartupItemSettingsSHA256 = "57df0ed39d797fd286deb583d81450bb8838b98ce92741e91c9a01d9a4ac3b81";

										if (_waccess((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), 0) == -1)
											ExtractResource((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), L"EXE", MAKEINTRESOURCE(229));
										else
										{
											string hash_md5, hash_sha256;
											{
												hashwrapper* myWrapper = new md5wrapper();
												hash_md5 = myWrapper->getHashFromFile(global_path + "api\\�ǻ��StartupItemSettings.exe");
												delete myWrapper;
											}
											{
												hashwrapper* myWrapper = new sha256wrapper();
												hash_sha256 = myWrapper->getHashFromFile(global_path + "api\\�ǻ��StartupItemSettings.exe");
												delete myWrapper;
											}

											if (hash_md5 != StartupItemSettingsMd5 || hash_sha256 != StartupItemSettingsSHA256)
												ExtractResource((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), L"EXE", MAKEINTRESOURCE(229));
										}
									}

									ShellExecute(NULL, L"runas", (string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), (L"/\"query" + to_wstring(QuestNumbers) + L"\" /\"" + GetCurrentExePath() + L"\" /\"xmg_drawpad_startup\"").c_str(), NULL, SW_SHOWNORMAL);
								}
								//if (_waccess((string_to_wstring(global_path) + L"api").c_str(), 0) == -1) filesystem::create_directory(string_to_wstring(global_path) + L"api");
								//ExtractResource((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), L"EXE", MAKEINTRESOURCE(229));
								//ShellExecute(NULL, L"runas", (string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), (L"/\"query" + to_wstring(QuestNumbers) + L"\" /\"" + GetCurrentExePath() + L"\" /\"xmg_drawpad_startup\"").c_str(), NULL, SW_SHOWNORMAL);

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
										if (receivedData == to_wstring(QuestNumbers) + L"fail")
										{
											receivedData = L"Success";
											setlist.StartUp = 1, StartUp = false;
										}
										else if (receivedData == to_wstring(QuestNumbers) + L"success")
										{
											receivedData = L"Success";
											setlist.StartUp = 2, StartUp = true;
										}
										else receivedData = L"Error unknown";
									}
									else receivedData = L"Error" + to_wstring(GetLastError());
								}
								CloseHandle(hPipe);

								QuestNumbers++, QuestNumbers %= 10;
							}
						}
						{
							ImGui::SetCursorPosY(45.0f);

							Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							CenteredText(reinterpret_cast<const char*>(u8" �����Զ�����"), 4.0f);

							Font->Scale = 0.7f, PushFontNum++, ImGui::PushFont(Font);
							ImGui::SameLine();
							if (setlist.StartUp) HelpMarker(reinterpret_cast<const char*>(u8"�����������ԱȨ��"), ImGui::GetStyleColorVec4(ImGuiCol_Text));
							else HelpMarker(reinterpret_cast<const char*>(u8"���������Զ���������ǰ��Ҫ��ѯ��ǰ״̬�������������ԱȨ�ޣ�\n�����Ϸ���ť��ѯ��ǰ״̬"), ImGui::GetStyleColorVec4(ImGuiCol_Text));

							if (setlist.StartUp)
							{
								PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
								PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 101 / 255.0f, 205 / 255.0f, 1.0f));
								PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
								PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
								PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
								ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 70.0f);
								ImGui::Toggle(reinterpret_cast<const char*>(u8"##�����Զ�����"), &StartUp, config);

								if (setlist.StartUp - 1 != (int)StartUp)
								{
									{
										// ��鱾���ļ�������
										{
											if (_waccess((string_to_wstring(global_path) + L"api").c_str(), 0) == -1)
											{
												error_code ec;
												filesystem::create_directory(string_to_wstring(global_path) + L"api", ec);
											}

											string StartupItemSettingsMd5 = "abaadf527bc925a85507a8361d2d9d44";
											string StartupItemSettingsSHA256 = "57df0ed39d797fd286deb583d81450bb8838b98ce92741e91c9a01d9a4ac3b81";

											if (_waccess((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), 0) == -1)
												ExtractResource((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), L"EXE", MAKEINTRESOURCE(229));
											else
											{
												string hash_md5, hash_sha256;
												{
													hashwrapper* myWrapper = new md5wrapper();
													hash_md5 = myWrapper->getHashFromFile(global_path + "api\\�ǻ��StartupItemSettings.exe");
													delete myWrapper;
												}
												{
													hashwrapper* myWrapper = new sha256wrapper();
													hash_sha256 = myWrapper->getHashFromFile(global_path + "api\\�ǻ��StartupItemSettings.exe");
													delete myWrapper;
												}

												if (hash_md5 != StartupItemSettingsMd5 || hash_sha256 != StartupItemSettingsSHA256)
													ExtractResource((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), L"EXE", MAKEINTRESOURCE(229));
											}
										}

										if (StartUp) ShellExecute(NULL, L"runas", (string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), (L"/\"set" + to_wstring(QuestNumbers) + L"\" /\"" + GetCurrentExePath() + L"\" /\"xmg_drawpad_startup\"").c_str(), NULL, SW_SHOWNORMAL);
										else ShellExecute(NULL, L"runas", (string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), (L"/\"delete" + to_wstring(QuestNumbers) + L"\" /\"" + GetCurrentExePath() + L"\" /\"xmg_drawpad_startup\"").c_str(), NULL, SW_SHOWNORMAL);
									}
									//if (_waccess((string_to_wstring(global_path) + L"api").c_str(), 0) == -1) filesystem::create_directory(string_to_wstring(global_path) + L"api");
									//ExtractResource((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), L"EXE", MAKEINTRESOURCE(229));

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
											if (treceivedData == to_wstring(QuestNumbers) + L"fail") setlist.StartUp = 1, StartUp = false;
											else if (treceivedData == to_wstring(QuestNumbers) + L"success") setlist.StartUp = 2, StartUp = true;
										}
									}
									else setlist.StartUp = 0, receivedData = L"Renew";

									CloseHandle(hPipe);
									QuestNumbers++, QuestNumbers %= 10;
								}
							}
						}
						{
							ImGui::SetCursorPosY(80.0f);

							Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							CenteredText(reinterpret_cast<const char*>(u8" ����ʱ���������ݷ�ʽ"), 4.0f);

							Font->Scale = 0.7f, PushFontNum++, ImGui::PushFont(Font);
							ImGui::SameLine();
							HelpMarker(reinterpret_cast<const char*>(u8"������ÿ������ʱ�����洴����ݷ�ʽ\n��������ܽ������Ϊ�������ӵ�и�����Զ��幦��"), ImGui::GetStyleColorVec4(ImGuiCol_Text));

							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 101 / 255.0f, 205 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
							ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 70.0f);
							ImGui::Toggle(reinterpret_cast<const char*>(u8"##����ʱ���������ݷ�ʽ"), &CreateLnk, config);

							if (setlist.CreateLnk != CreateLnk)
							{
								setlist.CreateLnk = CreateLnk;
								WriteSetting();

								if (CreateLnk) SetShortcut();
							}
						}

						ImGui::EndChild();
					}
					{
						ImGui::SetCursorPosX(20.0f);
						ImGui::BeginChild(reinterpret_cast<const char*>(u8"��۵���"), { 730.0f,50.0f }, true, ImGuiWindowFlags_NoScrollbar);

						{
							ImGui::SetCursorPosY(8.0f);

							Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							CenteredText(reinterpret_cast<const char*>(u8" ���Ƥ��"), 4.0f);

							ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 130.0f);
							static const char* items[] = { reinterpret_cast<const char*>(u8"  �Ƽ�Ƥ��"), reinterpret_cast<const char*>(u8"  Ĭ��Ƥ��"), reinterpret_cast<const char*>(u8"  ����ʱ��"), reinterpret_cast<const char*>(u8"  ����ӭ��") };
							ImGui::SetNextItemWidth(120);

							Font->Scale = 0.82f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(195 / 255.0f, 195 / 255.0f, 195 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							ImGui::Combo(reinterpret_cast<const char*>(u8"##���Ƥ��"), &SetSkinMode, items, IM_ARRAYSIZE(items));

							if (setlist.SetSkinMode != SetSkinMode)
							{
								setlist.SetSkinMode = SetSkinMode;
								WriteSetting();

								if (SetSkinMode == 0) setlist.SkinMode = 1;
								else setlist.SkinMode = SetSkinMode;
							}
						}

						ImGui::EndChild();
					}
					{
						ImGui::SetCursorPosX(20.0f);
						ImGui::BeginChild(reinterpret_cast<const char*>(u8"���ʵ���"), { 730.0f,90.0f }, true, ImGuiWindowFlags_NoScrollbar);

						{
							ImGui::SetCursorPosY(10.0f);

							Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							CenteredText(reinterpret_cast<const char*>(u8" ���ʻ���ʱ�Զ���������"), 4.0f);

							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 101 / 255.0f, 205 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
							ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 70.0f);
							ImGui::Toggle(reinterpret_cast<const char*>(u8"##���ʻ���ʱ�Զ���������"), &BrushRecover, config);

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
							CenteredText(reinterpret_cast<const char*>(u8" ��Ƥ����ʱ�Զ���������"), 4.0f);

							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 101 / 255.0f, 205 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
							ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 70.0f);
							ImGui::Toggle(reinterpret_cast<const char*>(u8"##��Ƥ����ʱ�Զ���������"), &RubberRecover, config);

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
					// ����
					ImGui::SetCursorPosY(20.0f);
					{
						ImGui::SetCursorPosX(20.0f);
						ImGui::BeginChild(reinterpret_cast<const char*>(u8"���ܻ�ͼ����"), { 730.0f,90.0f }, true, ImGuiWindowFlags_NoScrollbar);

						{
							ImGui::SetCursorPosY(10.0f);

							Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							CenteredText(reinterpret_cast<const char*>(u8" ���ܻ�ͼ"), 4.0f);

							Font->Scale = 0.7f, PushFontNum++, ImGui::PushFont(Font);
							ImGui::SameLine(); HelpMarker(reinterpret_cast<const char*>(u8"����ʱͣ�����Խ���ֱ�����Ƶ�ī����ֱ\ņ��ʱ���Խ���ֱ�����Ƶ�ī����ֱ�����Ƚϸߣ�\n������ֱ�������;�������"), ImGui::GetStyleColorVec4(ImGuiCol_Text));

							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 101 / 255.0f, 205 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
							ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 70.0f);
							ImGui::Toggle(reinterpret_cast<const char*>(u8"##���ܻ�ͼ"), &IntelligentDrawing, config);

							if (setlist.IntelligentDrawing != IntelligentDrawing)
							{
								setlist.IntelligentDrawing = IntelligentDrawing;
								WriteSetting();
							}
						}
						{
							ImGui::SetCursorPosY(45.0f);

							Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							CenteredText(reinterpret_cast<const char*>(u8" ƽ��ī��"), 4.0f);

							Font->Scale = 0.7f, PushFontNum++, ImGui::PushFont(Font);
							ImGui::SameLine(); HelpMarker(reinterpret_cast<const char*>(u8"̧��ʱ�Զ�ƽ��ī��"), ImGui::GetStyleColorVec4(ImGuiCol_Text));

							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0 / 255.0f, 111 / 255.0f, 225 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0 / 255.0f, 101 / 255.0f, 205 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f));
							ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 70.0f);
							ImGui::Toggle(reinterpret_cast<const char*>(u8"##ƽ��ī��"), &SmoothWriting, config);

							if (setlist.SmoothWriting != SmoothWriting)
							{
								setlist.SmoothWriting = SmoothWriting;
								WriteSetting();
							}
						}

						ImGui::EndChild();
					}
					{
						ImGui::SetCursorPosX(20.0f);
						ImGui::BeginChild(reinterpret_cast<const char*>(u8"��Ƥ����"), { 730.0f,50.0f }, true, ImGuiWindowFlags_NoScrollbar);

						{
							ImGui::SetCursorPosY(8.0f);

							Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							CenteredText(reinterpret_cast<const char*>(u8" ��Ƥ��ϸ������"), 4.0f);

							ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 130.0f);
							static const char* items[] = { reinterpret_cast<const char*>(u8"  �����豸"), reinterpret_cast<const char*>(u8"  PC ���") };
							ImGui::SetNextItemWidth(120);

							Font->Scale = 0.82f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(195 / 255.0f, 195 / 255.0f, 195 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							ImGui::Combo(reinterpret_cast<const char*>(u8"##��Ƥ��ϸ������"), &RubberMode, items, IM_ARRAYSIZE(items));

							if (setlist.RubberMode != RubberMode)
							{
								setlist.RubberMode = RubberMode;
								WriteSetting();
							}
						}

						ImGui::EndChild();
					}

					break;

				case Tab::tab4:
					//��ݼ�
					Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
					{
						ImGui::SetCursorPosY(10.0f);

						int left_x = 10, right_x = 760;

						std::vector<std::string> lines;
						std::wstring line, temp;
						std::wstringstream ss(L"���� Ctrl + Win + Alt �л�ѡ��/����\n\n�����ݼ����Զ������ڲ��ԣ������ڴ�");

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
					// ʵ����
					ImGui::SetCursorPosY(20.0f);
					{
						ImGui::SetCursorPosX(20.0f);
						ImGui::BeginChild(reinterpret_cast<const char*>(u8"PPT �ؼ����ű���"), { 730.0f,50.0f }, true, ImGuiWindowFlags_NoScrollbar);

						{
							ImGui::SetCursorPosY(8.0f);

							Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							CenteredText(reinterpret_cast<const char*>(u8" ���Ƥ��"), 4.0f);

							ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 230.0f);
							ImGui::SetNextItemWidth(220);

							Font->Scale = 0.82f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(195 / 255.0f, 195 / 255.0f, 195 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0 / 255.0f, 131 / 255.0f, 245 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0 / 255.0f, 101 / 255.0f, 205 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							ImGui::SliderFloat(reinterpret_cast<const char*>(u8"##PPT �ؼ����ű���"), &PPTUIScale, 0.25f, 3.0f, reinterpret_cast<const char*>(u8"%.5f ������"));
						}

						ImGui::EndChild();
					}
					break;

				case Tab::tab6:
					//�������
					Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
					{
						ImGui::SetCursorPosY(10.0f);
						wstring text;
						{
							GetCursorPos(&pt);

							text = L"���������£�";
							text += KeyBoradDown[VK_LBUTTON] ? L"��" : L"��";
							text += L"\n������� " + to_wstring(pt.x) + L"," + to_wstring(pt.y);

							if (uRealTimeStylus == 2) text += L"\n\n���ؿ���Ϣ������";
							else if (uRealTimeStylus == 3) text += L"\n\n���ؿ���Ϣ��̧��";
							else if (uRealTimeStylus == 4) text += L"\n\n���ؿ���Ϣ���ƶ�";
							else text += L"\n\n���ؿ���Ϣ������";

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
							text += PptInfoState.TotalPage != -1 ? L"���ڲ���" : L"δ����";
							text += L"\nPPT ��ҳ������";
							text += to_wstring(PptInfoState.TotalPage);
							text += L"\nPPT ��ǰҳ��ţ�";
							text += to_wstring(PptInfoState.CurrentPage);
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

				case Tab::tab7:
					// ��ʱͣ������
					Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
					{
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
						wstring text;
						{
							text = L"��ʱͣ������\n\n";
							text += L"���ȷǳ���л��ʹ���ǻ�̣�Ҳ��л�����ǻ�̵�֧��~\n\n";
							text += L"�����߱��˼����п���Ŀǰֻ��80�������ˣ�������2�����ǻ�̽���ʱֹͣ����\n";
							text += L"��л��λ���ǻ�̵Ĵ���֧�֣�\n\n2024.03.24";
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

				case Tab::tab8:
					// ����
					ImGui::SetCursorPos({ 35.0f,ImGui::GetCursorPosY() + 40.0f });
					ImGui::Image((void*)TextureSettingSign[1], ImVec2((float)SettingSign[1].getwidth(), (float)SettingSign[1].getheight()));

					Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
					{
						ImGui::SetCursorPos({ 20.0f,ImGui::GetCursorPosY() + 20.0f });
						ImGui::BeginChild(reinterpret_cast<const char*>(u8"����ͨ������"), { 730.0f,50.0f }, true, ImGuiWindowFlags_NoScrollbar);

						{
							ImGui::SetCursorPosY(8.0f);

							Font->Scale = 1.0f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							CenteredText(reinterpret_cast<const char*>(u8" ����ͨ��"), 4.0f);

							Font->Scale = 0.7f, PushFontNum++, ImGui::PushFont(Font);
							ImGui::SameLine(); HelpMarker(reinterpret_cast<const char*>(u8"��ʽͨ��(LTS) �ṩ������֤���ȶ�����汾\n����ͨ��(Beta) �ṩ�ȶ���һ��ĳ���汾\n����ʽͨ�������δ�ύɱ����з��󱨴���\n\nһ�����£����޷�ͨ���Զ����»��˰汾\n��ѡ��ĸ���ͨ��������ʱ������л���Ĭ��ͨ��"), ImGui::GetStyleColorVec4(ImGuiCol_Text));

							ImGui::SameLine(); ImGui::SetCursorPosX(730.0f - 180.0f);
							ImGui::SetNextItemWidth(170);

							Font->Scale = 0.82f, PushFontNum++, ImGui::PushFont(Font);
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(195 / 255.0f, 195 / 255.0f, 195 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(235 / 255.0f, 235 / 255.0f, 235 / 255.0f, 1.0f));
							PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(215 / 255.0f, 215 / 255.0f, 215 / 255.0f, 1.0f));

							int UpdateChannelMode;
							if (UpdateChannel == "LTS" || UpdateChannel == "Beta")
							{
								if (UpdateChannel == "LTS") UpdateChannelMode = 0;
								else if (UpdateChannel == "Beta") UpdateChannelMode = 1;

								static const char* items1[] = { reinterpret_cast<const char*>(u8" ��ʽͨ��(LTS)"), reinterpret_cast<const char*>(u8" ����ͨ��(Beta)") };

								ImGui::Combo(reinterpret_cast<const char*>(u8"##����ͨ��"), &UpdateChannelMode, items1, IM_ARRAYSIZE(items1));
							}
							else
							{
								UpdateChannelMode = 2;

								static const char* items2[] = { reinterpret_cast<const char*>(u8" ��ʽͨ��(LTS)"), reinterpret_cast<const char*>(u8" ����ͨ��(Beta)"), reinterpret_cast<const char*>(u8" ����ͨ��") };

								ImGui::Combo(reinterpret_cast<const char*>(u8"##����ͨ��"), &UpdateChannelMode, items2, IM_ARRAYSIZE(items2));
							}

							bool flag = false;
							if (UpdateChannelMode == 0 && UpdateChannel != "LTS") UpdateChannel = "LTS", flag = true;
							else if (UpdateChannelMode == 1 && UpdateChannel != "Beta") UpdateChannel = "Beta", flag = true;

							if (flag && setlist.UpdateChannel != UpdateChannel)
							{
								AutomaticUpdateStep = 1;
								setlist.UpdateChannel = UpdateChannel;
								WriteSetting();
							}
							else if (setlist.UpdateChannel != UpdateChannel) UpdateChannel = setlist.UpdateChannel;
						}

						ImGui::EndChild();
					}
					{
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
						wstring text;
						{
							text = L"����汾���� " + string_to_wstring(edition_code);
							text += L"\n���򷢲��汾 " + string_to_wstring(edition_date) + L"(" + string_to_wstring(edition_channel) + L")";
							text += L"\n���򹹽�ʱ�� " + buildTime;

#ifdef IDT_RELEASE
							text += L"\n���򹹽�ģʽ IDT_RELEASE ";
#else
							text += L"\n���򹹽�ģʽ IDT_DEBUG���Ƿ�������汾��";
#endif

							if (userid == L"Error") text += L"\n�û�ID�޷���ȷʶ��";
							else text += L"\n�û�ID " + userid;

							text += L"\n\n�ڴ˰汾�У������������ݶ����ڱ��ؽ��д���";
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

				{
					ImGui::SetCursorPos({ 120.0f,44.0f + 616.0f + 5.0f });
					Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
					PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f));
					CenteredText(reinterpret_cast<const char*>(u8"�������ö����Զ����沢������Ч"), 4.0f);
				}
				{
					if (AutomaticUpdateStep == 0)
					{
						ImGui::SetCursorPos({ 120.0f + 770.0f - ImGui::CalcTextSize(reinterpret_cast<const char*>(u8"�����Զ�����δ����")).x,44.0f + 616.0f + 5.0f });
						Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(150 / 255.0f, 150 / 255.0f, 150 / 255.0f, 1.0f));
						CenteredText(reinterpret_cast<const char*>(u8"�����Զ�����δ����"), 4.0f);
					}
					else if (AutomaticUpdateStep == 1)
					{
						ImGui::SetCursorPos({ 120.0f + 770.0f - ImGui::CalcTextSize(reinterpret_cast<const char*>(u8"�����Զ�����������")).x,44.0f + 616.0f + 5.0f });
						Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(150 / 255.0f, 150 / 255.0f, 150 / 255.0f, 1.0f));
						CenteredText(reinterpret_cast<const char*>(u8"�����Զ�����������"), 4.0f);
					}
					else if (AutomaticUpdateStep == 2)
					{
						ImGui::SetCursorPos({ 120.0f + 770.0f - ImGui::CalcTextSize(reinterpret_cast<const char*>(u8"�����Զ������������Ӵ���")).x , 44.0f + 616.0f + 5.0f });
						Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(229 / 255.0f, 55 / 255.0f, 66 / 255.0f, 1.0f));
						CenteredText(reinterpret_cast<const char*>(u8"�����Զ������������Ӵ���"), 4.0f);
					}
					else if (AutomaticUpdateStep == 3)
					{
						ImGui::SetCursorPos({ 120.0f + 770.0f - ImGui::CalcTextSize(reinterpret_cast<const char*>(u8"�����Զ������������°汾��Ϣʱʧ��")).x , 44.0f + 616.0f + 5.0f });
						Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(229 / 255.0f, 55 / 255.0f, 66 / 255.0f, 1.0f));
						CenteredText(reinterpret_cast<const char*>(u8"�����Զ������������°汾��Ϣʱʧ��"), 4.0f);
					}
					else if (AutomaticUpdateStep == 4)
					{
						ImGui::SetCursorPos({ 120.0f + 770.0f - ImGui::CalcTextSize(reinterpret_cast<const char*>(u8"�����Զ��������ص����°汾��Ϣ�����Ϲ淶")).x , 44.0f + 616.0f + 5.0f });
						Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(229 / 255.0f, 55 / 255.0f, 66 / 255.0f, 1.0f));
						CenteredText(reinterpret_cast<const char*>(u8"�����Զ��������ص����°汾��Ϣ�����Ϲ淶"), 4.0f);
					}
					else if (AutomaticUpdateStep == 5)
					{
						ImGui::SetCursorPos({ 120.0f + 770.0f - ImGui::CalcTextSize(reinterpret_cast<const char*>(u8"�����Զ��������ص����°汾��Ϣ�в�������ѡ��ͨ��")).x , 44.0f + 616.0f + 5.0f });
						Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(229 / 255.0f, 55 / 255.0f, 66 / 255.0f, 1.0f));
						CenteredText(reinterpret_cast<const char*>(u8"�����Զ��������ص����°汾��Ϣ�в�������ѡ��ͨ��"), 4.0f);
					}
					else if (AutomaticUpdateStep == 6)
					{
						ImGui::SetCursorPos({ 120.0f + 770.0f - ImGui::CalcTextSize(reinterpret_cast<const char*>(u8"�°汾������������")).x , 44.0f + 616.0f + 5.0f });
						Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(229 / 255.0f, 55 / 255.0f, 66 / 255.0f, 1.0f));
						CenteredText(reinterpret_cast<const char*>(u8"�°汾������������"), 4.0f);
					}
					else if (AutomaticUpdateStep == 7)
					{
						ImGui::SetCursorPos({ 120.0f + 770.0f - ImGui::CalcTextSize(reinterpret_cast<const char*>(u8"�����Զ������������°汾����ʧ��")).x , 44.0f + 616.0f + 5.0f });
						Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(229 / 255.0f, 55 / 255.0f, 66 / 255.0f, 1.0f));
						CenteredText(reinterpret_cast<const char*>(u8"�����Զ������������°汾����ʧ��"), 4.0f);
					}
					else if (AutomaticUpdateStep == 8)
					{
						ImGui::SetCursorPos({ 120.0f + 770.0f - ImGui::CalcTextSize(reinterpret_cast<const char*>(u8"�����Զ��������ص����°汾������")).x , 44.0f + 616.0f + 5.0f });
						Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(229 / 255.0f, 55 / 255.0f, 66 / 255.0f, 1.0f));
						CenteredText(reinterpret_cast<const char*>(u8"�����Զ��������ص����°汾������"), 4.0f);
					}
					else if (AutomaticUpdateStep == 9)
					{
						ImGui::SetCursorPos({ 120.0f + 770.0f - ImGui::CalcTextSize(reinterpret_cast<const char*>(u8"�������µ����°汾")).x , 44.0f + 616.0f + 5.0f });
						Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(229 / 255.0f, 55 / 255.0f, 66 / 255.0f, 1.0f));
						CenteredText(reinterpret_cast<const char*>(u8"�������µ����°汾"), 4.0f);
					}
					else if (AutomaticUpdateStep == 10)
					{
						ImGui::SetCursorPos({ 120.0f + 770.0f - ImGui::CalcTextSize(convert_to_utf8("�����Ѿ������°汾��" + setlist.UpdateChannel + "ͨ����").c_str()).x , 44.0f + 616.0f + 5.0f });
						Font->Scale = 0.76923076f, PushFontNum++, ImGui::PushFont(Font);
						PushStyleColorNum++, ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(98 / 255.0f, 175 / 255.0f, 82 / 255.0f, 1.0f));
						CenteredText(reinterpret_cast<const char*>(convert_to_utf8("�����Ѿ������°汾��" + setlist.UpdateChannel + "ͨ����").c_str()), 4.0f);
					}
				}

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

			if (!test.select) break;
			if (!ShowWindow && !IsWindowVisible(setting_window))
			{
				::ShowWindow(setting_window, SW_SHOW);
				::SetForegroundWindow(setting_window);
				ShowWindow = true;
			}
		}

		//::ShowWindow(setting_window, SW_HIDE);

		io.Fonts->Clear();

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	thread_status[L"SettingMain"] = false;
	return 0;
}

void FirstSetting(bool info)
{
	if (info && MessageBox(floating_window, L"��ӭʹ���ǻ��~\n���򵼻��ڽ����У�Ŀǰ��Ϊ��ª�����½�\n\n�Ƿ��������ǻ�̿����Զ�������", L"�ǻ���״�ʹ����alpha", MB_YESNO | MB_SYSTEMMODAL) == 6)
	{
		if (_waccess((string_to_wstring(global_path) + L"api").c_str(), 0) == -1) filesystem::create_directory(string_to_wstring(global_path) + L"api");
		ExtractResource((string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), L"EXE", MAKEINTRESOURCE(229));

		ShellExecute(NULL, L"runas", (string_to_wstring(global_path) + L"api\\�ǻ��StartupItemSettings.exe").c_str(), (L"/\"set\" /\"" + GetCurrentExePath() + L"\" /\"xmg_drawpad_startup\"").c_str(), NULL, SW_SHOWNORMAL);

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
	root["CreateLnk"] = Json::Value(false);
	root["BrushRecover"] = Json::Value(true);
	root["RubberRecover"] = Json::Value(false);
	root["RubberMode"] = Json::Value(0);
	root["IntelligentDrawing"] = Json::Value(true);
	root["SmoothWriting"] = Json::Value(true);
	root["SetSkinMode"] = Json::Value(0);
	root["UpdateChannel"] = Json::Value("LTS");

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
		if (root.isMember("RubberMode")) setlist.RubberMode = root["RubberMode"].asBool();
		if (root.isMember("IntelligentDrawing")) setlist.IntelligentDrawing = root["IntelligentDrawing"].asBool();
		if (root.isMember("SmoothWriting")) setlist.SmoothWriting = root["SmoothWriting"].asBool();
		if (root.isMember("SetSkinMode")) setlist.SetSkinMode = root["SetSkinMode"].asInt();
		if (root.isMember("UpdateChannel")) setlist.UpdateChannel = root["UpdateChannel"].asString();

		//Ԥ����
		if (first)
		{
			if (setlist.SetSkinMode == 0) setlist.SkinMode = 1;
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
	root["RubberMode"] = Json::Value(setlist.RubberMode);
	root["IntelligentDrawing"] = Json::Value(setlist.IntelligentDrawing);
	root["SmoothWriting"] = Json::Value(setlist.SmoothWriting);
	root["SetSkinMode"] = Json::Value(setlist.SetSkinMode);
	root["UpdateChannel"] = Json::Value(setlist.UpdateChannel);

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

// ʾ��
static void HelpMarker(const char* desc, ImVec4 Color)
{
	ImGui::TextColored(Color, reinterpret_cast<const char*>(u8"(?)"));
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}
static void CenteredText(const char* desc, float displacement)
{
	float temp = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(temp + displacement);
	ImGui::TextUnformatted(desc);
	ImGui::SetCursorPosY(temp);
}