#pragma once
#include "IdtMain.h"

#include "IdtConfiguration.h"
#include "IdtOther.h"
#include "IdtSetting.h"
#include "IdtTime.h"
#include "IdtWindow.h"

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