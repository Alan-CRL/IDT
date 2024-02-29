#include "IdtPlug-in.h"

#include "IdtDraw.h"
#include "IdtFloating.h"
#include "IdtMagnification.h"
#include "IdtRts.h"
#include "IdtText.h"
#include "IdtWindow.h"

ppt_infoStruct ppt_info = { -1, -1 };
ppt_imgStruct ppt_img = { false };
ppt_info_stayStruct ppt_info_stay = { -1, -1 };

IMAGE ppt_icon[5];
bool SeewoCamera; //ϣ����Ƶչ̨�Ƿ���

//PPT����

#import "PptCOM.tlb" // C# ��� PptCOM ��Ŀ��
using namespace PptCOM;

shared_mutex PPTManipulatedSm;
std::chrono::high_resolution_clock::time_point PPTManipulated;

wstring LinkTest()
{
	wstring ret = L"COM��(.dll) �����ڣ��ҷ������ش��󣬷���ֵ������";

	if (_waccess((string_to_wstring(global_path) + L"PptCOM.dll").c_str(), 4) == 0)
	{
		IPptCOMServerPtr pto;
		try
		{
			_com_util::CheckError(pto.CreateInstance(_uuidof(PptCOMServer)));
			ret = bstr_to_wstring(pto->LinkTest());
		}
		catch (_com_error& err)
		{
			ret = L"COM��(.dll) ���ڣ�COM�ɹ���ʼ������C++��COM�ӿ��쳣��" + wstring(err.ErrorMessage());
		}
	}
	else
	{
		wchar_t absolutePath[_MAX_PATH];

		if (_wfullpath(absolutePath, L"PptCOM.dll", _MAX_PATH) != NULL)
		{
			ret = L"COM��(.dll) �����ڣ�Ԥ�ڵ���Ŀ¼Ϊ��\"" + string_to_wstring(global_path) + L"PptCOM.dll\"";
		}
		else ret = L"COM��(.dll) �����ڣ�Ԥ�ڵ���Ŀ¼����ʧ��";
	}

	return ret;
}
wstring IsPptDependencyLoaded()
{
	wstring ret = L"PPT ��������쳣���ҷ������ش��󣬷���ֵ������";

	IPptCOMServerPtr pto;
	try
	{
		_com_util::CheckError(pto.CreateInstance(_uuidof(PptCOMServer)));
		ret = L"COM�ӿ�������C#��ⷴ����Ϣ��" + bstr_to_wstring(pto->IsPptDependencyLoaded());
	}
	catch (_com_error& err)
	{
		ret = L"COM�ӿ��쳣��" + wstring(err.ErrorMessage());
	}

	return ret;
}
HWND GetPptShow()
{
	HWND hWnd = NULL;

	IPptCOMServerPtr pto;
	try
	{
		_com_util::CheckError(pto.CreateInstance(_uuidof(PptCOMServer)));

		_variant_t result = pto->GetPptHwnd();
		hWnd = (HWND)result.llVal;

		return hWnd;
	}
	catch (_com_error)
	{
	}

	return NULL;
}
wstring GetPptTitle()
{
	wstring ret = L"";

	IPptCOMServerPtr pto;
	try
	{
		_com_util::CheckError(pto.CreateInstance(_uuidof(PptCOMServer)));
		ret = bstr_to_wstring(pto->slideNameIndex());

		return ret;
	}
	catch (_com_error)
	{
	}

	return ret;
}
bool EndPptShow()
{
	IPptCOMServerPtr pto;
	try
	{
		_com_util::CheckError(pto.CreateInstance(_uuidof(PptCOMServer)));
		pto->EndSlideShow();

		return true;
	}
	catch (_com_error)
	{
	}

	return false;
}

int NextPptSlides(int check)
{
	IPptCOMServerPtr pto;
	try
	{
		_com_util::CheckError(pto.CreateInstance(_uuidof(PptCOMServer)));
		return pto->NextSlideShow(check);
	}
	catch (_com_error)
	{
	}
	return -1;
}
int PreviousPptSlides()
{
	IPptCOMServerPtr pto;
	try
	{
		_com_util::CheckError(pto.CreateInstance(_uuidof(PptCOMServer)));
		return pto->PreviousSlideShow();
	}
	catch (_com_error)
	{
	}
	return -1;
}

