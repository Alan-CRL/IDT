#include "IdtDraw.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb-master/stb_image_write.h"

BrushColorChooseStruct BrushColorChoose = { 0,0,-1,-1 };
IMAGE ColorPaletteImg;
shared_mutex ColorPaletteSm;

penetrateStruct penetrate; //���ڴ�͸
chooseStruct choose; //ѡ��
brushStruct brush = { false,4,1,RGBA(0,0,0,0),RGBA(0,0,0,0),4,40 }; //����
rubberStruct rubber; //��Ƥ
testStruct test; //����

plug_in_RandomRollCallStruct plug_in_RandomRollCall;
FreezeFrameStruct FreezeFrame;

// ����׼�� sRGB ֵת��Ϊ���� RGB ֵ
double sRGBToLinear(double s) {
	if (s <= 0.04045) return s / 12.92;
	return pow((s + 0.055) / 1.055, 2.4);
}
// �����������
double computeLuminance(COLORREF color) {
	double R = sRGBToLinear(GetRValue(color) / 255.0);
	double G = sRGBToLinear(GetGValue(color) / 255.0);
	double B = sRGBToLinear(GetBValue(color) / 255.0);
	return 0.2126 * R + 0.7152 * G + 0.0722 * B;
}
// ����������ɫ�ĶԱȶ�
double computeContrast(COLORREF color1, COLORREF color2) {
	double L1 = computeLuminance(color1);
	double L2 = computeLuminance(color2);

	if (L1 > L2) return (L1 + 0.05) / (L2 + 0.05);
	return (L2 + 0.05) / (L1 + 0.05);
}

