#pragma once
#include "IdtMain.h"

#include "IdtWindow.h"
#include "IdtTime.h"
#include "IdtOther.h"

//�����������
void CrashedHandler();
//�����Զ�����
wstring get_domain_name(wstring url);
wstring convertToHttp(const wstring& url);

extern int AutomaticUpdateStep;
void AutomaticUpdate();

//����ע�� + ����Ǽ�
//�û�������󱻷���
//void NetUpdate();