//ppt �ؼ�
void DrawControlWindow()
{
	thread_status[L"DrawControlWindow"] = true;

	IMAGE ppt_background = CreateImageColor(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), RGBA(0, 0, 0, 0), true);
	Graphics graphics(GetImageHDC(&ppt_background));
	//ppt���ڳ�ʼ��
	{
		DisableResizing(ppt_window, true);//��ֹ��������
		SetWindowLong(ppt_window, GWL_STYLE, GetWindowLong(ppt_window, GWL_STYLE) & ~WS_CAPTION);//���ر�����
		SetWindowPos(ppt_window, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_DRAWFRAME | SWP_NOACTIVATE);
		SetWindowLong(ppt_window, GWL_EXSTYLE, WS_EX_TOOLWINDOW);//����������
	}

	// ����BLENDFUNCTION�ṹ��
	BLENDFUNCTION blend;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = 255; // ����͸���ȣ�0Ϊȫ͸����255Ϊ��͸��
	blend.AlphaFormat = AC_SRC_ALPHA; // ʹ��Դͼ���alphaͨ��
	HDC hdcScreen = GetDC(NULL);
	// ����UpdateLayeredWindow�������´���
	POINT ptSrc = { 0,0 };
	SIZE sizeWnd = { ppt_background.getwidth(),ppt_background.getheight() };
	POINT ptDst = { 0,0 }; // ���ô���λ��
	UPDATELAYEREDWINDOWINFO ulwi = { 0 };
	ulwi.cbSize = sizeof(ulwi);
	ulwi.hdcDst = hdcScreen;
	ulwi.pptDst = &ptDst;
	ulwi.psize = &sizeWnd;
	ulwi.pptSrc = &ptSrc;
	ulwi.crKey = RGB(255, 255, 255);
	ulwi.pblend = &blend;
	ulwi.dwFlags = ULW_ALPHA;

	do
	{
		Sleep(10);
		::SetWindowLong(ppt_window, GWL_EXSTYLE, ::GetWindowLong(ppt_window, GWL_EXSTYLE) | WS_EX_LAYERED);
	} while (!(::GetWindowLong(ppt_window, GWL_EXSTYLE) & WS_EX_LAYERED));
	do
	{
		Sleep(10);
		::SetWindowLong(ppt_window, GWL_EXSTYLE, ::GetWindowLong(ppt_window, GWL_EXSTYLE) | WS_EX_NOACTIVATE);
	} while (!(::GetWindowLong(ppt_window, GWL_EXSTYLE) & WS_EX_NOACTIVATE));

	magnificationWindowReady++;
	for (int for_i = 1; !off_signal; for_i = 2)
	{
		int currentSlides = ppt_info.currentSlides;
		int totalSlides = ppt_info.totalSlides;

		if (totalSlides != -1)
		{
			SetImageColor(ppt_background, RGBA(0, 0, 0, 0), true);

			//��෭ҳ
			{
				hiex::EasyX_Gdiplus_FillRoundRect(5, (float)GetSystemMetrics(SM_CYSCREEN) - 60 + 5, 50, 50, 20, 20, SET_ALPHA(RGB(100, 100, 100), 255), SET_ALPHA(WHITE, 160), 1, true, SmoothingModeHighQuality, &ppt_background);
				hiex::TransparentImage(&ppt_background, 5 + 5, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 5, &ppt_icon[1]);

				hiex::EasyX_Gdiplus_FillRoundRect(55 + 5, (float)GetSystemMetrics(SM_CYSCREEN) - 60 + 5, 70, 50, 20, 20, SET_ALPHA(RGB(100, 100, 100), 255), SET_ALPHA(WHITE, 160), 1, true, SmoothingModeHighQuality, &ppt_background);
				{
					wstring text = currentSlides == -1 ? L"-" : to_wstring(currentSlides);
					text += L"/";
					text += to_wstring(totalSlides);

					Gdiplus::Font gp_font(&HarmonyOS_fontFamily, Gdiplus::REAL(text.length() > 5 ? 16 : (text.length() > 3 ? 20 : 24)), FontStyleRegular, UnitPixel);
					SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGB(100, 100, 100), false));
					graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
					{
						pptwords_rect.left = (55 + 5);
						pptwords_rect.top = (GetSystemMetrics(SM_CYSCREEN) - 60 + 5) + 13;
						pptwords_rect.right = (55 + 5) + 70;
						pptwords_rect.bottom = (GetSystemMetrics(SM_CYSCREEN) - 60 + 5) + 40;
					}

					graphics.DrawString(text.c_str(), -1, &gp_font, hiex::RECTToRectF(pptwords_rect), &stringFormat, &WordBrush);
				}

				hiex::EasyX_Gdiplus_FillRoundRect(130 + 5, (float)GetSystemMetrics(SM_CYSCREEN) - 60 + 5, 50, 50, 20, 20, SET_ALPHA(RGB(100, 100, 100), 255), SET_ALPHA(WHITE, 160), 1, true, SmoothingModeHighQuality, &ppt_background);
				hiex::TransparentImage(&ppt_background, 130 + 5 + 5, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 5, &ppt_icon[2]);
			}
			//�м�ؼ�
			{
				hiex::EasyX_Gdiplus_FillRoundRect((float)GetSystemMetrics(SM_CXSCREEN) / 2 - 70, (float)GetSystemMetrics(SM_CYSCREEN) - 60 + 5, 140, 50, 20, 20, SET_ALPHA(RGB(100, 100, 100), 255), SET_ALPHA(WHITE, 160), 1, true, SmoothingModeHighQuality, &ppt_background);
				{
					hiex::TransparentImage(&ppt_background, GetSystemMetrics(SM_CXSCREEN) / 2 - 70 + 10, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 10, &ppt_icon[3]);

					Gdiplus::Font gp_font(&HarmonyOS_fontFamily, 20, FontStyleRegular, UnitPixel);
					SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGB(100, 100, 100), false));
					graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
					{
						pptwords_rect.left = (GetSystemMetrics(SM_CXSCREEN) / 2 - 70) + 40;
						pptwords_rect.top = (GetSystemMetrics(SM_CYSCREEN) - 60 + 5) + 13;
						pptwords_rect.right = (GetSystemMetrics(SM_CXSCREEN) / 2 - 70) + 140;
						pptwords_rect.bottom = (GetSystemMetrics(SM_CYSCREEN) - 60 + 5) + 40;
					}

					graphics.DrawString(L"������ӳ", -1, &gp_font, hiex::RECTToRectF(pptwords_rect), &stringFormat, &WordBrush);
				}
			}
			//�Ҳ෭ҳ
			{
				hiex::EasyX_Gdiplus_FillRoundRect((float)GetSystemMetrics(SM_CXSCREEN) - 185, (float)GetSystemMetrics(SM_CYSCREEN) - 60 + 5, 50, 50, 20, 20, SET_ALPHA(RGB(130, 130, 130), 255), SET_ALPHA(WHITE, 160), 1, true, SmoothingModeHighQuality, &ppt_background);
				hiex::TransparentImage(&ppt_background, GetSystemMetrics(SM_CXSCREEN) - 185 + 5, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 5, &ppt_icon[1]);

				hiex::EasyX_Gdiplus_FillRoundRect((float)GetSystemMetrics(SM_CXSCREEN) - 130, (float)GetSystemMetrics(SM_CYSCREEN) - 60 + 5, 70, 50, 20, 20, SET_ALPHA(RGB(130, 130, 130), 255), SET_ALPHA(WHITE, 160), 1, true, SmoothingModeHighQuality, &ppt_background);
				{
					wstring text = currentSlides == -1 ? L"-" : to_wstring(currentSlides);
					text += L"/";
					text += to_wstring(totalSlides);

					Gdiplus::Font gp_font(&HarmonyOS_fontFamily, Gdiplus::REAL(text.length() > 5 ? 16 : (text.length() > 3 ? 20 : 24)), FontStyleRegular, UnitPixel);
					SolidBrush WordBrush(hiex::ConvertToGdiplusColor(RGB(100, 100, 100), false));
					graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
					{
						pptwords_rect.left = (GetSystemMetrics(SM_CXSCREEN) - 130);
						pptwords_rect.top = (GetSystemMetrics(SM_CYSCREEN) - 60 + 5) + 13;
						pptwords_rect.right = (GetSystemMetrics(SM_CXSCREEN) - 130) + 70;
						pptwords_rect.bottom = (GetSystemMetrics(SM_CYSCREEN) - 60 + 5) + 40;
					}

					graphics.DrawString(text.c_str(), -1, &gp_font, hiex::RECTToRectF(pptwords_rect), &stringFormat, &WordBrush);
				}

				hiex::EasyX_Gdiplus_FillRoundRect((float)GetSystemMetrics(SM_CXSCREEN) - 55, (float)GetSystemMetrics(SM_CYSCREEN) - 60 + 5, 50, 50, 20, 20, SET_ALPHA(RGB(130, 130, 130), 255), SET_ALPHA(WHITE, 160), 1, true, SmoothingModeHighQuality, &ppt_background);
				hiex::TransparentImage(&ppt_background, GetSystemMetrics(SM_CXSCREEN) - 55 + 5, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 5, &ppt_icon[2]);
			}

			{
				ulwi.hdcSrc = GetImageHDC(&ppt_background);
				UpdateLayeredWindowIndirect(ppt_window, &ulwi);
			}

			SetWindowPos(ppt_window, floating_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			if (!IsWindowVisible(ppt_window))
			{
				ShowWindow(ppt_window, SW_SHOWNOACTIVATE);
				ppt_show = GetPptShow();

				std::wstringstream ss(GetPptTitle());
				getline(ss, ppt_title);
				getline(ss, ppt_software);

				if (ppt_software.find(L"WPS") != ppt_software.npos) ppt_software = L"WPS";
				else ppt_software = L"PowerPoint";

				if (!ppt_title_recond[ppt_title]) FreezePPT = true;
			}
		}
		else
		{
			if (IsWindowVisible(ppt_window))
			{
				ppt_img.is_save = false;
				ppt_img.is_saved.clear();
				ppt_img.image.clear();

				ShowWindow(ppt_window, SW_HIDE);
				ppt_show = NULL, ppt_software = L"";

				FreezePPT = false;
			}
		}

		Sleep(100);
	}

	thread_status[L"DrawControlWindow"] = false;
}
void ControlManipulation()
{
	ExMessage m;
	while (!off_signal)
	{
		if (ppt_info.totalSlides != -1)
		{
			hiex::getmessage_win32(&m, EM_MOUSE, ppt_window);

			//���
			{
				//��һҳ
				if (IsInRect(m.x, m.y, { 5, GetSystemMetrics(SM_CYSCREEN) - 60 + 5, 5 + 50, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 50 }))
				{
					if (m.lbutton)
					{
						SetForegroundWindow(ppt_show);

						std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
						PPTManipulated = std::chrono::high_resolution_clock::now();
						lock1.unlock();
						ppt_info_stay.CurrentPage = PreviousPptSlides();

						std::chrono::high_resolution_clock::time_point KeyboardInteractionManipulated = std::chrono::high_resolution_clock::now();
						while (1)
						{
							if (!KEY_DOWN(VK_LBUTTON)) break;
							if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - KeyboardInteractionManipulated).count() >= 400)
							{
								std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
								PPTManipulated = std::chrono::high_resolution_clock::now();
								lock1.unlock();

								ppt_info_stay.CurrentPage = PreviousPptSlides();
							}

							Sleep(15);
						}
					}

					/*
					if (m.lbutton)
					{
						int lx = m.x, ly = m.y;
						while (1)
						{
							ExMessage m = hiex::getmessage_win32(EM_MOUSE, ppt_window);
							if (IsInRect(m.x, m.y, { 5, GetSystemMetrics(SM_CYSCREEN) - 60 + 5, 5 + 50, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 50 }))
							{
								if (!m.lbutton)
								{
									std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
									PPTManipulated = std::chrono::high_resolution_clock::now();
									lock1.unlock();

									SetForegroundWindow(ppt_show);
									ppt_info_stay.CurrentPage = PreviousPptSlides();

									break;
								}
							}
							else
							{
								hiex::flushmessage_win32(EM_MOUSE, ppt_window);

								break;
							}
						}
					}*/
				}

				//��һҳ
				if (IsInRect(m.x, m.y, { 130 + 5, GetSystemMetrics(SM_CYSCREEN) - 60 + 5, 130 + 5 + 50, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 50 }))
				{
					if (m.lbutton)
					{
						int temp_currentpage = ppt_info_stay.CurrentPage;
						if (temp_currentpage == -1 && choose.select == false && penetrate.select == false)
						{
							if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) == 1)
							{
								std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
								PPTManipulated = std::chrono::high_resolution_clock::now();
								lock1.unlock();
								EndPptShow();

								brush.select = false;
								rubber.select = false;
								penetrate.select = false;
								choose.select = true;
							}
						}
						else if (temp_currentpage == -1)
						{
							std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
							PPTManipulated = std::chrono::high_resolution_clock::now();
							lock1.unlock();
							EndPptShow();
						}
						else
						{
							SetForegroundWindow(ppt_show);

							std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
							PPTManipulated = std::chrono::high_resolution_clock::now();
							lock1.unlock();
							ppt_info_stay.CurrentPage = NextPptSlides(temp_currentpage);

							std::chrono::high_resolution_clock::time_point KeyboardInteractionManipulated = std::chrono::high_resolution_clock::now();
							while (1)
							{
								if (!KEY_DOWN(VK_LBUTTON)) break;

								if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - KeyboardInteractionManipulated).count() >= 400)
								{
									temp_currentpage = ppt_info_stay.CurrentPage;
									if (temp_currentpage == -1 && choose.select == false && penetrate.select == false)
									{
										if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) == 1)
										{
											EndPptShow();

											brush.select = false;
											rubber.select = false;
											penetrate.select = false;
											choose.select = true;
										}
										break;
									}
									else if (temp_currentpage == -1) break;

									std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
									PPTManipulated = std::chrono::high_resolution_clock::now();
									lock1.unlock();
									ppt_info_stay.CurrentPage = NextPptSlides(temp_currentpage);
								}

								Sleep(15);
							}
						}
					}

					/*
					if (m.lbutton)
					{
						int lx = m.x, ly = m.y;
						while (1)
						{
							ExMessage m = hiex::getmessage_win32(EM_MOUSE, ppt_window);
							if (IsInRect(m.x, m.y, { 130 + 5, GetSystemMetrics(SM_CYSCREEN) - 60 + 5, 130 + 5 + 50, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 50 }))
							{
								if (!m.lbutton)
								{
									if (ppt_info_stay.CurrentPage == -1 && choose.select == false && penetrate.select == false)
									{
										if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) != 1) break;

										brush.select = false;
										rubber.select = false;
										penetrate.select = false;
										choose.select = true;
									}

									std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
									PPTManipulated = std::chrono::high_resolution_clock::now();
									lock1.unlock();

									SetForegroundWindow(ppt_show);
									ppt_info_stay.CurrentPage = NextPptSlides();

									break;
								}
							}
							else
							{
								hiex::flushmessage_win32(EM_MOUSE, ppt_window);

								break;
							}
						}
						hiex::flushmessage_win32(EM_MOUSE, ppt_window);
					}*/
				}
			}
			//�м�
			{
				//������ӳ
				if (IsInRect(m.x, m.y, { GetSystemMetrics(SM_CXSCREEN) / 2 - 70, GetSystemMetrics(SM_CYSCREEN) - 60 + 5, GetSystemMetrics(SM_CXSCREEN) / 2 - 70 + 140, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 50 }))
				{
					if (m.lbutton)
					{
						int lx = m.x, ly = m.y;
						while (1)
						{
							ExMessage m = hiex::getmessage_win32(EM_MOUSE, ppt_window);
							if (IsInRect(m.x, m.y, { GetSystemMetrics(SM_CXSCREEN) / 2 - 70, GetSystemMetrics(SM_CYSCREEN) - 60 + 5, GetSystemMetrics(SM_CXSCREEN) / 2 - 70 + 140, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 50 }))
							{
								if (!m.lbutton)
								{
									if (choose.select == false && penetrate.select == false)
									{
										if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) != 1) break;

										brush.select = false;
										rubber.select = false;
										penetrate.select = false;
										choose.select = true;
									}

									std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
									PPTManipulated = std::chrono::high_resolution_clock::now();
									lock1.unlock();

									EndPptShow();

									break;
								}
							}
							else
							{
								hiex::flushmessage_win32(EM_MOUSE, ppt_window);

								break;
							}
						}
						hiex::flushmessage_win32(EM_MOUSE, ppt_window);
					}
				}
			}
			//�Ҳ�
			{
				//��һҳ
				if (IsInRect(m.x, m.y, { GetSystemMetrics(SM_CXSCREEN) - 185, GetSystemMetrics(SM_CYSCREEN) - 60 + 5, GetSystemMetrics(SM_CXSCREEN) - 185 + 50, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 50 }))
				{
					if (m.lbutton)
					{
						SetForegroundWindow(ppt_show);

						std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
						PPTManipulated = std::chrono::high_resolution_clock::now();
						lock1.unlock();
						ppt_info_stay.CurrentPage = PreviousPptSlides();

						std::chrono::high_resolution_clock::time_point KeyboardInteractionManipulated = std::chrono::high_resolution_clock::now();
						while (1)
						{
							if (!KEY_DOWN(VK_LBUTTON)) break;
							if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - KeyboardInteractionManipulated).count() >= 400)
							{
								std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
								PPTManipulated = std::chrono::high_resolution_clock::now();
								lock1.unlock();

								ppt_info_stay.CurrentPage = PreviousPptSlides();
							}

							Sleep(15);
						}
					}
				}

				//��һҳ
				if (IsInRect(m.x, m.y, { GetSystemMetrics(SM_CXSCREEN) - 55, GetSystemMetrics(SM_CYSCREEN) - 60 + 5, GetSystemMetrics(SM_CXSCREEN) - 55 + 50, GetSystemMetrics(SM_CYSCREEN) - 60 + 5 + 50 }))
				{
					if (m.lbutton)
					{
						int temp_currentpage = ppt_info_stay.CurrentPage;
						if (temp_currentpage == -1 && choose.select == false && penetrate.select == false)
						{
							if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) == 1)
							{
								std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
								PPTManipulated = std::chrono::high_resolution_clock::now();
								lock1.unlock();
								EndPptShow();

								brush.select = false;
								rubber.select = false;
								penetrate.select = false;
								choose.select = true;
							}
						}
						else if (temp_currentpage == -1)
						{
							std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
							PPTManipulated = std::chrono::high_resolution_clock::now();
							lock1.unlock();
							EndPptShow();
						}
						else
						{
							SetForegroundWindow(ppt_show);

							std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
							PPTManipulated = std::chrono::high_resolution_clock::now();
							lock1.unlock();
							ppt_info_stay.CurrentPage = NextPptSlides(temp_currentpage);

							std::chrono::high_resolution_clock::time_point KeyboardInteractionManipulated = std::chrono::high_resolution_clock::now();
							while (1)
							{
								if (!KEY_DOWN(VK_LBUTTON)) break;

								if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - KeyboardInteractionManipulated).count() >= 400)
								{
									temp_currentpage = ppt_info_stay.CurrentPage;
									if (temp_currentpage == -1 && choose.select == false && penetrate.select == false)
									{
										if (MessageBox(floating_window, L"��ǰ���ڻ���ģʽ��������ӳ������ջ������ݡ�\n\n������ӳ��", L"�ǻ�̾���", MB_OKCANCEL | MB_ICONWARNING | MB_SYSTEMMODAL) == 1)
										{
											EndPptShow();

											brush.select = false;
											rubber.select = false;
											penetrate.select = false;
											choose.select = true;
										}
										break;
									}
									else if (temp_currentpage == -1) break;

									std::unique_lock<std::shared_mutex> lock1(PPTManipulatedSm);
									PPTManipulated = std::chrono::high_resolution_clock::now();
									lock1.unlock();
									ppt_info_stay.CurrentPage = NextPptSlides(temp_currentpage);
								}

								Sleep(15);
							}
						}
					}
				}
			}

			hiex::flushmessage_win32(EM_MOUSE, ppt_window);
		}
		else Sleep(100);
	}
}

