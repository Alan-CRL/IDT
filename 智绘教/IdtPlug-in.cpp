/*
 * @file		IdtPlug-in.cpp
 * @brief		IDT plugin linkage | �ǻ�̲������
 * @note		PPT linkage components and other plugins | PPT������������������
 *
 * @author		AlanCRL
 * @qq			2685549821
 * @email		alan-crl@foxmail.com
*/

// INFO: This source file will take the lead in refactoring the code logic and optimizing the reading experience.
// ��ʾ�����Դ�ļ��������ع������߼������Ż��Ķ����顣

// �������⣺һЩע�ͺ���� '*' �ţ����ǵĽ��������档
//
// *1
//
// PptInfoStateBuffer ������ PptInfoState �����Ļ��壬�� DrawpadDrawing ����������� PPT �Ļ���󣬻�������е�ֵ�Ż��Ϊ�� PptInfoState һ�¡�
// һЩ������ȡ PptInfoStateBuffer ��ֵ������Ҫ�ȵ� PPT �����ʼ����Ϻ�Ż������ı䣬����������Ӧ��

#include "IdtPlug-in.h"

#include "IdtConfiguration.h"
#include "IdtDisplayManagement.h"
#include "IdtDraw.h"
#include "IdtDrawpad.h"
#include "IdtFloating.h"
#include "IdtMagnification.h"
#include "IdtRts.h"
#include "IdtText.h"
#include "IdtWindow.h"
#include "IdtOther.h"
#include "IdtHistoricalDrawpad.h"
#include "IdtImage.h"
#include "IdtState.h"

// --------------------------------------------------
// PPT �������

#import "PptCOM.tlb" // C# ��� PptCOM ��Ŀ�� (PptCOM. cs)
using namespace PptCOM;
IPptCOMServerPtr PptCOMPto;

bool PptUiAnimationEnable;

PptUiRoundRectWidgetClass pptUiRoundRectWidget[9], pptUiRoundRectWidgetTarget[9];
PptUiImageWidgetClass pptUiImageWidget[5], pptUiImageWidgetTarget[5];
PptUiWordsWidgetClass pptUiWordsWidget[2], pptUiWordsWidgetTarget[2];

float PptUiWidgetScale[3];
ID2D1Bitmap* pptIconBitmap[5];

void PptUiWidgetValueTransformation(float* v, float tv, float s, float e, int num = 1)
{
	if (!s || !e) return;

	while (num--)
	{
		if (abs(*v - tv) <= e) *v = tv;
		else if (*v < tv) *v = float(*v + max(e, (tv - *v) / s));
		else *v = float(*v + min(-e, (tv - *v) / s));
	}
}
void PptUiWidgetColorTransformation(COLORREF* v, COLORREF tv, float s, float e, int num = 1)
{
	if (!s || !e) return;

	while (num--)
	{
		float r1 = GetRValue(*v);
		float g1 = GetGValue(*v);
		float b1 = GetBValue(*v);
		float a1 = float(((*v) >> 24) & 0xff);

		float r2 = GetRValue(tv);
		float g2 = GetGValue(tv);
		float b2 = GetBValue(tv);
		float a2 = float((tv >> 24) & 0xff);

		if (abs(r1 - r2) <= e) r1 = r2;
		else if (r1 < r2) r1 = r1 + max(e, (r2 - r1) / s);
		else if (r1 > r2) r1 = r1 + min(-e, (r2 - r1) / s);

		if (abs(g1 - g2) <= e) g1 = g2;
		else if (g1 < g2) g1 = g1 + max(e, (g2 - g1) / s);
		else if (g1 > g2) g1 = g1 + min(-e, (g2 - g1) / s);

		if (abs(b1 - b2) <= e) b1 = b2;
		else if (b1 < b2) b1 = b1 + max(e, (b2 - b1) / s);
		else if (b1 > b2) b1 = b1 + min(-e, (b2 - b1) / s);

		if (abs(a1 - a2) <= e) a1 = a2;
		else if (a1 < a2) a1 = a1 + max(e, (a2 - a1) / s);
		else if (a1 > a2) a1 = a1 + min(-e, (a2 - a1) / s);

		*v = RGBA(max(0, min(255, (int)r1)), max(0, min(255, (int)g1)), max(0, min(255, (int)b1)), max(0, min(255, (int)a1)));
	}
}

bool PptUiIsInEllipse(float x, float y, float x1, float y1, float w, float h)
{
	float x0 = x1 + w / 2.0f;
	float y0 = y1 + h / 2.0f;

	float value = ((x - x0) * (x - x0)) / ((w * w) / 4.0f) + ((y - y0) * (y - y0)) / ((h * h) / 4.0f);

	return value <= 1.0f;
}
bool PptUiIsInRoundRect(float x, float y, PptUiRoundRectWidgetClass pptUiRoundRectWidget)
{
	if (pptUiRoundRectWidget.X.v + pptUiRoundRectWidget.EllipseWidth.v / 2.0f <= x && x <= pptUiRoundRectWidget.X.v + pptUiRoundRectWidget.Width.v - pptUiRoundRectWidget.EllipseWidth.v / 2.0f &&
		pptUiRoundRectWidget.Y.v <= y && y <= pptUiRoundRectWidget.Y.v + pptUiRoundRectWidget.Height.v)
		return true;
	if (pptUiRoundRectWidget.X.v <= x && x <= pptUiRoundRectWidget.X.v + pptUiRoundRectWidget.Width.v &&
		pptUiRoundRectWidget.Y.v + pptUiRoundRectWidget.EllipseHeight.v / 2.0f <= y && y <= pptUiRoundRectWidget.Y.v + pptUiRoundRectWidget.Height.v - pptUiRoundRectWidget.EllipseHeight.v / 2.0f)
		return true;

	if (PptUiIsInEllipse(x, y, pptUiRoundRectWidget.X.v, pptUiRoundRectWidget.Y.v, pptUiRoundRectWidget.EllipseWidth.v, pptUiRoundRectWidget.EllipseHeight.v))
		return true;
	if (PptUiIsInEllipse(x, y, pptUiRoundRectWidget.X.v + pptUiRoundRectWidget.Width.v - pptUiRoundRectWidget.EllipseWidth.v, pptUiRoundRectWidget.Y.v, pptUiRoundRectWidget.EllipseWidth.v, pptUiRoundRectWidget.EllipseHeight.v))
		return true;
	if (PptUiIsInEllipse(x, y, pptUiRoundRectWidget.X.v, pptUiRoundRectWidget.Y.v + pptUiRoundRectWidget.Height.v - pptUiRoundRectWidget.EllipseHeight.v, pptUiRoundRectWidget.EllipseWidth.v, pptUiRoundRectWidget.EllipseHeight.v))
		return true;
	if (PptUiIsInEllipse(x, y, pptUiRoundRectWidget.X.v + pptUiRoundRectWidget.Width.v - pptUiRoundRectWidget.EllipseWidth.v, pptUiRoundRectWidget.Y.v + pptUiRoundRectWidget.Height.v - pptUiRoundRectWidget.EllipseHeight.v, pptUiRoundRectWidget.EllipseWidth.v, pptUiRoundRectWidget.EllipseHeight.v))
		return true;

	return false;
}

PptImgStruct PptImg = { false }; // ��洢�õ�Ƭ��ӳʱ������ͼ�����ݡ�
PptInfoStateStruct PptInfoState = { -1, -1 }; // ��洢�õ�Ƭ��ӳ�����ǰ��״̬��First �����ܻõ�Ƭҳ����Second ����ǰ�õ�Ƭ��š�
PptInfoStateStruct PptInfoStateBuffer = { -1, -1 }; // PptInfoState �Ļ��������*1

IMAGE PptIcon[5]; // PPT �ؼ��İ���ͼ��
IMAGE PptWindowBackground; // PPT ���ڱ�������

bool PptUiChangeSignal;
bool PptUiAllReplaceSignal;

wstring pptComVersion;
wstring GetPptComVersion()
{
	wstring ret = L"Error: COM��(.dll) �����ڣ��ҷ������ش��󣬷���ֵ������";

	if (_waccess((StringToWstring(globalPath) + L"PptCOM.dll").c_str(), 4) == 0)
	{
		try
		{
			ret = BstrToWstring(PptCOMPto->GetVersion());
			if (!regex_match(ret, wregex(L"\\d{8}[a-z]"))) ret = L"Error: " + ret;
		}
		catch (_com_error& err)
		{
			ret = L"Error: COM��(.dll) ���ڣ�COM�ɹ���ʼ������C++��COM�ӿ��쳣��" + wstring(err.ErrorMessage());
		}
	}
	else
	{
		wchar_t absolutePath[_MAX_PATH];

		if (_wfullpath(absolutePath, L"PptCOM.dll", _MAX_PATH) != NULL)
		{
			ret = L"Error: COM��(.dll) �����ڣ�Ԥ�ڵ���Ŀ¼Ϊ��\"" + StringToWstring(globalPath) + L"PptCOM.dll\"";
		}
		else ret = L"Error: COM��(.dll) �����ڣ�Ԥ�ڵ���Ŀ¼����ʧ��";
	}

	return ret;
}

