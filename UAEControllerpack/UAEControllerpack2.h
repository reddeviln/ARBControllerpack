#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <vector>
#include "csv.h"
#include "loguru.hpp"
#include <filesystem>
#include <sstream>
#include <set>
#include <string>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <regex>
#include "resource.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <unordered_map>
#include <utility>
#include <random>
#include "TimerRadar.h"
#include <stdexcept>
#define INT_PART 4
#define DEC_PART 3
#pragma warning(disable : 4996)
class Waypoint {
	//stores one navdata waypoint
public:
	std::string m_name;
	//first entry: airway name, second entry neighbor(s) on that airway
	std::unordered_map<std::string, std::vector<std::string>> m_connections;
	//constructor
	Waypoint(std::string name)
	{
		m_name = name;
	}
	void addConnection(std::string airway, std::string neighbor)
	{
		auto found = m_connections.find(airway);
		if (found == m_connections.end())
		{
			std::vector<std::string> neighborp;
			neighborp.push_back(neighbor);
			m_connections.emplace(std::make_pair(airway,neighborp));
		}
		else
		{
			std::vector<std::string> neighborp = found->second;
			m_connections.erase(found);
			neighborp.push_back(neighbor);
			m_connections.emplace(std::make_pair(airway, neighborp));
		}
	}
	std::vector<std::string> getNextPointNameOnAirway(std::string airway)
	{
		std::vector<std::string> mypoints;
		auto found = m_connections.find(airway);
		if (found == m_connections.end())
			mypoints.push_back("ERROR");
		else
		{
			mypoints = found->second;
		}
		return mypoints;
	}
	bool operator==(const Waypoint& rhs)
	{
		if (this->m_name == rhs.m_name)
			return true;
		else return false;
	}
	bool operator<(const Waypoint& rhs)
	{
		return std::lexicographical_compare(m_name.begin(), m_name.end(), rhs.m_name.begin(), rhs.m_name.end());
	}
	bool operator<(const Waypoint& rhs) const
	{
		return std::lexicographical_compare(m_name.begin(), m_name.end(), rhs.m_name.begin(), rhs.m_name.end());
	}
private:

};

class Fixes {
	//stores all waypoints
public:

	Fixes() {};
	void add_fix(Waypoint mypoint)
	{
		if (all_fixes.empty())
		{
			all_fixes.emplace(std::make_pair(mypoint.m_name, mypoint));
			return;
		}
		auto found = all_fixes.find(mypoint.m_name);
		if (found != all_fixes.end())
			all_fixes.erase(found);
		all_fixes.emplace(std::make_pair(mypoint.m_name, mypoint));
	}
	Waypoint find_waypoint(std::string name)
		//finds a waypoint in the list by its name using binary search
	{
		auto found = all_fixes.find(name);
		if (found == all_fixes.end()) 
		{
			return Waypoint("ERROR");;
		}
		return found->second;
	}
	
private:
	std::unordered_map<std::string,Waypoint> all_fixes;
};

class CTOTData
	//this class is used as a storage. Each aircraft that gets a ctot assigned will be put in a CTOTData object. It contains the flightplan the CTOT and TOBT a switch if it was manually assigned
{
public:
	CTOTData() {}
	EuroScopePlugIn::CFlightPlan flightplan;
	CTime CTOT, TOBT;
	int sequence;
	bool manual = false;
	bool operator==(const CTOTData& rhs)
		//overwriting the == and < operator to make use of the STL algorithms for sorting and finding lateron
	{
		if ((this->flightplan.GetCallsign() == rhs.flightplan.GetCallsign()) && this->CTOT == rhs.CTOT && this->sequence == rhs.sequence && this->TOBT == rhs.TOBT)
		{
			return true;
		}
		return false;
	}
	bool operator<(const CTOTData& rhs)
	{
		if (this->CTOT < rhs.CTOT)
		{
			return true;
		}
		return false;
	}
	static bool test()
	{
		return true;
	}

};

void WayPointNotFound(std::string name, std::string dep, std::string dest);
void AirwayWaypointConnectionNotFound(std::string pointname, std::string airwayname, std::string dep, std::string dest);
std::vector<Waypoint> parseATSPointsFromString(std::string route, std::string dep, std::string dest);