//��ȡ��ǰҳ���
int GetCurrentPage()
{
	int currentSlides = -1;

	IPptCOMServerPtr pto;
	try
	{
		_com_util::CheckError(pto.CreateInstance(_uuidof(PptCOMServer)));
		currentSlides = pto->currentSlideIndex();
	}
	catch (_com_error)
	{
	}

	return currentSlides;
}
//��ȡ��ҳ��
int GetTotalPage()
{
	int totalSlides = -1;

	IPptCOMServerPtr pto;
	try
	{
		_com_util::CheckError(pto.CreateInstance(_uuidof(PptCOMServer)));
		totalSlides = pto->totalSlideIndex();
		//wstring temp = bstr_to_wstring(pto->totalSlideIndex());
		//Testw(temp);
	}
	catch (_com_error)
	{
	}

	return totalSlides;
}
void ppt_state()
{
	thread_status[L"ppt_state"] = true;
	while (!off_signal)
	{
		if (ppt_info_stay.CurrentPage == -1) ppt_info_stay.TotalPage = GetTotalPage();

		if (ppt_info_stay.TotalPage != -1) ppt_info_stay.CurrentPage = GetCurrentPage();
		else ppt_info_stay.CurrentPage = -1;

		if (ppt_info_stay.TotalPage == -1 && !off_signal) for (int i = 0; i <= 30 && !off_signal; i++) Sleep(100);
		else if (!off_signal)
		{
			for (int i = 0; i <= 30 && !off_signal; i++)
			{
				Sleep(100);

				std::shared_lock<std::shared_mutex> lock1(PPTManipulatedSm);
				bool ret = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - PPTManipulated).count() <= 3000;
				lock1.unlock();
				if (ret) break;
			}
		}
	}
	thread_status[L"ppt_state"] = false;
}

