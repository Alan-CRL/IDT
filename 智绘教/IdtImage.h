#pragma once
#include "IdtMain.h"

//drawpad����
extern IMAGE alpha_drawpad; //��ʱ����
extern IMAGE putout; //�������ϵ��ӵĿؼ�����
extern IMAGE tester; //ͼ�λ��ƻ���
extern IMAGE pptdrawpad; //PPT�ؼ�����

extern IMAGE test_sign[5];

extern int recall_image_recond, recall_image_reference;
extern shared_mutex RecallImageManipulatedSm;
extern chrono::high_resolution_clock::time_point RecallImageManipulated;
extern tm RecallImageTm;
struct RecallStruct
{
	IMAGE img;
	std::map<std::pair<int, int>, bool> extreme_point;
	int type;
	pair<int, int> recond;
};
extern deque<RecallStruct> RecallImage;//����ջ

//������
extern IMAGE background;
extern Graphics graphics;

Bitmap* IMAGEToBitmap(IMAGE* easyXImage);