class RouteTo
	//This class stores the different standard routes to a destination icao.
{
public:
	std::string mDEPICAO, mDestICAO, mEvenOdd, mLevelR, mRoute;
	std::vector<Waypoint> points;
	RouteTo(std::string DepICAO, std::string DestICAO, std::string evenodd, std::string LevelR, std::string Route)
	{
		mDEPICAO = DepICAO;
		mDestICAO = DestICAO;
		mEvenOdd = evenodd;
		mLevelR = LevelR;
		mRoute = Route;
		points = parseATSPointsFromString(Route, DepICAO, DestICAO);
	}

	bool isCruiseValid(int Flightlevel)
	{
		if (this->mEvenOdd == "ALL")
			return true;
		bool returnval = false;
		if (Flightlevel > 40000)
		{
			if (this->mLevelR.empty())
			{
				if (this->mEvenOdd == "ODD")
				{
					if ((Flightlevel / 1000) % 4 == 1) return true;
					else return false;
				}
				if (this->mEvenOdd == "EVEN")
				{
					if ((Flightlevel / 1000) % 4 == 3) return true;
					else return false;
				}
			}
			else
			{

				if (this->mLevelR.at(0) == '<')
				{
					std::string restr = this->mLevelR.substr(1, 3);
					if (this->mEvenOdd == "ODD")
					{
						if (((Flightlevel / 1000) % 4 == 1) && Flightlevel <= std::stoi(restr)) return true;
						else return false;
					}
					if (this->mEvenOdd == "EVEN")
					{
						if (((Flightlevel / 1000) % 4 == 3) && Flightlevel <= std::stoi(restr) * 100) return true;
						else return false;
					}
				}
				else
				{
					int restr = std::stoi(this->mLevelR);
					return (Flightlevel == restr * 100);
				}
			}
		}
		else {
			if (this->mLevelR.empty())
			{
				if (this->mEvenOdd == "ODD")
				{
					if ((Flightlevel / 1000) % 2 == 1) return true;
					else return false;
				}
				if (this->mEvenOdd == "EVEN")
				{
					if ((Flightlevel / 1000) % 2 == 0) return true;
					else return false;
				}
			}
			else
			{

				if (this->mLevelR.at(0) == '<')
				{
					std::string restr = this->mLevelR.substr(1, 3);
					if (this->mEvenOdd == "ODD")
					{
						if (((Flightlevel / 1000) % 2 == 1) && Flightlevel <= std::stoi(restr) * 100) return true;
						else return false;
					}
					if (this->mEvenOdd == "EVEN")
					{
						if (((Flightlevel / 1000) % 2 == 0) && Flightlevel <= std::stoi(restr) * 100) return true;
						else return false;
					}
				}
				else
				{
					int restr = std::stoi(this->mLevelR);
					return (Flightlevel == restr * 100);
				}
			}
			return returnval;
		}
		return returnval;
		
	}
	bool isRouteValid(std::string Route)
	{
		std::vector<Waypoint> filedPoints = parseATSPointsFromString(Route, mDEPICAO, mDestICAO);
		std::string waypointNameShould, waypointNameIs;
		if (points.size() > filedPoints.size()) 
		{
			return false;				
		}
		auto check = std::mismatch(points.begin(), points.end(), filedPoints.begin());
		waypointNameIs = check.second->m_name;
		waypointNameShould = check.first->m_name;
		return points.end() == check.first;
	}
	static bool test()
	{
		return true;
	}
	std::string makeAirwaysUnique(std::string Route)
	{
		std::string buf;                 // Have a buffer string
		std::stringstream ss(Route);       // Insert the string into a stream
		std::vector<std::string> tokens; // Create vector to hold our words

		while (ss >> buf)
			tokens.push_back(buf);
		auto tokenscopy = tokens;
		tokens = RemoveDuplicatesInVector(tokens);
		std::string result;
		for (auto temp : tokens)
		{

			result += temp;
			result += " ";
		}
		return result;
	}
	std::vector<std::string> RemoveDuplicatesInVector(std::vector<std::string> vec)
	{
		std::set<std::string> values;
		//vec.erase(std::remove_if(vec.begin(), vec.end(), [&](const std::string & value) { return !values.insert(value).second; }), vec.end());
		std::vector<std::string> vec2 = vec;
		for (auto value : vec)
		{
			auto check = values.find(value);
			if (values.end() == check)
				values.insert(value);
			else
			{
				auto check1 = std::find(vec2.begin(), vec2.end(), *check);
				if (check1 != vec2.end() && *(check1 + 2) == *check)
				{
					vec2.erase(std::remove(vec2.begin(), check1 + 2, *(check1 + 1)));
					auto check2 = std::find(vec2.begin(), vec2.end(), *check);
					vec2.erase(std::remove(vec2.begin(), check2 + 1, *check2));
				}
			}
		}
		vec2.shrink_to_fit();
		return vec2;
	}
	std::string findAndReplaceAll(std::string data, std::string toSearch, std::string replaceStr)
	{
		// Get the first occurrence
		size_t pos = data.find(toSearch);
		// Repeat till end is reached
		while (pos != std::string::npos)
		{
			// Replace this occurrence of Sub String
			data.replace(pos, toSearch.size(), replaceStr);
			// Get the next occurrence from the current position
			pos = data.find(toSearch, pos + replaceStr.size());
		}
		return data;
	}

};

