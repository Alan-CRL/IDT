#pragma once
#include "IdtMain.h"

//drawpad����
extern IMAGE drawpad; //������
extern IMAGE alpha_drawpad; //��ʱ����
extern IMAGE last_drawpad; //��һ�λ������ݣ�����������
extern IMAGE putout; //�������ϵ��ӵĿؼ�����
extern IMAGE tester; //ͼ�λ��ƻ���
extern IMAGE pptdrawpad; //PPT�ؼ�����

extern IMAGE test_sign[5];

struct RecallStruct
{
	IMAGE img;
	map<pair<int, int>, bool> extreme_point;
};
extern deque<RecallStruct> RecallImage;//����ջ

//������
extern IMAGE background;
extern Graphics graphics;