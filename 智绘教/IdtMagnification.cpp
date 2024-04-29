#include "IdtMagnification.h"
#include <winuser.h>

#include <d3d9.h>
#pragma comment(lib, "d3d9")

HWND hwndHost, hwndMag;
IMAGE MagnificationBackground = CreateImageColor(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), RGBA(255, 255, 255, 255), false);
int magnificationWindowReady;

shared_mutex MagnificationBackgroundSm;
RECT magWindowRect;
RECT hostWindowRect;

HINSTANCE hInst;
BOOL MagImageScaling(HWND hwnd, void* srcdata, MAGIMAGEHEADER srcheader, void* destdata, MAGIMAGEHEADER destheader, RECT unclipped, RECT clipped, HRGN dirty)
{
	int ImageWidth = srcheader.width;
	int ImageHeight = srcheader.height;

	IMAGE SrcImage(ImageWidth, ImageHeight);
	DWORD* SrcBuffer = GetImageBuffer(&SrcImage);
	memcpy(SrcBuffer, (LPBYTE)srcdata + srcheader.offset, ImageWidth * ImageHeight * 4);

	std::unique_lock<std::shared_mutex> LockMagnificationBackgroundSm(MagnificationBackgroundSm);
	MagnificationBackground = SrcImage;
	LockMagnificationBackgroundSm.unlock();

	return 1;
}
void UpdateMagWindow()
{
	RECT sourceRect = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };

	MagSetWindowSource(hwndMag, sourceRect);
	InvalidateRect(hwndMag, NULL, TRUE);
}

LRESULT CALLBACK MagnifierWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}
ATOM RegisterHostWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MagnifierWindowWndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(1 + COLOR_BTNFACE);
	wcex.lpszClassName = TEXT("MagnifierWindow");

	return RegisterClassEx(&wcex);
}
BOOL SetupMagnifier(HINSTANCE hinst)
{
	hostWindowRect.top = 0;
	hostWindowRect.bottom = GetSystemMetrics(SM_CYSCREEN);
	hostWindowRect.left = 0;
	hostWindowRect.right = GetSystemMetrics(SM_CXSCREEN);

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ע��Ŵ�API����");
	RegisterHostWindowClass(hinst);
	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ע��Ŵ�API�������");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API��������");
	hwndHost = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, TEXT("MagnifierWindow"), TEXT("Idt Screen Magnifier"), WS_SIZEBOX | WS_SYSMENU | WS_CLIPCHILDREN | WS_CAPTION | WS_MAXIMIZEBOX, 0, 0, hostWindowRect.right, hostWindowRect.bottom, NULL, NULL, hInst, NULL);
	if (!hwndHost)
	{
		IDTLogger->error("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API��������ʧ��" + to_string(GetLastError()));
		return FALSE;
	}
	else IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API�����������");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API����");
	hwndMag = CreateWindow(WC_MAGNIFIER, TEXT("MagnifierWindow"), WS_CHILD | MS_CLIPAROUNDCURSOR | WS_VISIBLE, magWindowRect.left, magWindowRect.top, magWindowRect.right, magWindowRect.bottom, hwndHost, NULL, hInst, NULL);
	if (!hwndMag)
	{
		IDTLogger->error("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API����ʧ��" + to_string(GetLastError()));
		return FALSE;
	}
	else IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] �����Ŵ�API�������");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�API������ʽ");
	SetWindowLong(hwndMag, GWL_STYLE, GetWindowLong(hwndMag, GWL_STYLE) & ~WS_CAPTION);//���ر�����
	SetWindowLong(hwndMag, GWL_EXSTYLE, WS_EX_TOOLWINDOW);//����������
	SetWindowPos(hwndMag, NULL, hostWindowRect.left, hostWindowRect.top, hostWindowRect.right - hostWindowRect.left, hostWindowRect.bottom - hostWindowRect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_DRAWFRAME);
	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�API�������");

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
			{  1.0f,  1.0f,  1.0f,  0.0f,  1.0f }
		} };

		ret = MagSetColorEffect(hwndMag, NULL);

		IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�APIת���������");
	}
	else IDTLogger->error("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�APIת������ʧ��");

	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�API�ص�����");
	MagSetImageScalingCallback(hwndMag, (MagImageScalingCallback)MagImageScaling);
	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���÷Ŵ�API�ص��������");

	return ret;
}

bool RequestUpdateMagWindow;
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
	ShowWindow(hwndHost, SW_HIDE);
	UpdateWindow(hwndHost);
	IDTLogger->info("[�Ŵ�API�߳�][SetupMagnifier] ���·Ŵ�API�������");

	IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] �ȴ���͸���ڴ���");
	while (!off_signal)
	{
		if (magnificationWindowReady >= 4)
		{
			std::vector<HWND> hwndList;
			hwndList.emplace_back(floating_window);
			hwndList.emplace_back(drawpad_window);
			hwndList.emplace_back(ppt_window);
			hwndList.emplace_back(setting_window);

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
			else IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] ���ô�͸�����б����");

			magnificationWindowReady = -1;

			break;
		}

		Sleep(500);
	}
	IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] �ȴ���͸���ڴ������");

	RequestUpdateMagWindow = true;
	while (!off_signal)
	{
		while (!RequestUpdateMagWindow && !off_signal) Sleep(100);
		if (off_signal) break;

		{
			IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] ����APIͼ��");
			UpdateMagWindow();
			IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] ����APIͼ�����");

			RequestUpdateMagWindow = false;
		}
	}

	IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] �����ʼ��MagUninitialize");
	MagUninitialize();
	IDTLogger->info("[�Ŵ�API�߳�][MagnifierThread] �����ʼ��MagUninitialize���");
}