class RouteData
	//this Class holds all RouteTos
{
public:
	RouteData() {}
	std::vector<RouteTo> Routes;
	std::vector<std::string> icaos;

	std::vector<RouteTo> getDatafromICAO(std::string icao)
	{
		std::vector<RouteTo> routes;
		for (auto temp : Routes)
		{
			if (icao == temp.mDestICAO)
				routes.push_back(temp);
		}
		return routes;
	}
};

class Stand
{
	//this class stores all information regarding one particular stand
public:
	std::string number;
	double mlat, mLong;
	std::string mICAO;
	EuroScopePlugIn::CPosition position;
	std::string mAirlinecode;
	std::string mNeighbor1;
	std::string mNeighbor2;
	bool isEmpty = true;
	bool isAssigned = false;
	char mSize;
	bool isInFlytampa = false;
	Stand(std::string StandNumber, std::string lat, std::string Long, std::string airlinecode, std::string neighbor1, std::string neighbor2, std::string size, std::string flytampa, std::string airport)
	{
		number = StandNumber;
		mlat = Str2LatLong(lat);
		mLong = Str2LatLong(Long);
		position = EuroScopePlugIn::CPosition();
		position.m_Latitude = mlat;
		position.m_Longitude = mLong;
		mNeighbor1 = neighbor1;
		mNeighbor2 = neighbor2;
		mICAO = airport;
		mSize = size.at(0);
		mAirlinecode = airlinecode;
		if (flytampa == "yes")
		{
			isInFlytampa = true;
		}
		else if (flytampa == "no")
		{
			isInFlytampa = false;
		}
	}

	double Str2LatLong(std::string coord)

