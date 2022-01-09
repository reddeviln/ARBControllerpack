#include "pch.h"
#include "TimerRadar.h"
#include <Mmsystem.h>
#include <mciapi.h>


//std::wstring thisdirectory;
std::string thisdirectory;
std::string thisdirnorm;
const int TIMER = 354864;
CRect Timer(100, 100, 300, 200);
int Timerleft, Timertop, Timerright, Timerbottom;
std::string timeremaining("0:00");
int timeremainseconds;
bool timervisible = true;
bool timerrunning = false;
//std::vector<std::wstring> times;
std::vector<std::wstring> times;
std::vector<int> timesint;
time_t endTime = 0;

TimerRadar::TimerRadar(std::string dir)
{
	/*using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;
	thisdirnorm = converter.to_bytes(dir);*/
	if(Timerleft !=0 && Timertop != 0 && Timerright != 0 && Timerbottom != 0)
		Timer.SetRect(Timerleft, Timertop, Timerright, Timerbottom);
	thisdirectory = dir;
	times.push_back(L"1:20");
	times.push_back(L"1:40");
	times.push_back(L"2:00");
	times.push_back(L"2:20");
	times.push_back(L"2:40");
	times.push_back(L"3:00");
	times.push_back(L"3:20");
	times.push_back(L"3:40");
	times.push_back(L"4:00");
	times.push_back(L"C");
	timesint.push_back(80);
	timesint.push_back(100);
	timesint.push_back(120);
	timesint.push_back(140);
	timesint.push_back(160);
	timesint.push_back(180);
	timesint.push_back(200);
	timesint.push_back(220);
	timesint.push_back(240);
	
}

TimerRadar::~TimerRadar()
{
}

void TimerRadar::OnAsrContentLoaded(bool Loaded)
{
	Timerleft = Timer.TopLeft().x;
	Timerright = Timer.BottomRight().x;
	Timertop = Timer.TopLeft().y;
	Timerbottom = Timer.BottomRight().y;
	const char * p_value;
	if ((p_value = GetDataFromAsr("Timerleft")) != NULL)
		Timerleft = atoi(p_value);
	if ((p_value = GetDataFromAsr("Timerright")) != NULL)
		Timerright = atoi(p_value);
	if ((p_value = GetDataFromAsr("Timertop")) != NULL)
		Timertop = atoi(p_value);
	if ((p_value = GetDataFromAsr("Timerbottom")) != NULL)
		Timerbottom = atoi(p_value);
	if ((p_value = GetDataFromAsr("Timervisible")) != NULL)
	{
		int temp = atoi(p_value);
		if (temp == 0)
			timervisible = false;
	}
	Timer.MoveToXY(Timerleft, Timertop);
}

void TimerRadar::OnAsrContentToBeClosed()
{
	
	SaveDataToAsr("Timerleft", "Left Position of Timer window", std::to_string(Timer.TopLeft().x).c_str());
	SaveDataToAsr("Timerright", "Right Position of Timer window", std::to_string(Timer.BottomRight().x).c_str());
	SaveDataToAsr("Timertop", "Top Position of Timer window", std::to_string(Timer.TopLeft().y).c_str());
	SaveDataToAsr("Timerbottom", "Bottom Position of Timer window", std::to_string(Timer.BottomRight().y).c_str());
	SaveDataToAsr("Timervisible", "Is Timer window visible", std::to_string(timervisible).c_str());
}

