#pragma once
#include "UAEControllerpack2.h"
#define NOMINMAX
#include "windows.h"
using std::min;
using std::max;
#include <gdiplus.h>
#include <filesystem>
#include<future>
#include <chrono>
#include<sstream>
#pragma comment(lib, "Winmm.lib")

#pragma comment(lib,"gdiplus.lib")


class TimerRadar :public EuroScopePlugIn::CRadarScreen
{
public:
	TimerRadar(std::string dir);
	virtual ~TimerRadar();


	static std::wstring to_wstring(const std::string& str);
	//---OnAsrContentLoaded--------------------------------------------

	virtual void OnAsrContentLoaded(bool Loaded);

	//---OnAsrContentToBeSaved------------------------------------------

	virtual void OnAsrContentToBeClosed();
	inline static Gdiplus::Rect CopyRect(CRect &rect)
	{
		return Gdiplus::Rect(rect.left, rect.top, rect.Width(), rect.Height());
	}
	void startTimer(int sec);
	int GetSecondsRemaining();
	std::stringstream GetCurrentTimeString(int secondsRemaining);
	void cancelTimer();
	//---OnRefresh------------------------------------------------------

	virtual void OnRefresh(HDC hDC, int Phase);

	//---OnClickScreenObject-----------------------------------------

	virtual void OnClickScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, int Button);

	//---OnMoveScreenObject---------------------------------------------

	virtual void OnMoveScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, bool Released);

	//---OnOverScreenObject---------------------------------------------

	virtual void OnOverScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area);

	//---OnCompileCommand-----------------------------------------

	virtual bool OnCompileCommand(const char * sCommandLine);

	
	//---OnRadarTargetPositionUpdate---------------------------------------------

	virtual void OnRadarTargetPositionUpdate(EuroScopePlugIn::CRadarTarget RadarTarget);

	//---OnFlightPlanDisconnect---------------------------------------------

	virtual void OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan FlightPlan);

	

	//---GetBottomLine---------------------------------------------

	virtual std::string GetBottomLine(const char * Callsign);

	//---LineIntersect---------------------------------------------

	/*inline virtual POINT getIntersectionPoint(POINT lineA, POINT lineB, POINT lineC, POINT lineD) {

		double x1 = lineA.x;
		double y1 = lineA.y;
		double x2 = lineB.x;
		double y2 = lineB.y;

		double x3 = lineC.x;
		double y3 = lineC.y;
		double x4 = lineD.x;
		double y4 = lineD.y;

		POINT p = { 0, 0 };

		double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
		if (d != 0) {
			double xi = ((x3 - x4) * (x1 * y2 - y1 * x2) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
			double yi = ((y3 - y4) * (x1 * y2 - y1 * x2) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;

			p = { (int)xi, (int)yi };

		}
		return p;
	}*/

	//---OnFunctionCall-------------------------------------------------

	virtual void OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area);

	//---OnAsrContentToBeClosed-----------------------------------------

	
	
};