//������ɫ������������͸���Ȳ�Ϊ0�����ص㣬��Ϊָ����ɫ��
void ChangeColor(IMAGE& img, COLORREF color)
{
	// ��ȡͼ��Ŀ�Ⱥ͸߶�
	int width = img.getwidth();
	int height = img.getheight();

	// ��ȡͼ��Ļ�����ָ��
	DWORD* pBuf = GetImageBuffer(&img);

	// ����ÿ�����ص�
	for (int i = 0; i < width * height; i++)
	{
		// ��ȡ��ǰ���ص����ɫֵ
		DWORD pixel = pBuf[i];

		// ��ȡ��ǰ���ص��͸���ȣ�alpha ֵ��
		DWORD alpha = pixel >> 24;

		// ���Դͼ����ɫ���޸���ɫ��ͬ���޸ģ�ֱ��������
		if (alpha != 0)
		{
			if (pixel == BGR(color)) return;
		}
		else continue;

		// ��COLORREFת��ΪRGB
		DWORD rgb = ((color & 0xFF) << 16) | (color & 0xFF00) | ((color & 0xFF0000) >> 16);

		// ����͸���ȵ�����ɫ������
		DWORD r = (rgb & 0xFF0000) >> 16;
		DWORD g = (rgb & 0x00FF00) >> 8;
		DWORD b = (rgb & 0x0000FF);
		r = r * alpha / 255;
		g = g * alpha / 255;
		b = b * alpha / 255;
		rgb = (r << 16) | (g << 8) | b;

		// ���������ɫֵ��͸���Ⱥϲ�
		DWORD newPixel = (alpha << 24) | (rgb & 0x00FFFFFF);

		// ���µ���ɫֵд�뻺����
		pBuf[i] = newPixel;
	}
}
// ��������COLORREF��ɫ֮��ļ�Ȩ����
double color_distance(COLORREF c1, COLORREF c2) {
	// ��ȡ������ɫ����
	int r1 = GetRValue(c1);
	int g1 = GetGValue(c1);
	int b1 = GetBValue(c1);
	int r2 = GetRValue(c2);
	int g2 = GetGValue(c2);
	int b2 = GetBValue(c2);

	// ���ø���������Ȩ��
	double wr = 0.3;
	double wg = 0.59;
	double wb = 0.11;

	// �����Ȩƽ����
	double sum = wr * (r1 - r2) * (r1 - r2) +
		wg * (g1 - g2) * (g1 - g2) +
		wb * (b1 - b2) * (b1 - b2);

	// ��ƽ��������
	return sqrt(sum);
}
// ���巴ɫ����
COLORREF InvertColor(COLORREF color, bool alpha_enable)
{
	// ��ȡ��ɫ����
	BYTE red = GetRValue(color);
	BYTE green = GetGValue(color);
	BYTE blue = GetBValue(color);
	BYTE alpha;
	if (alpha_enable) alpha = (color >> 24) & 0xff;
	else alpha = 255;

	// ��ɫ����
	red = 255 - red;
	green = 255 - green;
	blue = 255 - blue;

	// �ϲ���ɫ������͸����
	COLORREF inverted = red | (green << 8) | (blue << 16) | (alpha << 24);

	// ���ط�ɫ
	return inverted;
}
//����ͼ�񵽱���
void saveImageToPNG(IMAGE img, const char* filename, bool alpha, int compression_level)
{
	int width = img.getwidth();
	int height = img.getheight();

	// Get the image buffer of the IMAGE object
	DWORD* pMem = GetImageBuffer(&img);

	if (alpha)
	{
		unsigned char* data = new unsigned char[width * height * 4];
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				COLORREF color = pMem[y * width + x];
				data[(y * width + x) * 4 + 0] = GetBValue(color); // Swap the red and blue components
				data[(y * width + x) * 4 + 1] = GetGValue(color);
				data[(y * width + x) * 4 + 2] = GetRValue(color); // Swap the red and blue components
				data[(y * width + x) * 4 + 3] = color >> 24;
			}
		}

		stbi_write_png_compression_level = compression_level;
		stbi_write_png(filename, width, height, 4, data, width * 4);
		delete[] data;
	}
	else
	{
		unsigned char* data = new unsigned char[width * height * 3];
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				COLORREF color = pMem[y * width + x];
				data[(y * width + x) * 3 + 0] = GetBValue(color); // Swap the red and blue components
				data[(y * width + x) * 3 + 1] = GetGValue(color);
				data[(y * width + x) * 3 + 2] = GetRValue(color); // Swap the red and blue components
			}
		}

		stbi_write_png_compression_level = compression_level;
		stbi_write_png(filename, width, height, 3, data, width * 3);
		delete[] data;
	}
}
void SaveHBITMAPToPNG(HBITMAP hBitmap, const char* filename)
{
	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);

	int width = bmp.bmWidth;
	int height = bmp.bmHeight;

	HDC hdc = GetDC(NULL);
	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hBitmap);

	unsigned char* pixels = new unsigned char[width * height * 4];
	BITMAPINFOHEADER bih;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = width;
	bih.biHeight = -height;
	bih.biPlanes = 1;
	bih.biBitCount = 32;
	bih.biCompression = BI_RGB;
	bih.biSizeImage = 0;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	GetDIBits(memDC, hBitmap, 0, height, pixels, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

	stbi_flip_vertically_on_write(1);
	stbi_write_png(filename, width, height, 4, pixels, width * 4);

	delete[] pixels;

	SelectObject(memDC, oldBmp);
	DeleteDC(memDC);
}
void saveImageToJPG(IMAGE img, const char* filename, int quality)
{
	int width = img.getwidth();
	int height = img.getheight();
	DWORD* pMem = GetImageBuffer(&img);

	unsigned char* data = new unsigned char[width * height * 3];

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			COLORREF color = pMem[y * width + x];
			data[(y * width + x) * 3 + 0] = GetBValue(color);
			data[(y * width + x) * 3 + 1] = GetGValue(color);
			data[(y * width + x) * 3 + 2] = GetRValue(color);
		}
	}
	stbi_write_jpg(filename, width, height, 3, data, quality);
	delete[] data;
}
//�Ƚ�ͼ��
bool CompareImagesWithBuffer(IMAGE* img1, IMAGE* img2)
{
	// ����Ⱥ͸߶�
	if (img1->getwidth() != img2->getwidth() || img1->getheight() != img2->getheight()) return false;

	DWORD* pBuf1 = GetImageBuffer(img1);
	DWORD* pBuf2 = GetImageBuffer(img2);

	int dataSize = img1->getwidth() * img1->getheight() * sizeof(DWORD);

	return memcmp(pBuf1, pBuf2, dataSize) == 0;
}
//����ͼ����벻ӵ��ȫ͸�����أ�������ȫ͸�����ص�͸��������Ϊ1��
void SetAlphaToOne(IMAGE* pImg) // pImg�ǻ�ͼ�豸ָ��
{
	// ��ȡͼ�񻺳���ָ��
	DWORD* pBuffer = GetImageBuffer(pImg);
	// ��ȡͼ���Ⱥ͸߶�
	int width = pImg->getwidth();
	int height = pImg->getheight();
	// ����ÿ����
	for (int i = 0; i < width * height; i++)
	{
		// ��ȡ��ǰ�����ɫֵ��ARGB��ʽ��
		DWORD color = pBuffer[i];
		// ���͸����Ϊ0��������Ϊ1
		if ((color >> 24) == 0)
		{
			// �����8λ��Ϊ1������������λ����
			color = (color & 0x00FFFFFF) | 0x01000000;
			// ���޸ĺ����ɫֵд�ص�ͼ�񻺳���
			pBuffer[i] = color;
		}
	}
}

