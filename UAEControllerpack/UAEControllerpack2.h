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
	bool operator==(const std::string& rhs)
	{
		if (this->m_name == rhs)
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
	bool operator<(const std::string& rhs) const
	{
		return std::lexicographical_compare(m_name.begin(), m_name.end(), rhs.begin(), rhs.end());
	}
	bool operator<(const std::string& rhs) 
	{
		return std::lexicographical_compare(m_name.begin(), m_name.end(), rhs.begin(), rhs.end());
	}
private:

};

class Fixes {
	//stores all waypoints
public:

	Fixes() {};
	static bool test()
	{
		return true;
	}
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
std::vector<Waypoint> parseATSPointsFromString(std::string route, std::vector<Waypoint> points = std::vector<Waypoint>());

std::vector<std::string> splitStringAtDelimiter(std::string string, char delimiter);

class Route 
{
public:
	Route(char type, std::string copn, std::string copx, std::string even, std::string routing,std::string levels, std::string onlyarr, std::string notarr, std::string onlydep, std::string notdep)
	{
		switch (type) 
		{
		case 'T': 
		{
			m_type = 0;
			break;
		}
		case 'A':
		{
			m_type = 1;
			break;
		}
		case 'D':
		{
			m_type = 2;
			break;
		}
		case 'I':
		{
			m_type = 3;
			break;
		}
		}
		routingATS = routing;
		evenodd = even;
		m_copn = copn;
		m_copx = copx;
		points = parseATSPointsFromString(routing);
		onlyForArrivalInto = splitStringAtDelimiter(onlyarr, ':');
		notForArrivalInto = splitStringAtDelimiter(notarr, ':');
		onlyforDepFrom = splitStringAtDelimiter(onlydep, ':');
		notforDepFrom = splitStringAtDelimiter(notdep, ':');
		levelrestriction = splitStringAtDelimiter(levels, ':');
	};
	std::string evenodd;
	std::vector<Waypoint> points;
	std::string m_copn,m_copx;
	int m_type; // 0: transit, 1: arrival, 2:departure
	std::string routingATS;
	std::vector<std::string> onlyForArrivalInto, notForArrivalInto, onlyforDepFrom, notforDepFrom,levelrestriction;
	std::string getCOPN()
	{
		return m_copn;
	}	
	std::string getCOPX()
	{
		return m_copx;
	}
	bool isValidForDepDestPair(std::string dep, std::string dest)
	{
		if (m_type == 1 && m_copx != dest) return false;
		if (m_type == 2 && m_copn != dep) return false;
		if (m_type == 3 && (m_copn != dep || m_copx != dest)) return false;
		if (onlyForArrivalInto.empty() && notForArrivalInto.empty() && onlyforDepFrom.empty() && notforDepFrom.empty())
			return true;
		auto onlyArrival = std::find(onlyForArrivalInto.begin(), onlyForArrivalInto.end(), dest);
		auto notForArrival = std::find(notForArrivalInto.begin(), notForArrivalInto.end(), dest);
		auto onlyDep = std::find(onlyforDepFrom.begin(), onlyforDepFrom.end(), dep);
		auto notDep = std::find(notforDepFrom.begin(), notforDepFrom.end(), dep);
		if (notDep != notforDepFrom.end() || notForArrival != notForArrivalInto.end())
			return false;
		if (!onlyForArrivalInto.empty() && onlyArrival == onlyForArrivalInto.end())
			return false;
		if (!onlyforDepFrom.empty() && onlyDep == onlyforDepFrom.end())
			return false;
		return true;
	}
	bool isValidForLevel(int level)
	{
		bool returnvalue = false;
		if (levelrestriction.empty()) returnvalue = true;
		for (auto elem : levelrestriction)
		{
			if (elem == "NONE")
			{
				returnvalue = true;
				break;
			}
				
			int restriction = std::stoi(elem.substr(0, 3));
			if (elem.ends_with("A"))
			{
				if (restriction * 100 <= level)
					returnvalue =  true;
				else return false;
			}
			if (elem.ends_with("B"))
			{
				if (restriction * 100 >= level)
					returnvalue = true;
				else return false;
			}
			if (elem.ends_with("0") || elem.ends_with("5"))
			{
				if (restriction * 100 == level)
					return true;
				else return false;
			}
		}
		//now checking evenodd
		if (evenodd == "ALL") return true;
		if (level > 40000) 
		{
			if (evenodd == "ODD")
			{
				if (((level / 1000) % 4 == 1)&& returnvalue) return true;
				else return false;
			}
			if (evenodd == "EVEN")
			{
				if (((level / 1000) % 4 == 3) && returnvalue) return true;
				else return false;
			}
			else return false;
		}
		else
		{
			if (evenodd == "ODD")
			{
				if ((level / 1000) % 2 == 1 && returnvalue) return true;
				else return false;
			}
			if (evenodd == "EVEN")
			{
				if ((level / 1000) % 2 == 0 && returnvalue) return true;
				else return false;
			}
			else return false;
			
		}		
	}
};
class FIR
	//holds information on valid routings and COPN/COPX 
{
public:
	FIR(std::string name) 
	{
		ICAOabb = name;
	};
	std::string ICAOabb;
	void add_Route(Route newRoute)
		//adds a Routing to the FIR
	{
		std::string key = newRoute.getCOPN();
		Routes.insert(std::make_pair(key, newRoute));
		COPNs.insert(key);
		COPXs.insert(newRoute.getCOPX());
	}
	std::vector<Route> getAllRoutesfromCOPN(std::string COPN)
		//returns all Routes for a given COPN
	{
		std::vector<Route> returnvalue;
		auto result = Routes.equal_range(COPN);
		for (auto elem = result.first; elem != result.second; elem++)
		{
			returnvalue.push_back(elem->second);
		}
		return returnvalue;
	}
	std::vector<Route> getAllRoutesToCOPX(std::string COPX)
	{
		std::vector<Route> returnvalue;
		for (auto& elem : Routes)
		{
			if (elem.second.getCOPX() == COPX)
				returnvalue.push_back(elem.second);
		}
		return returnvalue;
	}
	auto isValidInThisFIRUntil(EuroScopePlugIn::CFlightPlan fp, int pointsRemaining, std::vector<std::string>::iterator& copn)
	{
		bool arrivalThisFIR = false;
		bool departureThisFIR = false;
		std::string dest = fp.GetFlightPlanData().GetDestination();
		std::string origin = fp.GetFlightPlanData().GetOrigin();
		if (std::find(COPXs.begin(), COPXs.end(), dest) != COPXs.end())
		{
			arrivalThisFIR = true;
		}
		if (origin == *copn)
		{
			departureThisFIR = true;
		}
		std::vector<Route> allRoutes = this->getAllRoutesfromCOPN(*copn);
		for (auto& checkRoute : allRoutes)
		{
			auto& PointsRoute = checkRoute.points;
			if (*copn == origin)
			{
				copn++;
				pointsRemaining--;
			}
				
			if (pointsRemaining < PointsRoute.size())
				continue;
			auto mismatch = std::mismatch(PointsRoute.begin(), PointsRoute.end(), copn);
			if (mismatch.first == PointsRoute.end())
			{

				/*if (departureThisFIR && checkRoute.m_type != 2)
				{
					std::string logstring = "Found matching route " + checkRoute.routingATS + " for " + fp.GetCallsign() + " in " + this->ICAOabb;
					logstring += ". However it was not classified as a departure routing in this FIR.";
					LOG_F(INFO, logstring.c_str());
					continue;
				}
				else if (arrivalThisFIR && checkRoute.m_type != 1 )
				{
					std::string logstring = "Found matching route " + checkRoute.routingATS + " for " + fp.GetCallsign() + " in " + this->ICAOabb;
					logstring += ". However it was not classified as a arrival routing in this FIR.";
					LOG_F(INFO, logstring.c_str());
					continue;
				}
				else if (arrivalThisFIR)
				{
					if (*mismatch.second != dest)
						continue;
					else if (checkRoute.m_copx == dest)
						return checkRoute;
					else if (checkRoute.m_copx != dest)
						continue;
				}*/
				if (arrivalThisFIR && departureThisFIR && checkRoute.m_type != 3)
				{
					std::string logstring = "Found matching route " + checkRoute.routingATS + " for " + fp.GetCallsign() + " in " + this->ICAOabb;
					logstring += ". However it was not classified as a domestic routing in this FIR.";
					LOG_F(INFO, logstring.c_str());
					continue;
				}
				if (!checkRoute.isValidForDepDestPair(origin, dest))
				{
					std::string logstring = "Found matching route " + checkRoute.routingATS + " for " + fp.GetCallsign() + " in " + this->ICAOabb;
					logstring += ". However it was not valid for this departure/destination pairing.";
					LOG_F(INFO, logstring.c_str());
					continue;
				}
				
				//std::string logstring = "Found matching route " + checkRoute.routingATS + " for " + fp.GetCallsign() + " in " + this->ICAOabb;
				//LOG_F(INFO, logstring.c_str());
				copn = mismatch.second;
				copn--;
				return checkRoute;
			}
		}
		std::string routing = fp.GetFlightPlanData().GetRoute();
		std::string logstring = "No matching route (" + routing + ") for " + fp.GetCallsign() + " found in " + this->ICAOabb;
		LOG_F(INFO, logstring.c_str());
		return Route('T',"ERROR","ERROR", "ALL","VUTEB","NONE","NONE","NONE","NONE","NONE");
	}
	std::set<std::string> COPNs, COPXs;
	std::unordered_multimap<std::string, Route> Routes;
};
void WayPointNotFound(std::string name);
void AirwayWaypointConnectionNotFound(std::string pointname, std::string airwayname);

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

