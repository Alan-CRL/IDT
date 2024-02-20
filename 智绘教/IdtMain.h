/*
 * @file		IdtMain.h
 * @brief		�ǻ����Ŀ����ͷ�ļ�
 * @note		������������ͷ�ļ��Լ�������ı���
 *
 * @envir		VisualStudio 2022 | MSVC 143 | .NET Framework 3.5 | EasyX_20230723 | Windows 11
 * @site		https://github.com/Alan-CRL/Intelligent-Drawing-Teaching
 *
 * @author		Alan-CRL
 * @qq			2685549821
 * @email		alan-crl@foxmail.com
*/

// ������ڵ�λ�� IdtMain.cpp�������ļ��Ľ��ͽ����Ժ��д��Ŀǰ�����ƶ�Ӧ����
// ������ʾ��.NET �汾Ĭ��Ϊ .NET Framework 4.0 �����Ҫ�� .NET Framework 3.5�����������鿴 PptCOM��
// �״α�����Ҫȷ�� .NET Framework �汾Ϊ 4.0�������һ����ִ�� λ�� PptCOM �� <�л� .NET Framework ָ��>

#pragma once
#define IDT_RELEASE

#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

//������
#include <iostream>									// �ṩ��׼���������
#include <thread>									// �ṩ�߳���ص���ͺ���
#include <unordered_map>							// �ṩ���ڹ�ϣ��map����
#include <utility>									// �ṩһЩʵ�ó���ģ����
#include <windows.h>								// Windows API�Ļ���ͷ�ļ�

//ͼ����
#include "HiEasyX.h"								// HiEasyX ��չ��
#include <gdiplus.h>								// GDI+��ͼ�ӿ�

//COM��
#include <atlbase.h>								// ATL(Active Template Library)�Ļ���ͷ�ļ�
#include <comutil.h>								// COM��ʵ�ó���
#include <ole2.h>									// OLE�⣨����������Ƕ�룩
#include <rtscom.h>									// RTS ���ؿ�
#include <rtscom_i.c>								// RTS ���ؿ� COM �ӿ�
#include <comdef.h>									// �ṩCOM�����ʵ�ó���

//�ļ���
#include <filesystem>								// �ļ�ϵͳ��
#include "json/reader.h"							// JSON��ȡ��
#include "json/value.h"								// JSONֵ������
#include "json/writer.h"							// JSONд���
#include "hashlib2plus/hashlibpp.h"					// ��ϣ��
#include "zip_utils/unzip.h"						// ��ѹ����

//������
#include <typeinfo>									// �ṩtype_info�࣬��������ʱ���Ͳ�ѯ
#include <psapi.h>									// �ṩ����״̬API
#include <netlistmgr.h>								// ���������б����
#include <wininet.h>								// �ṩInternet�ĺ����ͺ�
#include <intrin.h>									// �ṩ��CPU��ص��ڲ�����
#include <regex>									// �ṩ������ʽ��ع���
#include <dwmapi.h>									// ���洰�ڹ�����API
#include <wbemidl.h>								// ����Windows����������
#include <versionhelpers.h>							// �ṩ�汾��������
#include <mutex>									// �ṩ��������ع���
#include <shared_mutex>								// �ṩ������������
//#include <spdlog/spdlog.h>						//�ṩ��־��¼����

#include "IdtKey.h" // ��������Կ�����ǲ���Դ��������Կ���Է���������ǵķ�������

//���ӿ�
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "wbemuuid.lib")

using namespace std;
using namespace Gdiplus;

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

#define HiBeginDraw() BEGIN_TASK()
#define HiEndDraw() END_TASK(); REDRAW_WINDOW()

#define Sleep(int) this_thread::sleep_for(chrono::milliseconds(int))

extern bool already;

extern wstring buildTime; //����ʱ��
extern string edition_date; //���򷢲�����
extern string edition_code; //����汾

extern wstring userid; //�û�ID���������кţ�
extern string global_path; //����ǰ·��

extern string server_feedback, server_code;
extern double server_updata_error, procedure_updata_error;
extern wstring server_updata_error_reason;

extern bool off_signal; //�ر�ָ��
extern map <wstring, bool> thread_status; //�߳�״̬����

//����ר��
#ifndef IDT_RELEASE
void Test();
void Testi(int t);
void Testw(wstring t);
#endif