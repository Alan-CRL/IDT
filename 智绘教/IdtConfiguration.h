#pragma once
#include "IdtMain.h"

struct SetListStruct
{
	SetListStruct()
	{
		StartUp = 0, CreateLnk = false;
		BrushRecover = true, RubberRecover = false;
		RubberMode = 0;

		IntelligentDrawing = true, SmoothWriting = true;

		SetSkinMode = 0, SkinMode = 1;

		UpdateChannel = "LTS";
	}

	int StartUp; bool CreateLnk;
	bool BrushRecover, RubberRecover;
	int RubberMode;

	bool IntelligentDrawing, SmoothWriting;

	int SetSkinMode, SkinMode;

	string UpdateChannel;
};
extern SetListStruct setlist;

struct DdbSetListStruct
{
	DdbSetListStruct()
	{
		DdbEnable = true;
		DdbEnhance = false;

		DdbEdition = L"20240510a";
		DdbSHA256 = "ff7824a53ea1a303f4f9755a635397062743d6deb99c38a74c5d40c391a6ea66";

		// -----

		sleepTime = 5000;

		mode = 1;
		hostPath = L"";
		restartHost = true;

		memset(InterceptWindow, true, sizeof(InterceptWindow));
	}

	bool DdbEnable, DdbEnhance;
	wstring DdbEdition;
	string DdbSHA256;

	// Ddb �����ļ�

	int sleepTime;

	int mode; // 0 ����ģʽ 1 ����������Ϳ����͹ر� 2 ����������ر�
	wstring hostPath;
	bool restartHost; // restartHost�������޶���ģʽ�����������򱻹رպ����ص���������Ĵ��ں�������������

	bool InterceptWindow[10];
	/* InterceptWindow �б�
	 *
	 * 0 AiClass ����������
	 * 1 ϣ�ְװ� ����������
	 * 2 ϣ��Ʒ�Σ�������������PPT�ؼ���
	 * 3 ϣ��Ʒ�� ���滭��
	 * 4 ϣ��PPTС����
	 *
	 */
};
extern DdbSetListStruct ddbSetList;
bool DdbReadSetting();
bool DdbWriteSetting(bool change, bool close);