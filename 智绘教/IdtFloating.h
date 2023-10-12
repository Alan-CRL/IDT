#pragma once
#include "IdtMain.h"

#include "IdtDraw.h"
#include "IdtImage.h"
#include "IdtMagnification.h"
#include "IdtPlug-in.h"
#include "IdtText.h"
#include "IdtUpdate.h"
#include "IdtWindow.h"

//���ڿ��Ƽ�
struct floating_windowsStruct
{
	int x, y;
	int height, width;
	int translucent;
};
extern floating_windowsStruct floating_windows;

extern IMAGE floating_icon[20], sign;

extern double state;
extern double target_status;

extern bool RestoreSketchpad, empty_drawpad;
extern bool smallcard_refresh;

//�ö����򴰿�
void TopWindow();

//UI �ؼ�

extern int BackgroundColorMode;
struct UIControlStruct
{
	float v, s, e;
};
extern map<wstring, UIControlStruct> UIControl, UIControlTarget;
map<wstring, UIControlStruct>& map<wstring, UIControlStruct>::operator=(const map<wstring, UIControlStruct>& m);

struct UIControlColorStruct
{
	COLORREF v;
	float s, e;
};
extern map<wstring, UIControlColorStruct> UIControlColor, UIControlColorTarget;
map<wstring, UIControlColorStruct>& map<wstring, UIControlColorStruct>::operator=(const map<wstring, UIControlColorStruct>& m);

//ѡɫ��
Color ColorFromHSV(float hue, float saturation, float value);
IMAGE DrawHSVWheel(int r, int z = 0, int angle = 0);

//������Ļ
void DrawScreen();
void SeekBar(ExMessage m);

void MouseInteraction();

int floating_main();