//���ܻ�ͼ����
map<pair<int, int>, bool> extreme_point;
//map<pair<Point, Point >, bool> extreme_line;
double pointToLineDistance(Point lineStart, Point lineEnd, Point p) {
	if (lineStart.X == lineEnd.X) {
		return abs(p.X - lineStart.X);
	}

	double a = double(lineEnd.Y - lineStart.Y) / (lineEnd.X - lineStart.X);
	double b = lineStart.Y - a * lineStart.X;

	return abs(a * p.X - p.Y + b) / sqrt(a * a + 1);
}
double pointToLineSegmentDistance(Point lineStart, Point lineEnd, Point p)
{
	double x1 = lineStart.X;
	double y1 = lineStart.Y;
	double x2 = lineEnd.X;
	double y2 = lineEnd.Y;
	double x3 = p.X;
	double y3 = p.Y;

	if (x1 == x2) {
		if (y3 >= min(y1, y2) && y3 <= max(y1, y2)) {
			return abs(x3 - x1);
		}
		else {
			return min(sqrt(pow(x3 - x1, 2) + pow(y3 - y1, 2)), sqrt(pow(x3 - x2, 2) + pow(y3 - y2, 2)));
		}
	}

	double a = (y2 - y1) / (x2 - x1);
	double b = y1 - a * x1;
	double x4 = (a * (y3 - b) + x3) / (a * a + 1);
	if (x4 >= min(x1, x2) && x4 <= max(x1, x2)) {
		return abs(a * x3 - y3 + b) / sqrt(a * a + 1);
	}
	else {
		return min(sqrt(pow(x3 - x1, 2) + pow(y3 - y1, 2)), sqrt(pow(x3 - x2, 2) + pow(y3 - y2, 2)));
	}
}
bool isLine(vector<Point> points, int tolerance, std::chrono::high_resolution_clock::time_point start)
{
	int n = points.size();
	if (n < 2) return false;
	if (n == 2) return true;

	double last_distance = 0;
	int trend = 0; //1Զ�� 2����
	int fluctuate = 0;

	for (int i = 1; i < n - 1; i += 2)
	{
		if (i % 10 == 0 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() > 100) return false;

		double distance = pointToLineSegmentDistance(points[0], points[n - 1], points[i]);
		if (distance > tolerance) return false;

		if (distance > last_distance)
		{
			if (trend != 1 && abs(distance - last_distance) >= double(tolerance) / 6.0)
			{
				fluctuate++;
				trend = 1;
				last_distance = distance;
			}
		}
		else if (distance < last_distance)
		{
			if (trend != 2 && abs(distance - last_distance) >= double(tolerance) / 6.0)
			{
				fluctuate++;
				trend = 2;
				last_distance = distance;
			}
		}
	}

	if (fluctuate <= 5) return true;
	else return false;
}