#include "IdtMagnification.h"
#include <winuser.h>

#include "IdtWindow.h"
#include "IdtDraw.h"
#include "IdtDisplayManagement.h"

#include <d3d9.h>
#pragma comment(lib, "d3d9")

// IDT �����ܵ�����������Ҫ Windows 8.1

HWND hwndHost, hwndMag;
HINSTANCE hInst;
IMAGE MagnificationBackground;

bool magnificationReady;

shared_mutex MagnificationBackgroundSm;
RECT hostWindowRect;
int MagTransparency;

void UpdateMagWindow()
{
	RECT sourceRect = { 0, 0, GetSystemMetrics(SM_CXSCREEN) - 1, GetSystemMetrics(SM_CYSCREEN) - 1 };
	MagSetWindowSource(hwndMag, sourceRect);
	InvalidateRect(hwndMag, NULL, TRUE);

	/*
	{
		std::unique_lock<std::shared_mutex> LockMagnificationBackgroundSm(MagnificationBackgroundSm);

		if (MagnificationBackground.getwidth() != GetSystemMetrics(SM_CXSCREEN) || MagnificationBackground.getheight() != GetSystemMetrics(SM_CYSCREEN))
			MagnificationBackground.Resize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

		PrintWindow(hwndMag, GetImageHDC(&MagnificationBackground), PW_RENDERFULLCONTENT);
		hiex::RemoveImageTransparency(&MagnificationBackground);

		LockMagnificationBackgroundSm.unlock();
	}*/
}

LRESULT CALLBACK MagnifierHostWindowWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
ATOM RegisterHostWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MagnifierHostWindowWndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(1 + COLOR_BTNFACE);
	wcex.lpszClassName = TEXT("IdtMagnifierWindowHost");

	return RegisterClassEx(&wcex);
}
BOOL SetupMagnifier(HINSTANCE hinst)
{
	// Set bounds of host window according to screen size.
	hostWindowRect.top = 0;
	hostWindowRect.bottom = GetSystemMetrics(SM_CYSCREEN) / 2;  // top quarter of screen
	hostWindowRect.left = 0;
	hostWindowRect.right = GetSystemMetrics(SM_CXSCREEN) / 2;

	// Create the host window.
	RegisterHostWindowClass(hinst);
	hwndHost = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED,
		TEXT("IdtMagnifierWindowHost"), L"IdtScreenMagnifierHost",
		WS_SIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN | WS_CAPTION | WS_MAXIMIZEBOX,
		200, 200, hostWindowRect.right, hostWindowRect.bottom, NULL, NULL, hInst, NULL);
	if (!hwndHost) Testi(1);

	// Make the window opaque.
	SetLayeredWindowAttributes(hwndHost, 0, 255, LWA_ALPHA);

	// Create a magnifier control that fills the client area.
	hwndMag = CreateWindow(WC_MAGNIFIER, TEXT("IdtScreenMagnifierMag"),
		WS_CHILD | MS_SHOWMAGNIFIEDCURSOR | WS_VISIBLE,
		hostWindowRect.left, hostWindowRect.top, hostWindowRect.right, hostWindowRect.bottom, hwndHost, NULL, hInst, NULL);
	if (!hwndMag) Testi(2);

	// ����Ϊ��ʼ��ʵ�鲿��
	Testi(123);

	hostWindowRect.left = 0;
	hostWindowRect.top = 0;
	hostWindowRect.right = GetSystemMetrics(SM_CXSCREEN);
	if (enableAppBarAutoHide) hostWindowRect.bottom = GetSystemMetrics(SM_CYSCREEN) - 1;
	else hostWindowRect.bottom = GetSystemMetrics(SM_CYSCREEN);

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ע��Ŵ�API��������");
	if (!RegisterHostWindowClass(hinst)) IDTLogger->warn("[�Ŵ�API�߳�][SetupMagnifier] ע��Ŵ�API��������ʧ��");
	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ע��Ŵ�API�����������");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API��������");
	hwndHost = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, TEXT("IdtMagnifierWindowHost"), TEXT("IdtScreenMagnifierHost"), WS_SIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN | WS_CAPTION | WS_MAXIMIZEBOX, 0, 0, hostWindowRect.right, hostWindowRect.bottom, NULL, NULL, hInst, NULL);
	if (!hwndHost)
	{
		IDTLogger->error("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API��������ʧ��" + to_string(GetLastError()));
		return FALSE;
	}
	else IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API�����������");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�API��������͸����");
	SetLayeredWindowAttributes(hwndHost, 0, 0, LWA_ALPHA);
	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�API��������͸�������");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API����");
	hwndMag = CreateWindow(WC_MAGNIFIER, TEXT("IdtScreenMagnifierMag"),
		WS_CHILD | MS_CLIPAROUNDCURSOR | WS_VISIBLE,
		hostWindowRect.left, hostWindowRect.top, hostWindowRect.right, hostWindowRect.bottom, hwndHost, NULL, hInst, NULL);
	if (!hwndMag)
	{
		IDTLogger->error("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API����ʧ��" + to_string(GetLastError()));
		return FALSE;
	}
	else IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API�������");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�API������ʽ");
	SetWindowLong(hwndHost, GWL_STYLE, GetWindowLong(hwndHost, GWL_STYLE) & ~WS_CAPTION); // ���ر�����
	SetWindowLong(hwndHost, GWL_STYLE, GetWindowLong(hwndHost, GWL_STYLE) & ~WS_THICKFRAME); // ��ֹ��������
	SetWindowPos(hwndHost, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED);

	SetWindowLong(hwndHost, GWL_EXSTYLE, (GetWindowLong(hwndHost, GWL_EXSTYLE) | WS_EX_TOOLWINDOW) & ~WS_EX_APPWINDOW); // ����������ͼ��
	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�API������ʽ���");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�API����");
	MAGTRANSFORM matrix;
	memset(&matrix, 0, sizeof(matrix));
	matrix.v[0][0] = 1.0f;
	matrix.v[1][1] = 1.0f;
	matrix.v[2][2] = 1.0f;
	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�API�������");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�APIת������");
	BOOL ret = MagSetWindowTransform(hwndMag, &matrix);
	if (ret)
	{
		MAGCOLOREFFECT magEffectInvert =
		{ {
			{  1.0f,  0.0f,  0.0f,  0.0f,  0.0f },
			{  0.0f,  1.0f,  0.0f,  0.0f,  0.0f },
			{  0.0f,  0.0f,  1.0f,  0.0f,  0.0f },
			{  0.0f,  0.0f,  0.0f,  1.0f,  0.0f },
			{  0.0f,  0.0f,  0.0f,  0.0f,  1.0f }
		} };

		ret = MagSetColorEffect(hwndMag, &magEffectInvert);

		IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�APIת���������");
	}
	else IDTLogger->error("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�APIת������ʧ��");

	return ret;
}

