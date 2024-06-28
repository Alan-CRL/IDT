/*
 * @file		IdtPlug-in.h
 * @brief		IDT plugin linkage | �ǻ�̲������
 * @note		PPT linkage components and other plugins | PPT������������������
 *
 * @author		AlanCRL
 * @qq			2685549821
 * @email		alan-crl@foxmail.com
*/

#pragma once
#include "IdtMain.h"

#include "IdtD2DPreparation.h"

// All function and variable descriptions should be in the corresponding cpp file.
// ���еĺ����ͱ���˵��Ӧ���ڶ�Ӧ�� cpp �ļ��С�

// --------------------------------------------------
// PPT �������

// -------------------------
// UI ����

extern bool PptUiAnimationEnable;

class PptUiWidgetValue
{
public:
	PptUiWidgetValue()
	{
		replace = false;
	}
	PptUiWidgetValue(float sTarget, float eTarget, bool replaceTarget = true)
	{
		v = 0, s = sTarget, e = eTarget;
		replace = replaceTarget;
	}

public:
	float v, s, e;
	bool replace;
};
class PptUiWidgetColor
{
public:
	PptUiWidgetColor()
	{
		replace = false;
	}
	PptUiWidgetColor(COLORREF vTarget, float sTarget, float eTarget, bool replaceTarget = true)
	{
		v = vTarget, s = sTarget, e = eTarget;
		replace = replaceTarget;
	}

public:
	COLORREF v;
	float s, e;
	bool replace;
};

enum PptUiRegionWidgetID
{
	LeftSide,
	MiddleSide,
	RightSide
};
enum PptUiRoundRectWidgetID
{
	//...
	BottomSide_LeftPageWidget,
	BottomSide_LeftPageWidget_PreviousPage,
	BottomSide_LeftPageWidget_NextPage,
	BottomSide_RightPageWidget,
	BottomSide_RightPageWidget_PreviousPage,
	BottomSide_RightPageWidget_NextPage,

	BottomSide_MiddleTabSlideWidget,
	BottomSide_MiddleTabSlideWidget_EndShow,
	BottomSide_MiddleTabDrawpadWidget
};
enum PptUiImageWidgetID
{
	BottomSide_LeftPreviousPage,
	BottomSide_LeftNextPage,
	BottomSide_MiddleEndShow,
	BottomSide_RightPreviousPage,
	BottomSide_RightNextPage
};
enum PptUiWordsWidgetID
{
	BottomSide_LeftPageNum,
	BottomSide_RightPageNum
};

class PptUiRegionWidgetClass
{
public:
	PptUiWidgetValue X;
	PptUiWidgetValue Y;
	PptUiWidgetValue Width;
	PptUiWidgetValue Height;
};
class PptUiRoundRectWidgetClass
{
public:
	PptUiWidgetValue X;
	PptUiWidgetValue Y;
	PptUiWidgetValue Width;
	PptUiWidgetValue Height;
	PptUiWidgetValue EllipseWidth;
	PptUiWidgetValue EllipseHeight;

	PptUiWidgetValue FrameThickness;
	PptUiWidgetColor FrameColor;
	PptUiWidgetColor FillColor;
};
class PptUiImageWidgetClass
{
public:
	PptUiImageWidgetClass()
	{
		Img = NULL;
	}

public:
	PptUiWidgetValue X;
	PptUiWidgetValue Y;
	PptUiWidgetValue Width;
	PptUiWidgetValue Height;
	PptUiWidgetValue Transparency;
	ID2D1Bitmap* Img;
};
class PptUiWordsWidgetClass
{
public:
	PptUiWordsWidgetClass()
	{
		WordsContent = L"";
	}

public:
	PptUiWidgetValue Left;
	PptUiWidgetValue Top;
	PptUiWidgetValue Right;
	PptUiWidgetValue Bottom;

	PptUiWidgetValue WordsHeight;
	PptUiWidgetColor WordsColor;

	wstring WordsContent;
};

extern PptUiRoundRectWidgetClass pptUiRoundRectWidget[9], pptUiRoundRectWidgetTarget[9];
extern PptUiImageWidgetClass pptUiImageWidget[5], pptUiImageWidgetTarget[5];
extern PptUiWordsWidgetClass pptUiWordsWidget[2], pptUiWordsWidgetTarget[2];

// -------------------------
// UI ����

enum PptUiWidgetScaleID
{
	BottomSide_LeftWidget,
	BottomSide_RightWidget,
	BottomSide_MiddleWidget
};
extern float PptUiWidgetScale[3];

// -------------------------
// ppt ��Ϣ

struct PptImgStruct
{
	bool IsSave;
	map<int, bool> IsSaved;
	map<int, IMAGE> Image;
};
extern PptImgStruct PptImg;
struct PptInfoStateStruct
{
	long CurrentPage, TotalPage;
};
extern PptInfoStateStruct PptInfoStateBuffer;
extern PptInfoStateStruct PptInfoState;
extern bool PptWindowBackgroundUiChange;

extern wstring pptComVersion;

// -------------------------
// Ppt ״̬

void NextPptSlides(int check);
void PreviousPptSlides();
bool EndPptShow();

void PPTLinkageMain();

// --------------------------------------------------
// �������

void StartDesktopDrawpadBlocker();