	std::string isFlightPlanValid(EuroScopePlugIn::CFlightPlan fp, EuroScopePlugIn::CFlightPlanExtractedRoute route, int level, bool showHelp = false);
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
	std::vector<std::string> getRoutePoints(EuroScopePlugIn::CFlightPlanExtractedRoute Route)
	{
		std::regex sid("[A-Z]*\\\d[A-Z]+");
		std::vector<std::string> filedPoints;
		std::smatch m;
		int length = Route.GetPointsNumber();
		std::string airwaylast = "";
		try {

			std::vector<std::string> espoints;
			for (int i = 1; i <= length - 1; i++) //was length -2
			{
				std::string airway = Route.GetPointAirwayName(i);
				std::string airwaynext = Route.GetPointAirwayName(i + 1);
				if (std::regex_search(airway, m, sid) && airway == airwaynext ) //|| airwaynext.empty()
					continue;
				espoints.push_back(Route.GetPointName(i));
				airwaylast = airway;
			}

			filedPoints = espoints;
			return filedPoints;
		}

		catch (...)
		{
			std::string logstring = "Exception thrown for route ";
			logstring += Route.GetPointName(0);
			logstring += " to ";
			logstring += Route.GetPointName(length - 1);
			LOG_F(ERROR, logstring.c_str());
			return filedPoints;
		}
	}
	bool isDestValid(std::string callsign, EuroScopePlugIn::CFlightPlanData data);
	Stand extractRandomStand(std::vector<Stand> stands, char size, std::string icao);
	char determineAircraftCat(EuroScopePlugIn::CFlightPlan);
	std::vector<Stand> getStandOfAircraft(EuroScopePlugIn::CPosition position);
	void readStandFile(std::string dir, std::string airport);

	void readCallsignFile(std::string dir, std::string airport);


	
};
