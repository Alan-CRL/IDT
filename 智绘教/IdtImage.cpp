#include "IdtImage.h"

//drawpad����
IMAGE drawpad(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); //������
IMAGE alpha_drawpad(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); //��ʱ����
IMAGE last_drawpad; //��һ�λ������ݣ�����������
IMAGE putout; //�������ϵ��ӵĿؼ�����
IMAGE tester(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); //ͼ�λ��ƻ���
IMAGE pptdrawpad(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); //PPT�ؼ�����

IMAGE test_sign[5];

deque<RecallStruct> RecallImage;//����ջ

//������
IMAGE background(576, 386);
Graphics graphics(GetImageHDC(&background));