wstring GetPptTitle()
{
	wstring ret = L"";

	try
	{
		ret = BstrToWstring(PptCOMPto->slideNameIndex());

		return ret;
	}
	catch (_com_error)
	{
	}

	return ret;
}
HWND GetPptShow()
{
	HWND hWnd = NULL;

	try
	{
		_variant_t result = PptCOMPto->GetPptHwnd();
		hWnd = (HWND)result.llVal;

		return hWnd;
	}
	catch (_com_error)
	{
	}

	return NULL;
}
void GetPptState()
{
	threadStatus[L"GetPptState"] = true;

	// ��ʼ��
	{
		bool rel = false;

		try
		{
			_com_util::CheckError(PptCOMPto.CreateInstance(_uuidof(PptCOMServer)));
			rel = PptCOMPto->Initialization(&PptInfoState.TotalPage, &PptInfoState.CurrentPage);
		}
		catch (_com_error)
		{
		}

		pptComVersion = GetPptComVersion();
	}

	while (!offSignal)
	{
		int tmp = -1;
		try
		{
			tmp = PptCOMPto->IsPptOpen();
		}
		catch (_com_error)
		{
		}

		if (tmp <= 0)
		{
			for (int i = 0; i <= 30 && !offSignal; i++)
				this_thread::sleep_for(chrono::milliseconds(100));
		}
	}

	threadStatus[L"GetPptState"] = false;
}

void NextPptSlides(int check)
{
	try
	{
		PptCOMPto->NextSlideShow(check);
	}
	catch (_com_error)
	{
	}
	return;
}
void PreviousPptSlides()
{
	try
	{
		PptCOMPto->PreviousSlideShow();
	}
	catch (_com_error)
	{
	}
	return;
}
bool EndPptShow()
{
	try
	{
		PptCOMPto->EndSlideShow();

		return true;
	}
	catch (_com_error)
	{
	}

	return false;
}