	{
		std::vector<double> splitted;
		std::string delimiter = "-";
		size_t pos = 0;
		std::string token;
		while ((pos = coord.find(delimiter)) != std::string::npos) {
			token = coord.substr(0, pos);
			switch (token.at(0))
			{
			case 'N':
			case 'E':
				token = token.substr(1, token.length() - 1);
				splitted.push_back(std::stod(token));
				break;
			case 'S':
			case 'W':
				token = token.substr(1, token.length() - 1);
				splitted.push_back(std::stod(token)*-1);
			default:
				splitted.push_back(std::stod(token));
			}
			coord.erase(0, pos + delimiter.length());
		}
		splitted.push_back(std::stod(coord));
		return splitted[0] + splitted[1] / 60 + splitted[2] / 3600;
	}

};
class Airport
{
	//stores the airport regex data
public:
	std::string m_icao;
	std::vector<std::regex> m_standregex;
	Airport(std::string icao, std::vector<std::string> standregex)
	{
		m_icao = icao;
		for (auto elem : standregex)
		{
			auto temp = std::regex(elem);
			m_standregex.push_back(temp);
		}
	}
	bool operator==(std::string rhs)
		//overwriting the == and < operator to make use of the STL algorithms for sorting and finding lateron
	{
		if (this->m_icao == rhs)
		{
			return true;
		}
		return false;
	}
};
void markStandsasOccupied(Stand mystand, std::string code, std::string icao);
class CUAEController :
	//The class that holds all our functions 
	public EuroScopePlugIn::CPlugIn
{
public:
	//the list displayed in euroscope
	//the list displayed in euroscope
	EuroScopePlugIn::CFlightPlanList  m_TOSequenceList;
	//this vector holds a CTOTData for each aircraft
	std::vector<CTOTData> m_sequence_OMDB;
	std::vector<CTOTData> m_sequence_OMSJ;
	std::vector<CTOTData> m_sequence_OMDW;
	std::vector<CTOTData> m_sequence_OMAA;


	CUAEController(void);



	virtual ~CUAEController(void);


	/*
	This function overrides a Euroscope function. If you type ".showtolist" in the euroscope textbox it will show the t/o sequence list
	Input: sCommandLine (the textbox string)
	*/
	static bool test()
	{
		return true;
	}

	bool fileExists(const std::string& filename)
	{
		struct stat buf;
		if (stat(filename.c_str(), &buf) != -1)
		{
			return true;
		}
		return false;
	}

	int     _SelectAcIndex(EuroScopePlugIn::CFlightPlan flightplan);
	//This function searches our sequence for the input "flightplan" and returns the index



	virtual void  OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan,
		EuroScopePlugIn::CRadarTarget RadarTarget,
		int ItemCode,
		int TagData,
		char sItemString[16],
		int * pColorCode,
		COLORREF * pRGB,
		double * pFontSize);

	std::string isFlightPlanValid(std::vector<RouteTo> dt, std::string Route, int level);
	//returnvalue: "o": valid
	//             "L": level invalid but route valid
	//             "R": route invalid but level valid
	//             "X": both invalid

	virtual EuroScopePlugIn::CRadarScreen    *OnRadarScreenCreated(const char * sDisplayName,
		bool NeedRadarContent,
		bool GeoReferenced,
		bool CanBeSaved,
		bool CanBeCreated);
	virtual void    OnFunctionCall(int FunctionId,
		const char * sItemString,
		POINT Pt,
		RECT Area);
	inline  virtual void    OnRadarTargetPositionUpdate(EuroScopePlugIn::CRadarTarget RadarTarget);


	inline  virtual bool    OnCompileCommand(const char * sCommandLine);
	inline  virtual void    OnTimer(int Counter);

	void cleanupStands();
	bool isDestValid(std::string callsign, EuroScopePlugIn::CFlightPlanData data);
	Stand extractRandomStand(std::vector<Stand> stands, char size, std::string icao);
	std::string getRouteRegion(std::unordered_map<std::string, RouteData> routedata, std::string icadep, std::string icaodest);
	char determineAircraftCat(EuroScopePlugIn::CFlightPlan);
	std::vector<Stand> getStandOfAircraft(EuroScopePlugIn::CPosition position);
	void readStandFile(std::string dir, std::string airport);

	void readCallsignFile(std::string dir, std::string airport);
	void assignCTOT(bool asap, EuroScopePlugIn::CFlightPlan);
	/*This function is called when a CTOT should be assigned to an aircraft. It constructs the new CTOTData object and redoes the sequence
		Input: bool asap (if an aircraft should be assigned a ctot that is as close to now as possible or at the end of the sequence)
			   CFlightPlan flightplan (the corresponding flightplan)
	*/


	void updateListOMDB();
	//This function is called from various other functions to do housekeeping on the actual euroscope list
	void updateListOMSJ();
	//This function is called from various other functions to do housekeeping on the actual euroscope list
	void updateListOMDW();
	//This function is called from various other functions to do housekeeping on the actual euroscope list
	void updateListOMAA();
	//This function is called from various other functions to do housekeeping on the actual euroscope list

	CTimeSpan getIncrement(EuroScopePlugIn::CFlightPlan fp1, EuroScopePlugIn::CFlightPlan fp2);


	/*This function is the heart of the implementation. It determines which separation fp2 needs to maintain to the preceeding aircraft fp1.
	  It takes into account the sids and wake turbulence category of the aircraft
	*/


	void recalculateCTOT(CTOTData inserted);

	/* This function is called when we change the order of the sequence either by assigning an aircraft and asap ctot or by manually assigning one.
	   The function recalculates all CTOTs that follow the "inserted" so the modified one.
	*/
};