void TimerRadar::OnRefresh(HDC hDC, int Phase)
{


	if (Phase == EuroScopePlugIn::REFRESH_PHASE_AFTER_LISTS)
	{

		// Creating the gdi+ graphics
		Gdiplus::Graphics graphics(hDC);
		if (!timervisible)
		{
			return;
		}
		graphics.SetPageUnit(Gdiplus::UnitPixel);
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		//Gdiplus::Bitmap* image = Gdiplus::Bitmap::FromFile(path.c_str());
		Gdiplus::SolidBrush timerbrush(Gdiplus::Color::Gray);
		Gdiplus::SolidBrush blackbrush(Gdiplus::Color::Black);
		Gdiplus::SolidBrush orangebrush(Gdiplus::Color::Orange);
		Gdiplus::SolidBrush redbrush(Gdiplus::Color::Red);
		Gdiplus::Pen mypen(Gdiplus::Color::Black);
		graphics.FillRectangle(&timerbrush, CopyRect(Timer));
		auto topleft = Timer.TopLeft();
		auto height = Timer.Height();
		auto width = Timer.Width();
		auto bottomright = Timer.BottomRight();
		//draw line seperating timer from time options
		
		
		graphics.DrawLine(&mypen, (int)topleft.x, (int)topleft.y + height / 3,(int) bottomright.x, (int)topleft.y + height / 3);
		//draw option seperating lines
		for (int i = 1; i <= 5; i++)
		{
			graphics.DrawLine(&mypen, topleft.x + i * width / 5, int(topleft.y + height / 3), topleft.x + i * width / 5, bottomright.y);
		}
		graphics.DrawLine(&mypen, (int)topleft.x, (int)topleft.y + 2 * height / 3,(int) bottomright.x, (int)topleft.y + 2 * height / 3);
		AddScreenObject(TIMER, "timer", Timer, true, "");
		Gdiplus::FontFamily   fontFamily(L"Consolas");
		auto dpix = graphics.GetDpiX();
		auto dpiy = graphics.GetDpiY();
		Gdiplus::Font         fontoptions(&fontFamily, 15, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::Font         fontrem(&fontFamily, 25, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
		Gdiplus::RectF        rectFrem(topleft.x, topleft.y, width, height / 3);
		std::vector<Gdiplus::RectF> optionrects;
		

		for (int i = 1; i <= 5; i++)
		{
			for (int j = 1; j <= 2; j++)
			{
				optionrects.push_back(Gdiplus::RectF(topleft.x + (i - 1)*width / 5, topleft.y + j * height / 3, width / 5, height / 3));
			}
		}
		Gdiplus::StringFormat stringFormat = new Gdiplus::StringFormat();
		stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
		stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		timeremaining = (TimerRadar::GetCurrentTimeString(TimerRadar::GetSecondsRemaining())).str();
		std::wstring wtime(timeremaining.begin(), timeremaining.end());
		if (TimerRadar::GetSecondsRemaining() < 20 && TimerRadar::GetSecondsRemaining() >5)
			graphics.DrawString(wtime.c_str(), -1, &fontrem, rectFrem, &stringFormat, &orangebrush);
		else if (timerrunning && TimerRadar::GetSecondsRemaining() <= 5)
			graphics.DrawString(wtime.c_str(), -1, &fontrem, rectFrem, &stringFormat, &redbrush);
		else 
			graphics.DrawString(wtime.c_str(), -1, &fontrem, rectFrem, &stringFormat, &blackbrush);
		for (int ij = 0; ij < 10; ij++)
		{
			graphics.DrawString(times.at(ij).c_str(), -1, &fontoptions, optionrects.at(ij), &stringFormat, &blackbrush);
		}

		graphics.ReleaseHDC(hDC);

		return;
	}
}

void TimerRadar::OnClickScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, int Button)
{
	if (ObjectType != TIMER || Button != EuroScopePlugIn::BUTTON_LEFT)
		return;
	auto topleft = Timer.TopLeft();
	auto height = Timer.Height();
	auto width = Timer.Width();
	auto bottomright = Timer.BottomRight();
	std::vector<CRect> optionrects;
	for (int i = 1; i <= 5; i++)
	{
		for (int j = 1; j <= 2; j++)
		{
			optionrects.push_back(CRect(topleft.x+(i-1)*width/5,topleft.y+j*height/3,topleft.x+i*width/5,topleft.y+(j+1)*height/3));
		}
	}
	for (auto rect : optionrects)
	{
		if (rect.PtInRect(Pt))
		{
			auto found = std::find(optionrects.begin(), optionrects.end(), rect);
			auto index = found - optionrects.begin();
			auto timestarted = times.at(index);
			if (timestarted == L"C")
			{
				timeremaining = "0:00";
				TimerRadar::cancelTimer();
			}
				
			else
			{
				timeremaining = std::string(timestarted.begin(),timestarted.end());
				TimerRadar::startTimer(timesint.at(index));
			}
				
		}
	}
}

void TimerRadar::OnMoveScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, bool Released)
{
	if (ObjectType != TIMER)
		return;
	Timer.MoveToXY(Pt.x, Pt.y);

	RequestRefresh();
}

void TimerRadar::OnOverScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area)
{
}

bool TimerRadar::OnCompileCommand(const char * sCommandLine)
{
	if (strcmp(".hidetimer", sCommandLine) == 0)
	{
		timervisible = false;
		return true;
	}
	if (strcmp(".showtimer", sCommandLine) == 0)
	{
		timervisible = true;
		return true;
	}
	if (strcmp(".resettimer", sCommandLine) == 0)
	{
		Timer.MoveToXY(100, 100);
		return true;
	}
	return false;
}

void TimerRadar::OnRadarTargetPositionUpdate(EuroScopePlugIn::CRadarTarget RadarTarget)
{
}

void TimerRadar::OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan FlightPlan)
{
}

std::string TimerRadar::GetBottomLine(const char * Callsign)
{
	return std::string();
}

void TimerRadar::OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area)
{
}

std::wstring TimerRadar::to_wstring(const std::string& str) {
	return std::filesystem::path(str).wstring();
}
void TimerRadar::startTimer(int sec)
{
	timeremainseconds = sec;
	endTime = time(0) + sec;
	timerrunning = true;
}
void TimerRadar::cancelTimer()
{
	if (endTime != NULL) {
		
		endTime = NULL;
		timerrunning = false;
	}
}
int TimerRadar::GetSecondsRemaining()
{
	if (endTime == NULL) {
		timerrunning = false;
		return 0;
	}

	int timeDiff = static_cast<int>(difftime(endTime, time(0)));
	if (timeDiff <= 0) {
		std::string mystring = thisdirnorm + "Alarm.wav";
		PlaySound(TEXT(mystring.c_str()), NULL, SND_ASYNC|SND_FILENAME);
		TimerRadar::cancelTimer();
		return 0;
	}

	return timeDiff;
}
std::stringstream TimerRadar::GetCurrentTimeString(int secondsRemaining)
{
	int minutes = secondsRemaining / 60;
	int seconds = secondsRemaining % 60;

	
	std::stringstream wss;

	if (seconds < 10) {
		wss << minutes << ":0" << seconds << '\0';
	}
	else {
		wss << minutes << ":" << seconds << '\0';
	}

	return wss;
}