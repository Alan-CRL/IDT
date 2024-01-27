#include "IdtRts.h"

int uRealTimeStylus;

bool touchDown = false;							// ��ʾ�����豸�Ƿ񱻰���
int touchNum = 0;								// ������ĵ������
unordered_map<LONG, pair<int, int>> PreviousPointPosition;	//�����ٶȼ���

unordered_map<LONG, double> TouchSpeed;
unordered_map<LONG, TouchMode> TouchPos;
vector<LONG> TouchList;

deque<TouchInfo> TouchTemp;

LONG TouchCnt = 0;
unordered_map<LONG, LONG> TouchPointer;
shared_mutex PointPosSm, TouchSpeedSm, PointListSm, PointTempSm;

IRealTimeStylus* g_pRealTimeStylus = NULL;
IStylusSyncPlugin* g_pSyncEventHandlerRTS = NULL;

IRealTimeStylus* CreateRealTimeStylus(HWND hWnd)
{
	// Check input argument
	if (hWnd == NULL)
	{
		//ASSERT(hWnd && L"CreateRealTimeStylus: invalid argument hWnd");
		return NULL;
	}

	// Create RTS object
	IRealTimeStylus* pRealTimeStylus = NULL;
	HRESULT hr = CoCreateInstance(CLSID_RealTimeStylus, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pRealTimeStylus));
	if (FAILED(hr))
	{
		//ASSERT(SUCCEEDED(hr) && L"CreateRealTimeStylus: failed to CoCreateInstance of RealTimeStylus");
		return NULL;
	}

	// Attach RTS object to a window
	hr = pRealTimeStylus->put_HWND((HANDLE_PTR)hWnd);
	if (FAILED(hr))
	{
		//ASSERT(SUCCEEDED(hr) && L"CreateRealTimeStylus: failed to set window handle");
		pRealTimeStylus->Release();
		return NULL;
	}

	// Register RTS object for receiving multi-touch input.
	IRealTimeStylus3* pRealTimeStylus3 = NULL;
	hr = pRealTimeStylus->QueryInterface(&pRealTimeStylus3);
	if (FAILED(hr))
	{
		//ASSERT(SUCCEEDED(hr) && L"CreateRealTimeStylus: cannot access IRealTimeStylus3");
		pRealTimeStylus->Release();
		return NULL;
	}
	hr = pRealTimeStylus3->put_MultiTouchEnabled(TRUE);
	if (FAILED(hr))
	{
		//ASSERT(SUCCEEDED(hr) && L"CreateRealTimeStylus: failed to enable multi-touch");
		pRealTimeStylus->Release();
		pRealTimeStylus3->Release();
		return NULL;
	}
	pRealTimeStylus3->Release();

	return pRealTimeStylus;
}
bool EnableRealTimeStylus(IRealTimeStylus* pRealTimeStylus)
{
	// Check input arguments
	if (pRealTimeStylus == NULL)
	{
		//ASSERT(pRealTimeStylus && L"EnableRealTimeStylus: invalid argument RealTimeStylus");
		return NULL;
	}

	// Enable RTS object
	HRESULT hr = pRealTimeStylus->put_Enabled(TRUE);
	if (FAILED(hr))
	{
		//ASSERT(SUCCEEDED(hr) && L"EnableRealTimeStylus: failed to enable RealTimeStylus");
		return false;
	}

	return true;
}

void RTSSpeed()
{
	int x, y;
	double speed;

	clock_t tRecord = 0;
	while (!off_signal)
	{
		for (int i = 0; i < touchNum; i++)
		{
			std::shared_lock<std::shared_mutex> lock1(PointPosSm);
			x = TouchPos[TouchList[i]].pt.x;
			y = TouchPos[TouchList[i]].pt.y;
			lock1.unlock();

			std::shared_lock<std::shared_mutex> lock2(TouchSpeedSm);
			if (PreviousPointPosition[TouchList[i]].first == -1 && PreviousPointPosition[TouchList[i]].second == -1) PreviousPointPosition[TouchList[i]].first = x, PreviousPointPosition[TouchList[i]].second = y, speed = 1;
			else speed = (TouchSpeed[TouchList[i]] + sqrt(pow(x - PreviousPointPosition[TouchList[i]].first, 2) + pow(y - PreviousPointPosition[TouchList[i]].second, 2))) / 2;
			lock2.unlock();

			std::unique_lock<std::shared_mutex> lock3(TouchSpeedSm);
			TouchSpeed[TouchList[i]] = speed;
			lock3.unlock();

			PreviousPointPosition[TouchList[i]].first = x, PreviousPointPosition[TouchList[i]].second = y;
		}

		if (tRecord)
		{
			int delay = 1000 / 20 - (clock() - tRecord);
			if (delay > 0) std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		}
		tRecord = clock();
	}
}