void PptUI(MainMonitorStruct* PPTMainMonitorStruct)
{
	threadStatus[L"PptUI"] = true;

	// ��������Ϣ��ʼ��
	MainMonitorStruct PPTMainMonitor;
	shared_lock<shared_mutex> DisplaysInfoLock2(DisplaysInfoSm);
	PPTMainMonitor = *PPTMainMonitorStruct;
	DisplaysInfoLock2.unlock();

	// ���� UI ����
	PptUiAnimationEnable = true;

	// UI ���ų�ʼ��
	{
		PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget] = 1.0f;
		PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget] = 1.0f;
		PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget] = 1.0f;
	}

	// UI ��ʼ��
	{
		// ���ؼ�
		{
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].X = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Y = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Width = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Height = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].EllipseWidth = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].EllipseHeight = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].FrameThickness = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].FillColor = PptUiWidgetColor(RGBA(225, 225, 225, 0), 25, 1);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].FrameColor = PptUiWidgetColor(RGBA(200, 200, 200, 0), 25, 1);

			{
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].X = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Y = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Width = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Height = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].EllipseWidth = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].EllipseHeight = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FrameThickness = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor = PptUiWidgetColor(RGBA(250, 250, 250, 0), 10, 1);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FrameColor = PptUiWidgetColor(RGBA(200, 200, 200, 0), 25, 1);

				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].X = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Y = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Width = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Height = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Transparency = PptUiWidgetValue(25, 1);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Img = pptIconBitmap[1];
			}
			{
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Left = PptUiWidgetValue(15, 0.1f);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Top = PptUiWidgetValue(15, 0.1f);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Right = PptUiWidgetValue(15, 0.1f);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Bottom = PptUiWidgetValue(15, 0.1f);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsHeight = PptUiWidgetValue(15, 0.1f);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsColor = PptUiWidgetColor(RGBA(50, 50, 50, 0), 25, 1);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsContent = L"Inkeys";
			}
			{
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].X = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].Y = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].Width = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].Height = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].EllipseWidth = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].EllipseHeight = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FrameThickness = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor = PptUiWidgetColor(RGBA(250, 250, 250, 0), 10, 1);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FrameColor = PptUiWidgetColor(RGBA(200, 200, 200, 0), 25, 1);

				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].X = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Y = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Width = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Height = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Transparency = PptUiWidgetValue(20, 1);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Img = pptIconBitmap[2];
			}
		}
		// �м�ؼ�
		{
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].X = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Y = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Width = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Height = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].EllipseWidth = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].EllipseHeight = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].FrameThickness = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].FillColor = PptUiWidgetColor(RGBA(225, 225, 225, 0), 25, 1);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].FrameColor = PptUiWidgetColor(RGBA(200, 200, 200, 0), 25, 1);

			{
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].X = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].Y = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].Width = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].Height = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].EllipseWidth = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].EllipseHeight = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FrameThickness = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FillColor = PptUiWidgetColor(RGBA(250, 250, 250, 0), 10, 1);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FrameColor = PptUiWidgetColor(RGBA(200, 200, 200, 0), 25, 1);

				pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].X = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Y = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Width = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Height = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Transparency = PptUiWidgetValue(25, 1);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Img = pptIconBitmap[3];
			}
		}
		// �Ҳ�ؼ�
		{
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].X = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Y = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Width = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Height = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].EllipseWidth = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].EllipseHeight = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].FrameThickness = PptUiWidgetValue(15, 0.1f);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].FillColor = PptUiWidgetColor(RGBA(225, 225, 225, 0), 25, 1);
			pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].FrameColor = PptUiWidgetColor(RGBA(200, 200, 200, 0), 25, 1);

			{
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].X = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Y = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Width = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Height = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].EllipseWidth = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].EllipseHeight = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FrameThickness = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor = PptUiWidgetColor(RGBA(250, 250, 250, 0), 10, 1);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FrameColor = PptUiWidgetColor(RGBA(200, 200, 200, 0), 25, 1);

				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].X = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Y = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Width = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Height = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Transparency = PptUiWidgetValue(25, 1);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Img = pptIconBitmap[1];
			}
			{
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].Left = PptUiWidgetValue(15, 0.1f);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].Top = PptUiWidgetValue(15, 0.1f);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].Right = PptUiWidgetValue(15, 0.1f);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].Bottom = PptUiWidgetValue(15, 0.1f);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].WordsHeight = PptUiWidgetValue(15, 0.1f);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].WordsColor = PptUiWidgetColor(RGBA(50, 50, 50, 0), 25, 1);
				pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].WordsContent = L"Inkeys";
			}
			{
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].X = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].Y = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].Width = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].Height = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].EllipseWidth = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].EllipseHeight = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FrameThickness = PptUiWidgetValue(15, 0.1f);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor = PptUiWidgetColor(RGBA(250, 250, 250, 0), 10, 1);
				pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FrameColor = PptUiWidgetColor(RGBA(200, 200, 200, 0), 25, 1);

				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].X = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Y = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Width = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Height = PptUiWidgetValue(15, 0.1f);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Transparency = PptUiWidgetValue(25, 1);
				pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Img = pptIconBitmap[2];
			}
		}

		memcpy(pptUiRoundRectWidgetTarget, pptUiRoundRectWidget, sizeof(pptUiRoundRectWidget));
		memcpy(pptUiImageWidgetTarget, pptUiImageWidget, sizeof(pptUiImageWidget));
		memcpy(pptUiWordsWidgetTarget, pptUiWordsWidget, sizeof(pptUiWordsWidget));
	}

	int pptTotalSlidesLast = -2;
	int tPptUiChangeSignal;
	PptUiChangeSignal = false;

	clock_t tRecord = clock();
	for (; !offSignal;)
	{
		// ��������Ϣ���
		shared_lock<shared_mutex> DisplaysInfoLock2(DisplaysInfoSm);
		PPTMainMonitor = *PPTMainMonitorStruct;
		DisplaysInfoLock2.unlock();

		// Ppt ��Ϣ���
		int pptTotalSlides = PptInfoState.TotalPage;
		int pptCurrentSlides = PptInfoState.CurrentPage;

		// UI ����
		{
			// UI �����޸ļ��㣨���簴ť��
			if (pptTotalSlides != pptTotalSlidesLast)
			{
				pptTotalSlidesLast = pptTotalSlides;
				if (pptTotalSlides != -1)
				{
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor.v, 160);
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FrameColor.v, 160);

					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor.v, 160);
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FrameColor.v, 160);

					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FillColor.v, 160);
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FrameColor.v, 160);

					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor.v, 160);
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FrameColor.v, 160);

					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v, 160);
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FrameColor.v, 160);
				}
				else if (pptTotalSlides == -1)
				{
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor.v, 0);
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FrameColor.v, 0);

					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor.v, 0);
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FrameColor.v, 0);

					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FillColor.v, 0);
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FrameColor.v, 0);

					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor.v, 0);
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FrameColor.v, 0);

					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v, 0);
					SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FrameColor.v, 0);
				}
			}

			// UI �ؼ�ʵʱ����
			{
				// ���ؼ�
				{
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].X.v = (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
					if (pptTotalSlides == -1) pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Y.v = PPTMainMonitor.MonitorHeight + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
					else pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Y.v = PPTMainMonitor.MonitorHeight - (65) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Width.v = (185) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Height.v = (60) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].EllipseWidth.v = (30) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].EllipseHeight.v = (30) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].FrameThickness.v = (1) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
					if (pptTotalSlides == -1)
					{
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].FillColor.v, 0);
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].FrameColor.v, 0);
					}
					else
					{
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].FillColor.v, 160);
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].FrameColor.v, 160);
					}

					{
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].X.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].X.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Y.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Width.v = (50) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Height.v = (50) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].EllipseWidth.v = (35) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].EllipseHeight.v = (35) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FrameThickness.v = (1) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];

						{
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].X.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].X.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Y.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Width.v = (40) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Height.v = (40) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
							if (pptTotalSlides == -1) pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Transparency.v = 0;
							else pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Transparency.v = 255;
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Img = pptIconBitmap[1];
						}
					}
					{
						pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Left.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].X.v + pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Width.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Top.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Right.v = pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Left.v + (65) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Bottom.v = pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Top.v + (55) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsHeight.v = (20) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						if (pptTotalSlides == -1) SetAlpha(pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsColor.v, 0);
						else SetAlpha(pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsColor.v, 255);
						{
							wstring temp;
							if (pptCurrentSlides >= 100 || pptTotalSlides >= 100)
							{
								temp = pptCurrentSlides == -1 ? L"-" : to_wstring(pptCurrentSlides);
								temp += L"\n";
								temp += to_wstring(pptTotalSlides);
							}
							else
							{
								temp = pptCurrentSlides == -1 ? L"-" : to_wstring(pptCurrentSlides);
								temp += L"/";
								temp += to_wstring(pptTotalSlides);
							}
							pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsContent = temp;
						}
					}
					{
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].X.v = pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Right.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].Y.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].Width.v = (50) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].Height.v = (50) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].EllipseWidth.v = (35) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].EllipseHeight.v = (35) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FrameThickness.v = (1) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];

						{
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftNextPage].X.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].X.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftNextPage].Y.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftNextPage].Width.v = (40) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftNextPage].Height.v = (40) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_LeftWidget];
							if (pptTotalSlides == -1) pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftNextPage].Transparency.v = 0;
							else pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftNextPage].Transparency.v = 255;
							if (pptCurrentSlides == -1) pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftNextPage].Img = pptIconBitmap[3];
							else pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_LeftNextPage].Img = pptIconBitmap[2];
						}
					}
				}
				// �м�ؼ�
				{
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].X.v = PPTMainMonitor.MonitorWidth / 2 - (30) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
					if (pptTotalSlides == -1) pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Y.v = PPTMainMonitor.MonitorHeight + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
					else pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Y.v = PPTMainMonitor.MonitorHeight - (65) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Width.v = (60) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Height.v = (60) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].EllipseWidth.v = (30) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].EllipseHeight.v = (30) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].FrameThickness.v = (1) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
					if (pptTotalSlides == -1)
					{
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].FillColor.v, 0);
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].FrameColor.v, 0);
					}
					else
					{
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].FillColor.v, 160);
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].FrameColor.v, 160);
					}

					{
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].X.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].X.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].Y.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].Width.v = (50) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].Height.v = (50) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].EllipseWidth.v = (35) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].EllipseHeight.v = (35) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FrameThickness.v = (1) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];

						{
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_MiddleEndShow].X.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].X.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Y.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Width.v = (40) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Height.v = (40) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_RightWidget];
							if (pptTotalSlides == -1) pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Transparency.v = 0;
							else pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Transparency.v = 255;
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Img = pptIconBitmap[3];
						}
					}
				}
				// �Ҳ�ؼ�
				{
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].X.v = PPTMainMonitor.MonitorWidth - (190) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
					if (pptTotalSlides == -1) pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Y.v = PPTMainMonitor.MonitorHeight + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
					else pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Y.v = PPTMainMonitor.MonitorHeight - (65) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Width.v = (185) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Height.v = (60) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].EllipseWidth.v = (30) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].EllipseHeight.v = (30) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].FrameThickness.v = (1) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
					if (pptTotalSlides == -1)
					{
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].FillColor.v, 0);
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].FrameColor.v, 0);
					}
					else
					{
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].FillColor.v, 160);
						SetAlpha(pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].FrameColor.v, 160);
					}

					{
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].X.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].X.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Y.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Width.v = (50) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Height.v = (50) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].EllipseWidth.v = (35) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].EllipseHeight.v = (35) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FrameThickness.v = (1) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];

						{
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightPreviousPage].X.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].X.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Y.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Width.v = (40) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Height.v = (40) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
							if (pptTotalSlides == -1) pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Transparency.v = 0;
							else pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Transparency.v = 255;
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Img = pptIconBitmap[1];
						}
					}
					{
						pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_RightPageNum].Left.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].X.v + pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Width.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_RightPageNum].Top.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_RightPageNum].Right.v = pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_RightPageNum].Left.v + (65) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_RightPageNum].Bottom.v = pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_RightPageNum].Top.v + (55) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_RightPageNum].WordsHeight.v = (20) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						if (pptTotalSlides == -1) SetAlpha(pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_RightPageNum].WordsColor.v, 0);
						else SetAlpha(pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_RightPageNum].WordsColor.v, 255);
						{
							pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_RightPageNum].WordsContent = pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsContent;
						}
					}
					{
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].X.v = pptUiWordsWidgetTarget[PptUiWordsWidgetID::BottomSide_RightPageNum].Right.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].Y.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].Width.v = (50) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].Height.v = (50) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].EllipseWidth.v = (35) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].EllipseHeight.v = (35) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FrameThickness.v = (1) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];

						{
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightNextPage].X.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].X.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightNextPage].Y.v = pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].Y.v + (5) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightNextPage].Width.v = (40) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
							pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightNextPage].Height.v = (40) * PptUiWidgetScale[PptUiWidgetScaleID::BottomSide_MiddleWidget];
							if (pptTotalSlides == -1) pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightNextPage].Transparency.v = 0;
							else pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightNextPage].Transparency.v = 255;
							if (pptCurrentSlides == -1) pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightNextPage].Img = pptIconBitmap[3];
							else pptUiImageWidgetTarget[PptUiImageWidgetID::BottomSide_RightNextPage].Img = pptIconBitmap[2];
						}
					}
				}
			}

			// UI �仯����
			{
				tPptUiChangeSignal = false;

				// ����
				for (int i = 0; i < size(pptUiRoundRectWidgetTarget); i++)
				{
					if (pptUiRoundRectWidget[i].X.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiRoundRectWidget[i].X.replace = false;
						pptUiRoundRectWidget[i].X.v = pptUiRoundRectWidgetTarget[i].X.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiRoundRectWidget[i].X.v != pptUiRoundRectWidgetTarget[i].X.v)
					{
						PptUiWidgetValueTransformation(&pptUiRoundRectWidget[i].X.v, pptUiRoundRectWidgetTarget[i].X.v, pptUiRoundRectWidget[i].X.s, pptUiRoundRectWidget[i].X.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiRoundRectWidget[i].Y.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiRoundRectWidget[i].Y.replace = false;
						pptUiRoundRectWidget[i].Y.v = pptUiRoundRectWidgetTarget[i].Y.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiRoundRectWidget[i].Y.v != pptUiRoundRectWidgetTarget[i].Y.v)
					{
						PptUiWidgetValueTransformation(&pptUiRoundRectWidget[i].Y.v, pptUiRoundRectWidgetTarget[i].Y.v, pptUiRoundRectWidget[i].Y.s, pptUiRoundRectWidget[i].Y.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiRoundRectWidget[i].Width.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiRoundRectWidget[i].Width.replace = false;
						pptUiRoundRectWidget[i].Width.v = pptUiRoundRectWidgetTarget[i].Width.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiRoundRectWidget[i].Width.v != pptUiRoundRectWidgetTarget[i].Width.v)
					{
						PptUiWidgetValueTransformation(&pptUiRoundRectWidget[i].Width.v, pptUiRoundRectWidgetTarget[i].Width.v, pptUiRoundRectWidget[i].Width.s, pptUiRoundRectWidget[i].Width.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiRoundRectWidget[i].Height.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiRoundRectWidget[i].Height.replace = false;
						pptUiRoundRectWidget[i].Height.v = pptUiRoundRectWidgetTarget[i].Height.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiRoundRectWidget[i].Height.v != pptUiRoundRectWidgetTarget[i].Height.v)
					{
						PptUiWidgetValueTransformation(&pptUiRoundRectWidget[i].Height.v, pptUiRoundRectWidgetTarget[i].Height.v, pptUiRoundRectWidget[i].Height.s, pptUiRoundRectWidget[i].Height.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiRoundRectWidget[i].EllipseWidth.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiRoundRectWidget[i].EllipseWidth.replace = false;
						pptUiRoundRectWidget[i].EllipseWidth.v = pptUiRoundRectWidgetTarget[i].EllipseWidth.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiRoundRectWidget[i].EllipseWidth.v != pptUiRoundRectWidgetTarget[i].EllipseWidth.v)
					{
						PptUiWidgetValueTransformation(&pptUiRoundRectWidget[i].EllipseWidth.v, pptUiRoundRectWidgetTarget[i].EllipseWidth.v, pptUiRoundRectWidget[i].EllipseWidth.s, pptUiRoundRectWidget[i].EllipseWidth.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiRoundRectWidget[i].EllipseHeight.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiRoundRectWidget[i].EllipseHeight.replace = false;
						pptUiRoundRectWidget[i].EllipseHeight.v = pptUiRoundRectWidgetTarget[i].EllipseHeight.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiRoundRectWidget[i].EllipseHeight.v != pptUiRoundRectWidgetTarget[i].EllipseHeight.v)
					{
						PptUiWidgetValueTransformation(&pptUiRoundRectWidget[i].EllipseHeight.v, pptUiRoundRectWidgetTarget[i].EllipseHeight.v, pptUiRoundRectWidget[i].EllipseHeight.s, pptUiRoundRectWidget[i].EllipseHeight.e);
						tPptUiChangeSignal = true;
					}

					//=====

					if (pptUiRoundRectWidget[i].FrameThickness.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiRoundRectWidget[i].FrameThickness.replace = false;
						pptUiRoundRectWidget[i].FrameThickness.v = pptUiRoundRectWidgetTarget[i].FrameThickness.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiRoundRectWidget[i].FrameThickness.v != pptUiRoundRectWidgetTarget[i].FrameThickness.v)
					{
						PptUiWidgetValueTransformation(&pptUiRoundRectWidget[i].FrameThickness.v, pptUiRoundRectWidgetTarget[i].FrameThickness.v, pptUiRoundRectWidget[i].FrameThickness.s, pptUiRoundRectWidget[i].FrameThickness.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiRoundRectWidget[i].FrameColor.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiRoundRectWidget[i].FrameColor.replace = false;
						pptUiRoundRectWidget[i].FrameColor.v = pptUiRoundRectWidgetTarget[i].FrameColor.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiRoundRectWidget[i].FrameColor.v != pptUiRoundRectWidgetTarget[i].FrameColor.v)
					{
						PptUiWidgetColorTransformation(&pptUiRoundRectWidget[i].FrameColor.v, pptUiRoundRectWidgetTarget[i].FrameColor.v, pptUiRoundRectWidget[i].FrameColor.s, pptUiRoundRectWidget[i].FrameColor.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiRoundRectWidget[i].FillColor.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiRoundRectWidget[i].FillColor.replace = false;
						pptUiRoundRectWidget[i].FillColor.v = pptUiRoundRectWidgetTarget[i].FillColor.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiRoundRectWidget[i].FillColor.v != pptUiRoundRectWidgetTarget[i].FillColor.v)
					{
						PptUiWidgetColorTransformation(&pptUiRoundRectWidget[i].FillColor.v, pptUiRoundRectWidgetTarget[i].FillColor.v, pptUiRoundRectWidget[i].FillColor.s, pptUiRoundRectWidget[i].FillColor.e);
						tPptUiChangeSignal = true;
					}
				}
				// ͼ��
				for (int i = 0; i < size(pptUiImageWidgetTarget); i++)
				{
					if (pptUiImageWidget[i].X.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiImageWidget[i].X.replace = false;
						pptUiImageWidget[i].X.v = pptUiImageWidgetTarget[i].X.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiImageWidget[i].X.v != pptUiImageWidgetTarget[i].X.v)
					{
						PptUiWidgetValueTransformation(&pptUiImageWidget[i].X.v, pptUiImageWidgetTarget[i].X.v, pptUiImageWidget[i].X.s, pptUiImageWidget[i].X.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiImageWidget[i].Y.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiImageWidget[i].Y.replace = false;
						pptUiImageWidget[i].Y.v = pptUiImageWidgetTarget[i].Y.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiImageWidget[i].Y.v != pptUiImageWidgetTarget[i].Y.v)
					{
						PptUiWidgetValueTransformation(&pptUiImageWidget[i].Y.v, pptUiImageWidgetTarget[i].Y.v, pptUiImageWidget[i].Y.s, pptUiImageWidget[i].Y.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiImageWidget[i].Width.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiImageWidget[i].Width.replace = false;
						pptUiImageWidget[i].Width.v = pptUiImageWidgetTarget[i].Width.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiImageWidget[i].Width.v != pptUiImageWidgetTarget[i].Width.v)
					{
						PptUiWidgetValueTransformation(&pptUiImageWidget[i].Width.v, pptUiImageWidgetTarget[i].Width.v, pptUiImageWidget[i].Width.s, pptUiImageWidget[i].Width.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiImageWidget[i].Height.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiImageWidget[i].Height.replace = false;
						pptUiImageWidget[i].Height.v = pptUiImageWidgetTarget[i].Height.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiImageWidget[i].Height.v != pptUiImageWidgetTarget[i].Height.v)
					{
						PptUiWidgetValueTransformation(&pptUiImageWidget[i].Height.v, pptUiImageWidgetTarget[i].Height.v, pptUiImageWidget[i].Height.s, pptUiImageWidget[i].Height.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiImageWidget[i].Transparency.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiImageWidget[i].Transparency.replace = false;
						pptUiImageWidget[i].Transparency.v = pptUiImageWidgetTarget[i].Transparency.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiImageWidget[i].Transparency.v != pptUiImageWidgetTarget[i].Transparency.v)
					{
						PptUiWidgetValueTransformation(&pptUiImageWidget[i].Transparency.v, pptUiImageWidgetTarget[i].Transparency.v, pptUiImageWidget[i].Transparency.s, pptUiImageWidget[i].Transparency.e);
						tPptUiChangeSignal = true;
					}

					//=====

					if (pptUiImageWidget[i].Img != pptUiImageWidgetTarget[i].Img)
					{
						pptUiImageWidget[i].Img = pptUiImageWidgetTarget[i].Img;
						tPptUiChangeSignal = true;
					}
				}
				// ����
				for (int i = 0; i < size(pptUiWordsWidgetTarget); i++)
				{
					if (pptUiWordsWidget[i].Left.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiWordsWidget[i].Left.replace = false;
						pptUiWordsWidget[i].Left.v = pptUiWordsWidgetTarget[i].Left.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiWordsWidget[i].Left.v != pptUiWordsWidgetTarget[i].Left.v)
					{
						PptUiWidgetValueTransformation(&pptUiWordsWidget[i].Left.v, pptUiWordsWidgetTarget[i].Left.v, pptUiWordsWidget[i].Left.s, pptUiWordsWidget[i].Left.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiWordsWidget[i].Top.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiWordsWidget[i].Top.replace = false;
						pptUiWordsWidget[i].Top.v = pptUiWordsWidgetTarget[i].Top.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiWordsWidget[i].Top.v != pptUiWordsWidgetTarget[i].Top.v)
					{
						PptUiWidgetValueTransformation(&pptUiWordsWidget[i].Top.v, pptUiWordsWidgetTarget[i].Top.v, pptUiWordsWidget[i].Top.s, pptUiWordsWidget[i].Top.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiWordsWidget[i].Right.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiWordsWidget[i].Right.replace = false;
						pptUiWordsWidget[i].Right.v = pptUiWordsWidgetTarget[i].Right.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiWordsWidget[i].Right.v != pptUiWordsWidgetTarget[i].Right.v)
					{
						PptUiWidgetValueTransformation(&pptUiWordsWidget[i].Right.v, pptUiWordsWidgetTarget[i].Right.v, pptUiWordsWidget[i].Right.s, pptUiWordsWidget[i].Right.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiWordsWidget[i].Bottom.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiWordsWidget[i].Bottom.replace = false;
						pptUiWordsWidget[i].Bottom.v = pptUiWordsWidgetTarget[i].Bottom.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiWordsWidget[i].Bottom.v != pptUiWordsWidgetTarget[i].Bottom.v)
					{
						PptUiWidgetValueTransformation(&pptUiWordsWidget[i].Bottom.v, pptUiWordsWidgetTarget[i].Bottom.v, pptUiWordsWidget[i].Bottom.s, pptUiWordsWidget[i].Bottom.e);
						tPptUiChangeSignal = true;
					}

					//=====

					if (pptUiWordsWidget[i].WordsHeight.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiWordsWidget[i].WordsHeight.replace = false;
						pptUiWordsWidget[i].WordsHeight.v = pptUiWordsWidgetTarget[i].WordsHeight.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiWordsWidget[i].WordsHeight.v != pptUiWordsWidgetTarget[i].WordsHeight.v)
					{
						PptUiWidgetValueTransformation(&pptUiWordsWidget[i].WordsHeight.v, pptUiWordsWidgetTarget[i].WordsHeight.v, pptUiWordsWidget[i].WordsHeight.s, pptUiWordsWidget[i].WordsHeight.e);
						tPptUiChangeSignal = true;
					}

					if (pptUiWordsWidget[i].WordsColor.replace || PptUiAllReplaceSignal || !PptUiAnimationEnable)
					{
						pptUiWordsWidget[i].WordsColor.replace = false;
						pptUiWordsWidget[i].WordsColor.v = pptUiWordsWidgetTarget[i].WordsColor.v;
						tPptUiChangeSignal = true;
					}
					else if (pptUiWordsWidget[i].WordsColor.v != pptUiWordsWidgetTarget[i].WordsColor.v)
					{
						PptUiWidgetColorTransformation(&pptUiWordsWidget[i].WordsColor.v, pptUiWordsWidgetTarget[i].WordsColor.v, pptUiWordsWidget[i].WordsColor.s, pptUiWordsWidget[i].WordsColor.e);
						tPptUiChangeSignal = true;
					}

					//=====

					if (pptUiWordsWidget[i].WordsContent != pptUiWordsWidgetTarget[i].WordsContent)
					{
						pptUiWordsWidget[i].WordsContent = pptUiWordsWidgetTarget[i].WordsContent;
						tPptUiChangeSignal = true;
					}
				}

				if (tPptUiChangeSignal) PptUiChangeSignal = true;
				else PptUiChangeSignal = false;
			}
		}

		// ��̬ƽ��֡��
		if (tRecord)
		{
			int delay = 1000 / 60 - (clock() - tRecord);
			if (delay > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		}
		tRecord = clock();
	}

	threadStatus[L"PptUI"] = false;
}
void PptInfo()
{
	threadStatus[L"PptInfo"] = true;

	bool Initialization = false; // �ؼ���ʼ�����
	for (; !offSignal;)
	{
		// Ppt ��Ϣ��� | �ؼ���Ϣ����
		if (!Initialization && PptInfoState.TotalPage != -1)
		{
			ppt_show = GetPptShow();

			std::wstringstream ss(GetPptTitle());
			getline(ss, ppt_title);
			getline(ss, ppt_software);

			if (ppt_software.find(L"WPS") != ppt_software.npos) ppt_software = L"WPS";
			else ppt_software = L"PowerPoint";

			//if (!ppt_title_recond[ppt_title]) FreezePPT = true;
			Initialization = true;
		}
		else if (Initialization && PptInfoState.TotalPage == -1)
		{
			PptImg.IsSave = false;
			PptImg.IsSaved.clear();
			PptImg.Image.clear();

			ppt_show = NULL, ppt_software = L"";

			FreezePPT = false;
			Initialization = false;
		}

		this_thread::sleep_for(chrono::milliseconds(500));
	}

	threadStatus[L"PptInfo"] = false;
}
void PptDraw()
{
	threadStatus[L"PptDraw"] = true;

	//ppt���ڳ�ʼ��
	MainMonitorStruct PPTMainMonitor;
	{
		DisableResizing(ppt_window, true);//��ֹ��������
		SetWindowLong(ppt_window, GWL_STYLE, GetWindowLong(ppt_window, GWL_STYLE) & ~WS_CAPTION);//���ر�����
		SetWindowPos(ppt_window, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_DRAWFRAME | SWP_NOACTIVATE);
		SetWindowLong(ppt_window, GWL_EXSTYLE, WS_EX_TOOLWINDOW);//����������

		shared_lock<shared_mutex> DisplaysInfoLock2(DisplaysInfoSm);
		PPTMainMonitor = MainMonitor;
		DisplaysInfoLock2.unlock();

		PptWindowBackground.Resize(PPTMainMonitor.MonitorWidth, PPTMainMonitor.MonitorHeight);
		SetWindowPos(ppt_window, NULL, PPTMainMonitor.rcMonitor.left, PPTMainMonitor.rcMonitor.top, PPTMainMonitor.MonitorWidth, PPTMainMonitor.MonitorHeight, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	// ���� EasyX ���ݵ� DC Render Target
	ID2D1DCRenderTarget* DCRenderTarget = nullptr;
	D2DFactory->CreateDCRenderTarget(&D2DProperty, &DCRenderTarget);

	// �� EasyX DC
	RECT PptBackgroundWindowRect = { 0, 0, PptWindowBackground.getwidth(), PptWindowBackground.getheight() };
	DCRenderTarget->BindDC(GetImageHDC(&PptWindowBackground), &PptBackgroundWindowRect);

	// ���ÿ����
	DCRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE::D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	//ý���ʼ��
	{
		loadimage(&PptIcon[1], L"PNG", L"ppt1");
		loadimage(&PptIcon[2], L"PNG", L"ppt2");
		loadimage(&PptIcon[3], L"PNG", L"ppt3");

		ChangeColor(PptIcon[1], RGB(50, 50, 50));
		ChangeColor(PptIcon[2], RGB(50, 50, 50));
		ChangeColor(PptIcon[3], RGB(50, 50, 50));

		{
			int width = PptIcon[1].getwidth();
			int height = PptIcon[1].getheight();
			DWORD* pMem = GetImageBuffer(&PptIcon[1]);

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

			D2D1_BITMAP_PROPERTIES bitmapProps = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
			DCRenderTarget->CreateBitmap(D2D1::SizeU(width, height), data, width * 4, bitmapProps, &pptIconBitmap[1]);
			delete[] data;
		}
		{
			int width = PptIcon[2].getwidth();
			int height = PptIcon[2].getheight();
			DWORD* pMem = GetImageBuffer(&PptIcon[2]);

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

			D2D1_BITMAP_PROPERTIES bitmapProps = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
			DCRenderTarget->CreateBitmap(D2D1::SizeU(width, height), data, width * 4, bitmapProps, &pptIconBitmap[2]);
			delete[] data;
		}
		{
			int width = PptIcon[3].getwidth();
			int height = PptIcon[3].getheight();
			DWORD* pMem = GetImageBuffer(&PptIcon[3]);

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

			D2D1_BITMAP_PROPERTIES bitmapProps = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
			DCRenderTarget->CreateBitmap(D2D1::SizeU(width, height), data, width * 4, bitmapProps, &pptIconBitmap[3]);
			delete[] data;
		}
	}

	thread(PptUI, &PPTMainMonitor).detach();

	// ����BLENDFUNCTION�ṹ��
	BLENDFUNCTION blend;
	{
		blend.BlendOp = AC_SRC_OVER;
		blend.BlendFlags = 0;
		blend.SourceConstantAlpha = 255; // ����͸���ȣ�0Ϊȫ͸����255Ϊ��͸��
		blend.AlphaFormat = AC_SRC_ALPHA; // ʹ��Դͼ���alphaͨ��
	}

	POINT ptSrc = { 0,0 };
	SIZE sizeWnd = { PptWindowBackground.getwidth(),PptWindowBackground.getheight() };
	POINT ptDst = { PPTMainMonitor.rcMonitor.left, PPTMainMonitor.rcMonitor.top };

	// ����UpdateLayeredWindow�������´���
	UPDATELAYEREDWINDOWINFO ulwi = { 0 };
	{
		ulwi.cbSize = sizeof(ulwi);
		ulwi.hdcDst = GetDC(NULL);
		ulwi.hdcSrc = GetImageHDC(&PptWindowBackground);
		ulwi.pptDst = &ptDst;
		ulwi.psize = &sizeWnd;
		ulwi.pptSrc = &ptSrc;
		ulwi.crKey = RGB(255, 255, 255);
		ulwi.pblend = &blend;
		ulwi.dwFlags = ULW_ALPHA;
	}

	while (!(GetWindowLong(ppt_window, GWL_EXSTYLE) & WS_EX_LAYERED))
	{
		SetWindowLong(ppt_window, GWL_EXSTYLE, GetWindowLong(ppt_window, GWL_EXSTYLE) | WS_EX_LAYERED);
		if (GetWindowLong(ppt_window, GWL_EXSTYLE) & WS_EX_LAYERED) break;

		this_thread::sleep_for(chrono::milliseconds(10));
	}
	while (!(GetWindowLong(ppt_window, GWL_EXSTYLE) & WS_EX_NOACTIVATE))
	{
		SetWindowLong(ppt_window, GWL_EXSTYLE, GetWindowLong(ppt_window, GWL_EXSTYLE) | WS_EX_NOACTIVATE);
		if (GetWindowLong(ppt_window, GWL_EXSTYLE) & WS_EX_NOACTIVATE) break;

		this_thread::sleep_for(chrono::milliseconds(10));
	}

	clock_t tRecord = clock();
	for (bool IsShowWindow = false; !offSignal;)
	{
		// ��������Ϣ���
		{
			shared_lock<shared_mutex> DisplaysInfoLock2(DisplaysInfoSm);
			bool MainMonitorDifferent = (PPTMainMonitor != MainMonitor);
			if (MainMonitorDifferent) PPTMainMonitor = MainMonitor;
			DisplaysInfoLock2.unlock();

			if (MainMonitorDifferent)
			{
				PptWindowBackground.Resize(PPTMainMonitor.MonitorWidth, PPTMainMonitor.MonitorHeight);
				ulwi.hdcSrc = GetImageHDC(&PptWindowBackground);

				SetWindowPos(ppt_window, NULL, PPTMainMonitor.rcMonitor.left, PPTMainMonitor.rcMonitor.top, PPTMainMonitor.MonitorWidth, PPTMainMonitor.MonitorHeight, SWP_NOZORDER | SWP_NOACTIVATE);

				sizeWnd = { PPTMainMonitor.MonitorWidth, PPTMainMonitor.MonitorHeight };
				POINT ptDst = { PPTMainMonitor.rcMonitor.left, PPTMainMonitor.rcMonitor.top };

				RECT PptBackgroundWindowRect = { 0, 0, PPTMainMonitor.MonitorWidth, PPTMainMonitor.MonitorHeight };
				DCRenderTarget->BindDC(GetImageHDC(&PptWindowBackground), &PptBackgroundWindowRect);
			}
		}

		// ���Ʋ���
		if (PptUiChangeSignal)
		{
			SetImageColor(PptWindowBackground, RGBA(0, 0, 0, 0), true);

			DCRenderTarget->BeginDraw();

			// ���ؼ�
			{
				// pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget]
				{
					ID2D1SolidColorBrush* pFrameBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].FrameColor.v), &pFrameBrush);
					ID2D1SolidColorBrush* pFillBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].FillColor.v), &pFillBrush);

					D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(D2D1::RectF(
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].X.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Y.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].X.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Width.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Y.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].Height.v),
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].EllipseWidth.v / 2.0f,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].EllipseHeight.v / 2.0f
					);

					DCRenderTarget->FillRoundedRectangle(&roundedRect, pFillBrush);
					DCRenderTarget->DrawRoundedRectangle(&roundedRect, pFrameBrush, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget].FrameThickness.v);

					DxObjectSafeRelease(&pFrameBrush);
					DxObjectSafeRelease(&pFillBrush);
				}

				// pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage]
				{
					ID2D1SolidColorBrush* pFrameBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FrameColor.v), &pFrameBrush);
					ID2D1SolidColorBrush* pFillBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor.v), &pFillBrush);

					D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(D2D1::RectF(
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].X.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Y.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].X.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Width.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Y.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].Height.v),
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].EllipseWidth.v / 2.0f,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].EllipseHeight.v / 2.0f
					);

					DCRenderTarget->FillRoundedRectangle(&roundedRect, pFillBrush);
					DCRenderTarget->DrawRoundedRectangle(&roundedRect, pFrameBrush, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FrameThickness.v);

					DxObjectSafeRelease(&pFrameBrush);
					DxObjectSafeRelease(&pFillBrush);
				}
				// pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage]
				if (pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Img != NULL)
				{
					DCRenderTarget->DrawBitmap(pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Img, D2D1::RectF(pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].X.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Y.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].X.v + pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Width.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Y.v + pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Height.v), pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftPreviousPage].Transparency.v / 255.0f);
				}

				// pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum]
				{
					IDWriteTextFormat* textFormat = NULL;

					D2DTextFactory->CreateTextFormat(
						L"HarmonyOS Sans SC",
						D2DFontCollection,
						DWRITE_FONT_WEIGHT_NORMAL,
						DWRITE_FONT_STYLE_NORMAL,
						DWRITE_FONT_STRETCH_NORMAL,
						pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsHeight.v,
						L"zh-cn",
						&textFormat
					);

					ID2D1SolidColorBrush* pBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(D2D1::ColorF(ConvertToD2DColor(pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsColor.v)), &pBrush);

					textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
					textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

					DCRenderTarget->DrawText(
						pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsContent.c_str(),  // �ı�
						wcslen(pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].WordsContent.c_str()),  // �ı�����
						textFormat,  // �ı���ʽ
						D2D1::RectF(
							pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Left.v,
							pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Top.v,
							pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Right.v,
							pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_LeftPageNum].Bottom.v
						),
						pBrush
					);

					DxObjectSafeRelease(&pBrush);
				}

				// pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage]
				{
					ID2D1SolidColorBrush* pFrameBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FrameColor.v), &pFrameBrush);
					ID2D1SolidColorBrush* pFillBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor.v), &pFillBrush);

					D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(D2D1::RectF(
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].X.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].Y.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].X.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].Width.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].Y.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].Height.v),
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].EllipseWidth.v / 2.0f,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].EllipseHeight.v / 2.0f
					);

					DCRenderTarget->FillRoundedRectangle(&roundedRect, pFillBrush);
					DCRenderTarget->DrawRoundedRectangle(&roundedRect, pFrameBrush, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FrameThickness.v);

					DxObjectSafeRelease(&pFrameBrush);
					DxObjectSafeRelease(&pFillBrush);
				}
				// pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage]
				if (pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Img != NULL)
				{
					DCRenderTarget->DrawBitmap(pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Img, D2D1::RectF(pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].X.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Y.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].X.v + pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Width.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Y.v + pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Height.v), pptUiImageWidget[PptUiImageWidgetID::BottomSide_LeftNextPage].Transparency.v / 255.0f);
				}
			}
			// �м�ؼ�
			{
				// pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget]
				{
					ID2D1SolidColorBrush* pFrameBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].FrameColor.v), &pFrameBrush);
					ID2D1SolidColorBrush* pFillBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].FillColor.v), &pFillBrush);

					D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(D2D1::RectF(
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].X.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Y.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].X.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Width.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Y.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].Height.v),
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].EllipseWidth.v / 2.0f,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].EllipseHeight.v / 2.0f
					);

					DCRenderTarget->FillRoundedRectangle(&roundedRect, pFillBrush);
					DCRenderTarget->DrawRoundedRectangle(&roundedRect, pFrameBrush, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget].FrameThickness.v);

					DxObjectSafeRelease(&pFrameBrush);
					DxObjectSafeRelease(&pFillBrush);
				}

				// pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow]
				{
					ID2D1SolidColorBrush* pFrameBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FrameColor.v), &pFrameBrush);
					ID2D1SolidColorBrush* pFillBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FillColor.v), &pFillBrush);

					D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(D2D1::RectF(
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].X.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].Y.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].X.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].Width.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].Y.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].Height.v),
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].EllipseWidth.v / 2.0f,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].EllipseHeight.v / 2.0f
					);

					DCRenderTarget->FillRoundedRectangle(&roundedRect, pFillBrush);
					DCRenderTarget->DrawRoundedRectangle(&roundedRect, pFrameBrush, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FrameThickness.v);

					DxObjectSafeRelease(&pFrameBrush);
					DxObjectSafeRelease(&pFillBrush);
				}
				// pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow]
				if (pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Img != NULL)
				{
					DCRenderTarget->DrawBitmap(pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Img, D2D1::RectF(pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].X.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Y.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].X.v + pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Width.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Y.v + pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Height.v), pptUiImageWidget[PptUiImageWidgetID::BottomSide_MiddleEndShow].Transparency.v / 255.0f);
				}
			}
			// �Ҳ�ؼ�
			{
				// pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget]
				{
					ID2D1SolidColorBrush* pFrameBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].FrameColor.v), &pFrameBrush);
					ID2D1SolidColorBrush* pFillBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].FillColor.v), &pFillBrush);

					D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(D2D1::RectF(
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].X.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Y.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].X.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Width.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Y.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].Height.v),
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].EllipseWidth.v / 2.0f,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].EllipseHeight.v / 2.0f
					);

					DCRenderTarget->FillRoundedRectangle(&roundedRect, pFillBrush);
					DCRenderTarget->DrawRoundedRectangle(&roundedRect, pFrameBrush, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget].FrameThickness.v);

					DxObjectSafeRelease(&pFrameBrush);
					DxObjectSafeRelease(&pFillBrush);
				}

				// pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage]
				{
					ID2D1SolidColorBrush* pFrameBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FrameColor.v), &pFrameBrush);
					ID2D1SolidColorBrush* pFillBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor.v), &pFillBrush);

					D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(D2D1::RectF(
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].X.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Y.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].X.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Width.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Y.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].Height.v),
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].EllipseWidth.v / 2.0f,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].EllipseHeight.v / 2.0f
					);

					DCRenderTarget->FillRoundedRectangle(&roundedRect, pFillBrush);
					DCRenderTarget->DrawRoundedRectangle(&roundedRect, pFrameBrush, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FrameThickness.v);

					DxObjectSafeRelease(&pFrameBrush);
					DxObjectSafeRelease(&pFillBrush);
				}
				// pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage]
				if (pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Img != NULL)
				{
					DCRenderTarget->DrawBitmap(pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Img, D2D1::RectF(pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].X.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Y.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].X.v + pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Width.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Y.v + pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Height.v), pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightPreviousPage].Transparency.v / 255.0f);
				}

				// pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum]
				{
					IDWriteTextFormat* textFormat = NULL;
					D2DTextFactory->CreateTextFormat(
						L"HarmonyOS Sans SC",
						D2DFontCollection,
						DWRITE_FONT_WEIGHT_NORMAL,
						DWRITE_FONT_STYLE_NORMAL,
						DWRITE_FONT_STRETCH_NORMAL,
						pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].WordsHeight.v,
						L"zh-cn",
						&textFormat
					);

					ID2D1SolidColorBrush* pBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(D2D1::ColorF(ConvertToD2DColor(pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].WordsColor.v)), &pBrush);

					textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
					textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

					DCRenderTarget->DrawText(
						pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].WordsContent.c_str(),  // �ı�
						wcslen(pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].WordsContent.c_str()),  // �ı�����
						textFormat,  // �ı���ʽ
						D2D1::RectF(
							pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].Left.v,
							pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].Top.v,
							pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].Right.v,
							pptUiWordsWidget[PptUiWordsWidgetID::BottomSide_RightPageNum].Bottom.v
						),
						pBrush
					);

					DxObjectSafeRelease(&pBrush);
				}

				// pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage]
				{
					ID2D1SolidColorBrush* pFrameBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FrameColor.v), &pFrameBrush);
					ID2D1SolidColorBrush* pFillBrush = NULL;
					DCRenderTarget->CreateSolidColorBrush(ConvertToD2DColor(pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v), &pFillBrush);

					D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(D2D1::RectF(
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].X.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].Y.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].X.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].Width.v,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].Y.v + pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].Height.v),
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].EllipseWidth.v / 2.0f,
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].EllipseHeight.v / 2.0f
					);

					DCRenderTarget->FillRoundedRectangle(&roundedRect, pFillBrush);
					DCRenderTarget->DrawRoundedRectangle(&roundedRect, pFrameBrush, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FrameThickness.v);

					DxObjectSafeRelease(&pFrameBrush);
					DxObjectSafeRelease(&pFillBrush);
				}
				// pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage]
				if (pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Img != NULL)
				{
					DCRenderTarget->DrawBitmap(pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Img, D2D1::RectF(pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].X.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Y.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].X.v + pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Width.v, pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Y.v + pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Height.v), pptUiImageWidget[PptUiImageWidgetID::BottomSide_RightNextPage].Transparency.v / 255.0f);
				}
			}

			DCRenderTarget->EndDraw();

			{
				UpdateLayeredWindowIndirect(ppt_window, &ulwi);

				if (!IsShowWindow)
				{
					IdtWindowsIsVisible.pptWindow = true;
					IsShowWindow = true;
				}

				// ��̬ƽ��֡��
				if (tRecord)
				{
					int delay = 1000 / 60 - (clock() - tRecord);
					if (delay > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay));
				}
				tRecord = clock();
			}
		}
		else for (int i = 1; i <= 50; i++)
		{
			if (PptUiChangeSignal) break;
			this_thread::sleep_for(chrono::milliseconds(10));
		}
	}

	for (int r = 0; r < (int)size(pptIconBitmap); r++) DxObjectSafeRelease(&pptIconBitmap[r]);
	DxObjectSafeRelease(&DCRenderTarget);

	for (int i = 1; i <= 5; i++)
	{
		if (!threadStatus[L"PptUI"]) break;
		this_thread::sleep_for(chrono::milliseconds(500));
	}
	threadStatus[L"PptDraw"] = false;
}
void PptInteract()
{
	ExMessage m;
	int last_x = -1, last_y = -1;

	while (!offSignal)
	{
		if (PptInfoStateBuffer.TotalPage != -1)
		{
			hiex::getmessage_win32(&m, EM_MOUSE, ppt_window);
			if (PptInfoStateBuffer.TotalPage == -1)
			{
				hiex::flushmessage_win32(EM_MOUSE, ppt_window);
				continue;
			}

			// ��� ��һҳ
			if (PptUiIsInRoundRect(m.x, m.y, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage]))
			{
				if (last_x != m.x || last_y != m.y)
				{
					if (pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor.v != RGBA(225, 225, 225, 255))
					{
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor.v = RGBA(225, 225, 225, 255);
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor.replace = true;
					}
				}
				else pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor.v = RGBA(250, 250, 250, 160);

				if (m.message == WM_LBUTTONDOWN)
				{
					SetForegroundWindow(ppt_show);

					PreviousPptSlides();
					pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor.v = RGBA(200, 200, 200, 255);

					std::chrono::high_resolution_clock::time_point KeyboardInteractionManipulated = std::chrono::high_resolution_clock::now();
					while (1)
					{
						if (!KeyBoradDown[VK_LBUTTON]) break;
						if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - KeyboardInteractionManipulated).count() >= 400)
						{
							PreviousPptSlides();
							pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor.v = RGBA(200, 200, 200, 255);
						}

						this_thread::sleep_for(chrono::milliseconds(15));
					}

					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor.v = RGBA(250, 250, 250, 160);
					hiex::flushmessage_win32(EM_MOUSE, ppt_window);

					POINT pt;
					GetCursorPos(&pt);
					last_x = pt.x, last_y = pt.y;
				}
			}
			else if (PptInfoStateBuffer.TotalPage != -1) pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_PreviousPage].FillColor.v = RGBA(250, 250, 250, 160);
			// ��� ��һҳ
			if (PptUiIsInRoundRect(m.x, m.y, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage]))
			{
				if (last_x != m.x || last_y != m.y)
				{
					if (pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor.v != RGBA(225, 225, 225, 255))
					{
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor.v = RGBA(225, 225, 225, 255);
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor.replace = true;
					}
				}
				else pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor.v = RGBA(250, 250, 250, 160);

				if (m.message == WM_LBUTTONDOWN)
				{
					int temp_currentpage = PptInfoState.CurrentPage;
					if (temp_currentpage == -1 && stateMode.StateModeSelect != StateModeSelectEnum::IdtSelection && penetrate.select == false)
					{
						if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) == 1)
						{
							ChangeStateModeToSelection();
						}
					}
					else if (temp_currentpage == -1)
					{
						EndPptShow();
					}
					else
					{
						SetForegroundWindow(ppt_show);

						NextPptSlides(temp_currentpage);
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor.v = RGBA(200, 200, 200, 255);

						std::chrono::high_resolution_clock::time_point KeyboardInteractionManipulated = std::chrono::high_resolution_clock::now();
						while (1)
						{
							if (!KeyBoradDown[VK_LBUTTON]) break;

							if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - KeyboardInteractionManipulated).count() >= 400)
							{
								temp_currentpage = PptInfoState.CurrentPage;
								if (temp_currentpage == -1 && stateMode.StateModeSelect != StateModeSelectEnum::IdtSelection && penetrate.select == false)
								{
									if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) == 1)
									{
										EndPptShow();

										ChangeStateModeToSelection();
									}
									break;
								}
								else if (temp_currentpage != -1)
								{
									NextPptSlides(temp_currentpage);
									pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor.v = RGBA(200, 200, 200, 255);
								}
							}

							this_thread::sleep_for(chrono::milliseconds(15));
						}
					}

					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor.v = RGBA(250, 250, 250, 160);
					hiex::flushmessage_win32(EM_MOUSE, ppt_window);

					POINT pt;
					GetCursorPos(&pt);
					last_x = pt.x, last_y = pt.y;
				}
			}
			else if (PptInfoStateBuffer.TotalPage != -1) pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_LeftPageWidget_NextPage].FillColor.v = RGBA(250, 250, 250, 160);

			// �м� ������ӳ
			if (PptUiIsInRoundRect(m.x, m.y, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow]))
			{
				if (last_x != m.x || last_y != m.y)
				{
					if (pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FillColor.v != RGBA(225, 225, 225, 255))
					{
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FillColor.v = RGBA(225, 225, 225, 255);
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FillColor.replace = true;
					}
				}
				else pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FillColor.v = RGBA(250, 250, 250, 160);

				if (m.message == WM_LBUTTONDOWN)
				{
					int lx = m.x, ly = m.y;
					while (1)
					{
						ExMessage m = hiex::getmessage_win32(EM_MOUSE, ppt_window);
						if (PptUiIsInRoundRect(m.x, m.y, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow]))
						{
							if (!m.lbutton)
							{
								pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FillColor.v = RGBA(200, 200, 200, 255);

								if (stateMode.StateModeSelect != StateModeSelectEnum::IdtSelection && penetrate.select == false)
								{
									if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) != 1) break;

									ChangeStateModeToSelection();
								}

								EndPptShow();

								break;
							}
						}
						else break;
					}

					hiex::flushmessage_win32(EM_MOUSE, ppt_window);

					POINT pt;
					GetCursorPos(&pt);
					last_x = pt.x, last_y = pt.y;
				}
			}
			else if (PptInfoStateBuffer.TotalPage != -1)
			{
				pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_MiddleTabSlideWidget_EndShow].FillColor.v = RGBA(250, 250, 250, 160);
			}

			// �Ҳ� ��һҳ
			if (PptUiIsInRoundRect(m.x, m.y, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage]))
			{
				if (last_x != m.x || last_y != m.y)
				{
					if (pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor.v != RGBA(225, 225, 225, 255))
					{
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor.v = RGBA(225, 225, 225, 255);
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor.replace = true;
					}
				}
				else pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor.v = RGBA(250, 250, 250, 160);

				if (m.message == WM_LBUTTONDOWN)
				{
					SetForegroundWindow(ppt_show);

					PreviousPptSlides();
					pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor.v = RGBA(200, 200, 200, 255);

					std::chrono::high_resolution_clock::time_point KeyboardInteractionManipulated = std::chrono::high_resolution_clock::now();
					while (1)
					{
						if (!KeyBoradDown[VK_LBUTTON]) break;
						if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - KeyboardInteractionManipulated).count() >= 400)
						{
							PreviousPptSlides();
							pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor.v = RGBA(200, 200, 200, 255);
						}

						this_thread::sleep_for(chrono::milliseconds(15));
					}

					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor.v = RGBA(250, 250, 250, 160);
					hiex::flushmessage_win32(EM_MOUSE, ppt_window);

					POINT pt;
					GetCursorPos(&pt);
					last_x = pt.x, last_y = pt.y;
				}
			}
			else if (PptInfoStateBuffer.TotalPage != -1) pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_PreviousPage].FillColor.v = RGBA(250, 250, 250, 160);
			// �Ҳ� ��һҳ
			if (PptUiIsInRoundRect(m.x, m.y, pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage]))
			{
				if (last_x != m.x || last_y != m.y)
				{
					if (pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v != RGBA(225, 225, 225, 255))
					{
						pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v = RGBA(225, 225, 225, 255);
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.replace = true;
					}
				}
				else pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v = RGBA(250, 250, 250, 160);

				if (m.message == WM_LBUTTONDOWN)
				{
					int temp_currentpage = PptInfoState.CurrentPage;
					if (temp_currentpage == -1 && stateMode.StateModeSelect != StateModeSelectEnum::IdtSelection && penetrate.select == false)
					{
						if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) == 1)
						{
							EndPptShow();

							ChangeStateModeToSelection();
						}
					}
					else if (temp_currentpage == -1)
					{
						EndPptShow();
					}
					else
					{
						SetForegroundWindow(ppt_show);

						NextPptSlides(temp_currentpage);
						pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v = RGBA(200, 200, 200, 255);

						std::chrono::high_resolution_clock::time_point KeyboardInteractionManipulated = std::chrono::high_resolution_clock::now();
						while (1)
						{
							if (!KeyBoradDown[VK_LBUTTON]) break;

							if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - KeyboardInteractionManipulated).count() >= 400)
							{
								temp_currentpage = PptInfoState.CurrentPage;
								if (temp_currentpage == -1 && stateMode.StateModeSelect != StateModeSelectEnum::IdtSelection && penetrate.select == false)
								{
									if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) == 1)
									{
										EndPptShow();

										ChangeStateModeToSelection();
									}
									break;
								}
								else if (temp_currentpage != -1)
								{
									NextPptSlides(temp_currentpage);
									pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v = RGBA(200, 200, 200, 255);
								}
							}

							this_thread::sleep_for(chrono::milliseconds(15));
						}
					}

					pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v = RGBA(250, 250, 250, 160);
					hiex::flushmessage_win32(EM_MOUSE, ppt_window);

					POINT pt;
					GetCursorPos(&pt);
					last_x = pt.x, last_y = pt.y;
				}
			}
			else if (PptInfoStateBuffer.TotalPage != -1) pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v = RGBA(250, 250, 250, 160);

			// ������Ϣ
			if (m.message == WM_MOUSEWHEEL)
			{
				// ��һҳ
				if (m.wheel <= -120)
				{
					int temp_currentpage = PptInfoState.CurrentPage;
					if (temp_currentpage == -1 && stateMode.StateModeSelect != StateModeSelectEnum::IdtSelection && penetrate.select == false)
					{
						if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) == 1)
						{
							EndPptShow();

							ChangeStateModeToSelection();
						}
					}
					else if (temp_currentpage == -1)
					{
						EndPptShow();
					}
					else
					{
						SetForegroundWindow(ppt_show);

						NextPptSlides(temp_currentpage);
						//pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v = RGBA(200, 200, 200, 255);
					}

					//pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v = RGBA(250, 250, 250, 160);
					hiex::flushmessage_win32(EM_MOUSE, ppt_window);
				}
				// ��һҳ
				else
				{
					SetForegroundWindow(ppt_show);

					PreviousPptSlides();
					//pptUiRoundRectWidget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v = RGBA(200, 200, 200, 255);

					//pptUiRoundRectWidgetTarget[PptUiRoundRectWidgetID::BottomSide_RightPageWidget_NextPage].FillColor.v = RGBA(250, 250, 250, 160);
					hiex::flushmessage_win32(EM_MOUSE, ppt_window);
				}
			}
		}
		else
		{
			last_x = -1, last_y = -1;

			hiex::flushmessage_win32(EM_MOUSE, ppt_window);
			this_thread::sleep_for(chrono::milliseconds(500));
		}
	}
}
void PPTLinkageMain()
{
	threadStatus[L"PptDraw"] = true;

	thread(GetPptState).detach();
	thread(PptInfo).detach();

	thread DrawControlWindowThread(PptDraw);
	DrawControlWindowThread.detach();
	thread ControlManipulationThread(PptInteract);
	ControlManipulationThread.detach();

	while (!offSignal) this_thread::sleep_for(chrono::milliseconds(500));

	int i = 1;
	for (; i <= 5; i++)
	{
		if (!threadStatus[L"GetPptState"] && !threadStatus[L"PptDraw"] && !threadStatus[L"PptInfo"]) break;
		this_thread::sleep_for(chrono::milliseconds(500));
	}

	threadStatus[L"PptDraw"] = false;
}

