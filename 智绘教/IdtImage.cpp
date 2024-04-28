#include "IdtImage.h"

//drawpad����
IMAGE alpha_drawpad(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); //��ʱ����
IMAGE putout; //�������ϵ��ӵĿؼ�����
IMAGE tester(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); //ͼ�λ��ƻ���
IMAGE pptdrawpad(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); //PPT�ؼ�����

int recall_image_recond, recall_image_reference;
shared_mutex RecallImageManipulatedSm;
chrono::high_resolution_clock::time_point RecallImageManipulated;

tm RecallImageTm;
int RecallImagePeak = 0;
deque<RecallStruct> RecallImage;//����ջ

//������
IMAGE background(576, 386);
Graphics graphics(GetImageHDC(&background));

Bitmap* IMAGEToBitmap(IMAGE* easyXImage)
{
	if (!easyXImage || easyXImage->getwidth() <= 0 || easyXImage->getheight() <= 0) {
		return nullptr;
	}

	// ��ȡ EasyX ͼ�����Ϣ
	int width = easyXImage->getwidth();
	int height = easyXImage->getheight();
	int channels = 4;  // ���� EasyX ͼ��ʹ�� 32 λ ARGB ��ʽ

	// ���� GDI+ Bitmap
	Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	if (!bitmap) {
		return nullptr;
	}

	// ���� GDI+ Bitmap ������
	Gdiplus::BitmapData bitmapData;
	Gdiplus::Rect rect(0, 0, width, height);
	bitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);

	// �� EasyX ͼ�����ݸ��Ƶ� GDI+ Bitmap
	DWORD* srcData = GetImageBuffer(easyXImage);
	BYTE* destData = static_cast<BYTE*>(bitmapData.Scan0);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			// ÿ���������ĸ��ֽڣ��ֱ��� B, G, R, A
			destData[4 * (y * width + x) + 0] = GetRValue(srcData[(y * width + x)]);  // Blue
			destData[4 * (y * width + x) + 1] = GetGValue(srcData[(y * width + x)]);  // Green
			destData[4 * (y * width + x) + 2] = GetBValue(srcData[(y * width + x)]);  // Red
			destData[4 * (y * width + x) + 3] = (srcData[(y * width + x)] >> 24) & 0xFF;  // Alpha
		}
	}

	// ���� GDI+ Bitmap ������
	bitmap->UnlockBits(&bitmapData);

	return bitmap;
}
bool ImgCpy(IMAGE* tag, IMAGE* src)
{
	if (tag == NULL || src == NULL) return false;
	if (tag->getwidth() != src->getwidth() || tag->getheight() != src->getheight())
	{
		tag->Resize(src->getwidth(), src->getheight());
	}

	int width = src->getwidth();
	int height = src->getheight();
	DWORD* pSrc = GetImageBuffer(src);
	DWORD* pTag = GetImageBuffer(tag);

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			pTag[y * width + x] = pSrc[y * width + x];
		}
	}

	return true;
}