//��������
//�ر�AIClass��ϣ�ְװ�5����
HWND FindWindowByStrings(const std::wstring& className, const std::wstring& windowTitle, const std::wstring& style, int width, int height)
{
	HWND hwnd = NULL;
	while ((hwnd = FindWindowEx(NULL, hwnd, NULL, NULL)) != NULL)
	{
		TCHAR classNameBuffer[1024];
		GetClassName(hwnd, classNameBuffer, 1024);
		if (_tcsstr(classNameBuffer, className.c_str()) == NULL) continue;

		TCHAR title[1024];
		GetWindowText(hwnd, title, 1024);
		if (_tcsstr(title, windowTitle.c_str()) == NULL) continue;

		if (windowTitle.length() == 0)
		{
			if (wstring(title) == windowTitle && to_wstring(GetWindowLong(hwnd, GWL_STYLE)) == style)
			{
				if (width && height)
				{
					RECT rect{};
					DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT));
					int twidth = rect.right - rect.left, thwight = rect.bottom - rect.top;

					HDC hdc = GetDC(NULL);
					int horizontalDPI = GetDeviceCaps(hdc, LOGPIXELSX);
					int verticalDPI = GetDeviceCaps(hdc, LOGPIXELSY);
					ReleaseDC(NULL, hdc);
					float scale = (horizontalDPI + verticalDPI) / 2.0f / 96.0f;

					if (abs(width * scale - twidth) <= 1 && abs(height * scale - thwight) <= 1) return hwnd;
				}
				else return hwnd;
			}
		}
		else if (to_wstring(GetWindowLong(hwnd, GWL_STYLE)) == style) return hwnd;
	}
	return NULL;
}
void black_block()
{
	thread_status[L"black_block"] = true;
	while (!off_signal)
	{
		HWND ai_class = FindWindowByStrings(L"UIIrregularWindow", L"UIIrregularWindow", L"-1811939328");
		if (ai_class != NULL) PostMessage(ai_class, WM_CLOSE, 0, 0);

		HWND Seewo_Whiteboard = FindWindowByStrings(L"HwndWrapper[EasiNote;;", L"", L"369623040", 550, 200);
		if (Seewo_Whiteboard != NULL) PostMessage(Seewo_Whiteboard, WM_CLOSE, 0, 0);

		HWND Seewo_Camera = FindWindowByStrings(L"HwndWrapper[EasiCamera.exe;;", L"ϣ����Ƶչ̨", L"386400256");
		if (Seewo_Camera != NULL) SeewoCamera = true;
		else SeewoCamera = false;

		for (int i = 1; i <= 5 && !off_signal; i++) Sleep(1000);
	}
	thread_status[L"black_block"] = false;
}