// --------------------------------------------------
// �������

// DesktopDrawpadBlocker ���
void StartDesktopDrawpadBlocker()
{
	if (ddbSetList.DdbEnable)
	{
		// ���� json
		{
			if (_waccess((StringToWstring(globalPath) + L"PlugIn\\DDB\\interaction_configuration.json").c_str(), 0) == 0) DdbReadSetting();

			ddbSetList.hostPath = GetCurrentExePath();
			if (ddbSetList.DdbEnhance)
			{
				ddbSetList.mode = 0;
				ddbSetList.restartHost = true;
			}
			else
			{
				ddbSetList.mode = 1;
				ddbSetList.restartHost = true;
			}
		}

		// ���� EXE
		if (_waccess((StringToWstring(globalPath) + L"PlugIn\\DDB\\DesktopDrawpadBlocker.exe").c_str(), 0) == -1)
		{
			if (_waccess((StringToWstring(globalPath) + L"PlugIn\\DDB").c_str(), 0) == -1)
			{
				error_code ec;
				filesystem::create_directories(StringToWstring(globalPath) + L"PlugIn\\DDB", ec);
			}
			ExtractResource((StringToWstring(globalPath) + L"PlugIn\\DDB\\DesktopDrawpadBlocker.exe").c_str(), L"EXE", MAKEINTRESOURCE(237));
		}
		else
		{
			string hash_sha256;
			{
				hashwrapper* myWrapper = new sha256wrapper();
				hash_sha256 = myWrapper->getHashFromFileW(StringToWstring(globalPath) + L"PlugIn\\DDB\\DesktopDrawpadBlocker.exe");
				delete myWrapper;
			}

			if (hash_sha256 != ddbSetList.DdbSHA256)
			{
				if (isProcessRunning((StringToWstring(globalPath) + L"PlugIn\\DDB\\DesktopDrawpadBlocker.exe").c_str()))
				{
					// ��Ҫ�رվɰ� DDB �����°汾

					DdbWriteSetting(true, true);
					for (int i = 1; i <= 20; i++)
					{
						if (!isProcessRunning((StringToWstring(globalPath) + L"PlugIn\\DDB\\DesktopDrawpadBlocker.exe").c_str()))
							break;
						this_thread::sleep_for(chrono::milliseconds(500));
					}
				}
				ExtractResource((StringToWstring(globalPath) + L"PlugIn\\DDB\\DesktopDrawpadBlocker.exe").c_str(), L"EXE", MAKEINTRESOURCE(237));
			}
		}

		// ��������������ʶ
		if (ddbSetList.DdbEnhance && _waccess((StringToWstring(globalPath) + L"PlugIn\\DDB\\start_up.signal").c_str(), 0) == -1)
		{
			std::ofstream file((StringToWstring(globalPath) + L"PlugIn\\DDB\\start_up.signal").c_str());
			file.close();
		}
		// �Ƴ�����������ʶ
		else if (!ddbSetList.DdbEnhance && _waccess((StringToWstring(globalPath) + L"PlugIn\\DDB\\start_up.signal").c_str(), 0) == 0)
		{
			error_code ec;
			filesystem::remove(StringToWstring(globalPath) + L"PlugIn\\DDB\\start_up.signal", ec);
		}

		// ���� DDB
		if (!isProcessRunning((StringToWstring(globalPath) + L"PlugIn\\DDB\\DesktopDrawpadBlocker.exe").c_str()))
		{
			DdbWriteSetting(true, false);
			ShellExecute(NULL, NULL, (StringToWstring(globalPath) + L"PlugIn\\DDB\\DesktopDrawpadBlocker.exe").c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
	}
	else if (_waccess((StringToWstring(globalPath) + L"PlugIn\\DDB").c_str(), 0) == 0)
	{
		error_code ec;
		filesystem::remove_all(StringToWstring(globalPath) + L"PlugIn\\DDB", ec);
	}
}