int RequestUpdateMagWindow;
/*
* RequestUpdateMagWindow ���� IDT �Ŵ���Ϊ
* 0 ���ش���
* 1 ��ʾ���ڲ�����
* 2 ��ʾ���ڲ�ʵʱ
*/

void MagnifierThread()
{
	IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] ��ʼ��MagInitialize");
	if (!MagInitialize())
	{
		IDTLogger->error("[�Ŵ�API�߳�][MagnifierThread] ��ʼ��MagInitializeʧ��");
		return;
	}
	else IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] ��ʼ��MagInitialize���");

	IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] �����Ŵ�API");
	if (!SetupMagnifier(GetModuleHandle(0)))
	{
		IDTLogger->error("[�Ŵ�API�߳�][MagnifierThread] �����Ŵ�APIʧ��");
		return;
	}
	else IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] �����Ŵ�API���");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���·Ŵ�API����");
	ShowWindow(hwndHost, SW_SHOW);
	UpdateWindow(hwndHost);
	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���·Ŵ�API�������");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API�������");
	thread(MagnifierUpdate).detach();
	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API������̳ɹ�");

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
void MagnifierUpdate()
{
	IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] �ȴ���͸���ڴ���");
	while (!offSignal)
	{
		if (IdtWindowsIsVisible.allCompleted)
		{
			std::vector<HWND> hwndList;
			hwndList.emplace_back(floating_window);
			hwndList.emplace_back(ppt_window);
			hwndList.emplace_back(drawpad_window);
			hwndList.emplace_back(freeze_window);
			//hwndList.emplace_back(setting_window);

			IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] ���ô�͸�����б�");

			if (MagSetWindowFilterList(hwndMag, MW_FILTERMODE_EXCLUDE, hwndList.size(), hwndList.data()) == FALSE)
			{
				IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
				if (pD3D != nullptr)
				{
					D3DCAPS9 caps;
					HRESULT hr = pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
					if (SUCCEEDED(hr))
					{
						if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) IDTLogger->error("[�Ŵ�API�߳�][MagnifierThread] ���ô�͸�����б�ʧ�ܣ��豸֧�� WDDM 1.0 �汾��ԭ��δ֪��");
						else IDTLogger->error("[�Ŵ�API�߳�][MagnifierThread] ���ô�͸�����б�ʧ�ܣ��豸��֧�� WDDM 1.0 �汾��");
					}
					else IDTLogger->error("[�Ŵ�API�߳�][MagnifierThread] ���ô�͸�����б�ʧ�ܣ��޷� GetDeviceCaps ����ѯ�Ƿ�֧�� WDDM��");

					pD3D->Release();
				}
				else IDTLogger->error("[�Ŵ�API�߳�][MagnifierThread] ���ô�͸�����б�ʧ�ܣ��޷���ʼ�� IDirect3D9 ����ѯ�Ƿ�֧�� WDDM��");
			}
			else
			{
				IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] ���ô�͸�����б����");
				magnificationReady = true;
			}

			break;
		}

		this_thread::sleep_for(chrono::milliseconds(500));
	}
	IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] �ȴ���͸���ڴ������");

	while (!offSignal)
	{
		if (RequestUpdateMagWindow == 1)
		{
			if (MagTransparency == 0)
			{
				UpdateMagWindow();

				SetLayeredWindowAttributes(hwndHost, 0, 255, LWA_ALPHA);
				MagTransparency = 255;
			}

			for (int i = 0; RequestUpdateMagWindow == 1; i++, i %= 10)
			{
				if (!i) SetWindowPos(hwndHost, freeze_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

				this_thread::sleep_for(chrono::milliseconds(100));
			}
		}
		else if (RequestUpdateMagWindow == 0)
		{
			if (MagTransparency == 255)
			{
				SetLayeredWindowAttributes(hwndHost, 0, 0, LWA_ALPHA);
				MagTransparency = 0;
			}

			while (RequestUpdateMagWindow == 0) this_thread::sleep_for(chrono::milliseconds(50));
		}
	}

	IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] �����ʼ��MagUninitialize");
	MagUninitialize();
	IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] �����ʼ��MagUninitialize���");
}