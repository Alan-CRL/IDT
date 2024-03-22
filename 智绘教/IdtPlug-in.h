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

extern shared_mutex PPTManipulatedSm;
extern chrono::high_resolution_clock::time_point PPTManipulated;

struct PPTUIControlStruct
{
	float v, s, e;
};
struct PPTUIControlColorStruct
{
	COLORREF v;
	float s, e;
};
extern map<wstring, PPTUIControlStruct> PPTUIControl, PPTUIControlTarget;
map<wstring, PPTUIControlStruct>& map<wstring, PPTUIControlStruct>::operator=(const map<wstring, PPTUIControlStruct>& m);
extern map<wstring, PPTUIControlColorStruct> PPTUIControlColor, PPTUIControlColorTarget;
map<wstring, PPTUIControlColorStruct>& map<wstring, PPTUIControlColorStruct>::operator=(const map<wstring, PPTUIControlColorStruct>& m);
extern map<wstring, wstring> PPTUIControlString, PPTUIControlStringTarget;

struct PptImgStruct
{
	bool IsSave;
	map<int, bool> IsSaved;
	map<int, IMAGE> Image;
};
extern PptImgStruct PptImg;
struct PptInfoStateStruct
{
	int CurrentPage, TotalPage;
};
extern PptInfoStateStruct PptInfoStateBuffer;
extern PptInfoStateStruct PptInfoState;
extern bool PptWindowBackgroundUiChange;
extern bool SeewoCameraIsOpen;

wstring LinkTest();
wstring IsPptDependencyLoaded();
bool EndPptShow();
int NextPptSlides(int check);
int PreviousPptSlides();
void GetPptState();
void PPTLinkageMain();
void BlackBlock();