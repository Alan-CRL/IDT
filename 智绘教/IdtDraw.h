#pragma once
#include "IdtMain.h"

// ѡɫ�����
struct BrushColorChooseStruct
{
	int x, y;
	int last_x, last_y;
};
extern BrushColorChooseStruct BrushColorChoose;
extern IMAGE ColorPaletteImg;
extern shared_mutex ColorPaletteSm;

enum DrawModeSelectEnum
{
	IdtSelection,
	IdtPen,
	IdtEraser
};
enum PenModeSelectEnum
{
	IdtPenBrush1,
	IdtPenHighlighter1
};

class DrawModeClass
{
public:
	DrawModeClass()
	{
		DrawModeSelect = DrawModeSelectEnum::IdtSelection;
		DrawModeSelectEcho = DrawModeSelectEnum::IdtSelection;

		Pen.ModeSelect = PenModeSelectEnum::IdtPenBrush1;
		Pen.Brush1.width = 3;
		Pen.Brush1.color = RGBA(50, 30, 181, 255);
	}

public:
	DrawModeSelectEnum DrawModeSelect;
	DrawModeSelectEnum DrawModeSelectEcho;

	struct
	{
		PenModeSelectEnum ModeSelect;

		struct
		{
			int width;
			COLORREF color;
		}Brush1;
		struct
		{
			int width;
			COLORREF color;
		}Highlighter1;
	}Pen;
};
extern DrawModeClass drawMode;

struct brushStruct
{
	int width, mode;
	COLORREF color, primary_colour;
}; //����
extern brushStruct brush;

// TODO �Ͼɲ���
struct penetrateStruct
{
	bool select;
}; //���ڴ�͸
extern penetrateStruct penetrate;
struct testStruct
{
	bool select;
}; //����
extern testStruct test;

struct plug_in_RandomRollCallStruct
{
	int select;
};
extern plug_in_RandomRollCallStruct plug_in_RandomRollCall;
struct FreezeFrameStruct
{
	bool select;
	int mode;

	bool update;
};
extern FreezeFrameStruct FreezeFrame;

// ����׼�� sRGB ֵת��Ϊ���� RGB ֵ
double sRGBToLinear(double s);
// �����������
double computeLuminance(COLORREF color);
// ����������ɫ�ĶԱȶ�
double computeContrast(COLORREF color1, COLORREF color2);

//������ɫ������������͸���Ȳ�Ϊ0�����ص㣬��Ϊָ����ɫ��
void ChangeColor(IMAGE& img, COLORREF color);
// ��������COLORREF��ɫ֮��ļ�Ȩ����
double color_distance(COLORREF c1, COLORREF c2);
// ���巴ɫ����
COLORREF InvertColor(COLORREF color, bool alpha_enable = false);
//����ͼ�񵽱���
void saveImageToPNG(IMAGE img, const char* filename, bool alpha = true, int compression_level = 9);
void SaveHBITMAPToPNG(HBITMAP hBitmap, const char* filename);
void saveImageToJPG(IMAGE img, const char* filename, int quality = 100);
//�Ƚ�ͼ��
bool CompareImagesWithBuffer(IMAGE* img1, IMAGE* img2);
//����ͼ����벻ӵ��ȫ͸�����أ�������ȫ͸�����ص�͸��������Ϊ1��
void SetAlphaToOne(IMAGE* pImg);

//���ܻ�ͼ����

extern map<pair<int, int>, bool> extreme_point;
extern shared_mutex ExtremePointSm;
//extern map<pair<Point, Point >, bool> extreme_line;
double pointToLineDistance(Point lineStart, Point lineEnd, Point p);
double pointToLineSegmentDistance(Point lineStart, Point lineEnd, Point p);
bool isLine(vector<Point> points, int tolerance, std::chrono::high_resolution_clock::time_point start);