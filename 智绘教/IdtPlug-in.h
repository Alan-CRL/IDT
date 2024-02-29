#pragma once
#include "IdtMain.h"

extern IMAGE ppt_icon[5];
extern bool SeewoCamera; //ϣ����Ƶչ̨�Ƿ���

//PPT����

struct ppt_infoStruct
{
	int currentSlides;
	int totalSlides;
};
extern ppt_infoStruct ppt_info;

struct ppt_imgStruct
{
	bool is_save;
	map<int, bool> is_saved;
	map<int, IMAGE> image;
};
extern ppt_imgStruct ppt_img;

struct ppt_info_stayStruct
{
	int CurrentPage, TotalPage;
};
extern ppt_info_stayStruct ppt_info_stay;

extern shared_mutex PPTManipulatedSm;
extern std::chrono::high_resolution_clock::time_point PPTManipulated;

wstring LinkTest();
wstring IsPptDependencyLoaded();
HWND GetPptShow();
wstring GetPptTitle();
bool EndPptShow();

int NextPptSlides(int check);
int PreviousPptSlides();

//ppt �ؼ�
void DrawControlWindow();
void ControlManipulation();

//��ȡ��ǰҳ���
int GetCurrentPage();
//��ȡ��ҳ��
int GetTotalPage();
void ppt_state();

//��������
//�ر�AIClass��ϣ�ְװ�5����
HWND FindWindowByStrings(const std::wstring& className, const std::wstring& windowTitle, const std::wstring& style, int width = 0, int height = 0);
void black_block();