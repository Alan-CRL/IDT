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

// All function and variable descriptions should be in the corresponding cpp file.
// ���еĺ����ͱ���˵��Ӧ���ڶ�Ӧ�� cpp �ļ��С�

extern float PPTUIScale;

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
wstring GetPptComVersion();

void NextPptSlides(int check);
void PreviousPptSlides();
bool EndPptShow();

void GetPptState();
void PPTLinkageMain();

// --------------------------------------------------
// ���

// DesktopDrawpadBlocker ���
void StartDesktopDrawpadBlocker();