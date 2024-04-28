#pragma once
#include "IdtMain.h"

#include "IdtWindow.h"
#include <magnification.h>
#pragma comment(lib, "magnification.lib")

extern IMAGE MagnificationBackground;
extern HWND hwndHost, hwndMag;
extern int magnificationWindowReady;

extern shared_mutex MagnificationBackgroundSm;
extern RECT magWindowRect;
extern RECT hostWindowRect;

extern HINSTANCE hInst;
BOOL MagImageScaling(HWND hwnd, void* srcdata, MAGIMAGEHEADER srcheader, void* destdata, MAGIMAGEHEADER destheader, RECT unclipped, RECT clipped, HRGN dirty);
void UpdateMagWindow();

LRESULT CALLBACK MagnifierWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
ATOM RegisterHostWindowClass(HINSTANCE hInstance);
BOOL SetupMagnifier(HINSTANCE hinst);

extern bool RequestUpdateMagWindow;

void MagnifierThread();