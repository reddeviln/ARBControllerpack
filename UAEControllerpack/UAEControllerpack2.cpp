#pragma once
#include "pch.h"
#include "UAEControllerpack2.h"
#include "loguru.cpp"

#define MY_PLUGIN_NAME      "Controller Pack ARBvACC"
#define MY_PLUGIN_VERSION   "4.0.0"
#define MY_PLUGIN_DEVELOPER "Nils Dornbusch"
#define MY_PLUGIN_COPYRIGHT "Licensed under GNU GPLv3"
#define MY_PLUGIN_VIEW      ""

//Tag items
const int TAG_ITEM_CTOT = 1121312;
const int TAG_ITEM_Sequence = 212312;
const int TAG_ITEM_TOBT = 312312;
const int TAG_ITEM_ROUTE_VALID = 123123;
const int TAG_ITEM_RECAT = 155;
const int TAG_ITEM_RECAT_NOSLASH = 12341;
const int TAG_ITEM_STAND = 1548915;
const int TAG_ITEM_COPXSHORT = 524865;
const int TAG_ITEM_APPSHORT = 58426;

//Functions
const int TAG_FUNC_EDIT = 423;
const int TAG_FUNC_ASSIGN_POPUP = 456456;
const int TAG_FUNC_ASSIGN_AUTO = 412;
const int TAG_FUNC_ASSIGN_CARGO = 4578;
const int TAG_FUNC_ASSIGN_PAX = 456;
const int TAG_FUNC_ASSIGN_UAE = 31854;
const int TAG_FUNC_ASSIGN_LOWCOST = 3458;
const int TAG_FUNC_ASSIGN_VIP = 4868;
const int TAG_FUNC_ASSIGN_GA = 486;
const int TAG_FUNC_ASSIGN_ABY = 2342;
const int TAG_FUNC_ASSIGN_CARGO2 = 23230;
const int TAG_FUNC_MANUAL_FINISH = 2345;
const int TAG_FUNC_ASSIGN_ETD = 568978;
const int TAG_FUNC_CLEAR = 264;
const int TAG_FUNC_ROUTING = 15;
const int TAG_FUNC_ROUTING_OPT = 8463682;
const int TAG_FUNC_CTOT_MANUAL = 1;
const int TAG_FUNC_CTOT_ASSIGN = 2;
const int TAG_FUNC_CTOT_MANUAL_FINISH = 10;
const int TAG_FUNC_CTOT_ASSIGN_SEQ = 11;
const int TAG_FUNC_CTOT_ASSIGN_ASAP = 13;
const int TAG_FUNC_CTOT_CLEAR = 86715123;

//Global variables
const CTimeSpan taxitime = CTimeSpan(0, 0, 20, 0);
const double TOL = 0.02;
std::unordered_map<std::string, std::string> recatdict;
std::unordered_map<std::string, std::string> parkingdict;
std::string directory;
std::unordered_map<std::string, std::unordered_map<std::string, Stand>> data;
std::unordered_map<std::string, std::unordered_map<std::string, Stand>> standmapping;
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> callsignmap;
std::unordered_map<std::string, std::vector<Stand>> standsUAE;
std::unordered_map<std::string, std::vector<Stand>> standsPAX;
std::unordered_map<std::string, std::vector<Stand>> standsCARGO;
std::unordered_map<std::string, std::vector<Stand>> standsLOWCOST;
std::unordered_map<std::string, std::vector<Stand>> standsVIP;
std::unordered_map<std::string, std::vector<Stand>> standsGA;
std::unordered_map<std::string, std::vector<Stand>> standsOverflow;
std::unordered_map<std::string, std::vector<Stand>> standsABY;
std::unordered_map<std::string, std::vector<Stand>> standsETD;
std::unordered_map<std::string, std::vector<Stand>> standsCargoSpecial;
std::vector<Airport> activeAirports;
std::vector<std::string> WaypointErrors;
std::unordered_map<std::string, FIR> allFIRs;
std::unordered_map<std::string, std::string> abb;
std::hash<std::string>  hasher;
Fixes fixes;
//Constructor (run at plugin load)
CUAEController::CUAEController(void)
	: CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
		MY_PLUGIN_NAME,
		MY_PLUGIN_VERSION,
		MY_PLUGIN_DEVELOPER,
		MY_PLUGIN_COPYRIGHT)
{
	char path[MAX_PATH];
	HMODULE hm = NULL;
	//workaround to get dll path
	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)&CUAEController::test, &hm) == 0)
	{
		int ret = GetLastError();
		fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
		// Return or however you want to handle an error.
	}
	if (GetModuleFileName(hm, path, sizeof(path)) == 0)
	{
		int ret = GetLastError();
		fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
		// Return or however you want to handle an error.
	}
	std::string dir(path);
	std::string filename("ARBControllerpack.dll");
	size_t pos = dir.find(filename);
	dir.replace(pos, filename.length(), "");
	loguru::set_thread_name("ARBControllerpack");
	loguru::set_fatal_handler([](const loguru::Message& message) {
		std::string stack = loguru::stacktrace_as_stdstring(1);
		LOG_F(ERROR, stack.c_str());
		throw std::runtime_error(std::string(message.prefix) + message.message); });
	//putting a logfile in place
	std::string temp = dir + "ARBC.log";
	loguru::add_file(temp.c_str(), loguru::Truncate, loguru::Verbosity_INFO);
	std::string logstring = "We successfully started ARBControllerPack version ";
	logstring += MY_PLUGIN_VERSION;
	logstring += ". Great success!";
	LOG_F(INFO, logstring.c_str());
	
	//registering all Functions and Tag items
	RegisterTagItemType("RECAT", TAG_ITEM_RECAT);
	RegisterTagItemType("RECATnoSlash", TAG_ITEM_RECAT_NOSLASH);
	RegisterTagItemType("Stand", TAG_ITEM_STAND);
	RegisterTagItemFunction("Edit Stand", TAG_FUNC_EDIT);
	RegisterTagItemFunction("Assign Stand", TAG_FUNC_ASSIGN_POPUP);
	RegisterTagItemFunction("Clear", TAG_FUNC_CLEAR);
	RegisterTagItemType("RouteValid", TAG_ITEM_ROUTE_VALID);
	RegisterTagItemFunction("Routing help smart", TAG_FUNC_ROUTING);
	RegisterTagItemFunction("Routing help full", TAG_FUNC_ROUTING_OPT);
	RegisterTagItemType("COPX Short form/Destination", TAG_ITEM_COPXSHORT);
	RegisterTagItemType("Shortening of SID/arriving airport", TAG_ITEM_APPSHORT);
	//m_TOSequenceList = RegisterFpList("T/O Sequence List");
	//if (m_TOSequenceList.GetColumnNumber() == 0)
	//{
	//	// fill in the default columns of the list in none are available from settings
	//	m_TOSequenceList.AddColumnDefinition("C/S", 10, true, NULL, EuroScopePlugIn::TAG_ITEM_TYPE_CALLSIGN,
	//		NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO,
	//		NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO);
	//	m_TOSequenceList.AddColumnDefinition("CTOT", 4, true, MY_PLUGIN_NAME, TAG_ITEM_CTOT,
	//		NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO,
	//		NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO);
	//	m_TOSequenceList.AddColumnDefinition("#", 2, true, MY_PLUGIN_NAME, TAG_ITEM_Sequence,
	//		NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO,
	//		NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO);
	//	m_TOSequenceList.AddColumnDefinition("W", 1, true, NULL, EuroScopePlugIn::TAG_ITEM_TYPE_AIRCRAFT_CATEGORY,
	//		NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO,
	//		NULL, EuroScopePlugIn::TAG_ITEM_FUNCTION_NO);

	//}

	directory = dir;
	LOG_F(INFO, "Everything registered. Ready to go!");

	//----------------------------------------Loading of all the different files--------------------------------------------
	//0. Navdata 
	std::string navdir = directory;
	
	std::string from = "Plugins\\ARBControllerPack";
	std::string from1 = "Plugins\\ARBControllerpack";
	auto start_pos = navdir.find(from);
	if (start_pos == std::string::npos)
		start_pos = navdir.find(from1);
	if (start_pos == std::string::npos)
		LOG_F(ERROR, "Reinstall your sectorfile please directory setup does not match the expected.");
	navdir.replace(start_pos, from.length(), "Navdata");
	auto navfile = navdir + "airway.txt";
	io::CSVReader<16, io::trim_chars<' '>, io::no_quote_escape<'\t'>, io::throw_on_overflow, io::single_line_comment<'\;'>> navreader(navfile);
	std::string name, lat1, long1, garbage, airway, highlow, prevPoint, lat2, long2, minAlt, pointprevOK, nextPoint, lat3, long3, minalt2,nextPointOK;
	navreader.set_header("name", "lat1", "long1", "garbage", "airway", "highlow", "prevPoint", "lat2", "long2", "minAlt", "pointprevOK", "nextPoint", "lat3", "long3", "minalt2", "nextPointOK");
	LOG_F(INFO, "Start reading navdata this might take a bit ....");
	while (navreader.read_row(name, lat1, long1, garbage, airway, highlow, prevPoint, lat2, long2, minAlt, pointprevOK, nextPoint, lat3, long3, minalt2, nextPointOK))
	{
		Waypoint temp= Waypoint(name);
		auto found = fixes.find_waypoint(name);
		if (found.m_name != "ERROR")
		{
			temp = found;
		}
					
		if (nextPointOK == "Y")
		{
			temp.addConnection(airway, nextPoint);
		}
		if (pointprevOK == "Y")
		{
			temp.addConnection(airway, prevPoint);
		}
		fixes.add_fix(temp);

	}
	LOG_F(INFO, "Navdata reading complete. Very nice!");
	//1. RECAT dictionary
	dir += "RECAT.csv";
	io::CSVReader<3, io::trim_chars<' '>, io::no_quote_escape<','>> in(dir);
	in.read_header(io::ignore_extra_column, "Aircraft", "RECAT", "PARKING");
	std::string aircraft, recat, parkingcode;
	while (in.read_row(aircraft, recat, parkingcode))
	{
		std::pair<std::string, std::string> temp(aircraft, recat);
		std::pair<std::string, std::string> tmp(aircraft, parkingcode);
		recatdict.insert(temp);
		parkingdict.insert(tmp);
	}
	LOG_F(INFO, "RECAT dictionary read without issues!");
	///////////////////////////////////////////////////////////////////////////////////
	//-------------------------------------------2. All Airport stands and mappings
	////////////////////////////////////////////////////////////////////////////////////
	std::string airportdir = directory;
	airportdir += "airports.csv";
	io::CSVReader<5, io::trim_chars<' '>, io::double_quote_escape<';','\"'>> inAirport(airportdir);
	inAirport.read_header(io::ignore_extra_column, "ICAO", "Regex1","Regex2","Regex3","Regex4");
	std::string airport, regex1, regex2,regex3,regex4;
	while (inAirport.read_row(airport, regex1, regex2,regex3,regex4))
	{
		if (strcmp(regex1.c_str(), "NONE") == 0)
		{
			std::string logstring = "No regex was specified for airport ";
			logstring += airport;
			logstring += ". Ignoring it.";
			LOG_F(WARNING, logstring.c_str());
			continue;
		}
		std::vector<std::string> regexs;
		regexs.push_back(regex1);
		if (strcmp(regex2.c_str(), "NONE") != 0)
			regexs.push_back(regex2);
		if (strcmp(regex3.c_str(), "NONE") != 0)
			regexs.push_back(regex3);		
		if (strcmp(regex4.c_str(), "NONE") != 0)
			regexs.push_back(regex4);
		
		Airport temp = Airport(airport, regexs);
		activeAirports.push_back(temp);
	}
	LOG_F(INFO, "Airport regexes read successfully.");
	std::regex icao(R"(.*\\([A-Z]{4}))");
	for (auto entry : std::filesystem::directory_iterator(directory))
	{
		if (entry.is_directory())
		{
			std::smatch m;
			std::string name = entry.path().string();
			if (std::regex_search(name, m, icao))
			{
				std::string temp = "/";
				auto temp2 = temp;
				temp += m.str(1);
				temp += ".csv";
				temp2 += "CallsignMap";
				temp2 += m.str(1);
				temp2 += ".csv";
				auto standpath = std::filesystem::path(name + temp);
				auto callsignpath = std::filesystem::path(name + temp2);
				if (std::filesystem::exists(standpath) && std::filesystem::exists(callsignpath))
				{
					readStandFile(standpath.string(), m.str(1));
					readCallsignFile(callsignpath.string(), m.str(1));
					std::string logstring;
					logstring = "Successfully parsed stand data for ";
					logstring += m[0];
					logstring += ".";
					LOG_F(INFO, logstring.c_str());
				}
				else
				{
					std::string logstring;
					logstring = "Not all required csv files found for ";
					logstring += m[0];
					logstring += ". Skipping this airport ...";
					LOG_F(WARNING, logstring.c_str());
				}
			}
		}
	}
	//data handling of parsed stands
	for (auto airport : activeAirports)
	{
		auto found = data.find(airport.m_icao);
		if (found == data.end()) break;
		std::vector<Stand> thisstandsUAE;
		std::vector<Stand> thisstandsABY;
		std::vector<Stand> thisstandsPAX;
		std::vector<Stand> thisstandsCARGO;
		std::vector<Stand> thisstandsCARGOspec;
		std::vector<Stand> thisstandsLOWCOST;
		std::vector<Stand> thisstandsGA;
		std::vector<Stand> thisstandsETD;
		std::vector<Stand> thisstandsVIP;
		std::vector<Stand> thisstandsOverflow;
		for (auto stand : found->second)
		{
			auto code = stand.second.mAirlinecode;
			if (code == "UAE")
			{
				thisstandsUAE.push_back(stand.second);
				continue;
			}
			if (code == "ABY")
			{
				thisstandsABY.push_back(stand.second);
				continue;
			}
			if (code == "PAX")
			{
				thisstandsPAX.push_back(stand.second);
				continue;
			}
			if (code == "ETD")
			{
				thisstandsETD.push_back(stand.second);
				continue;
			}

			if (code == "CARGO" || code == "CLC")
				thisstandsCARGO.push_back(stand.second);
			if (code == "LWC" || code == "CLC")
				thisstandsLOWCOST.push_back(stand.second);
			if (code == "GA")
			{
				thisstandsGA.push_back(stand.second);
				continue;
			}
			if (code == "CARGO1")
			{
				thisstandsCARGOspec.push_back(stand.second);
				continue;
			}
			if (code == "VIP")
			{
				thisstandsVIP.push_back(stand.second);
				continue;
			}
			if (code == "ALL")
			{
				thisstandsOverflow.push_back(stand.second);
				continue;
			}


		}
		std::pair<std::string, std::vector<Stand>> temp(airport.m_icao, thisstandsUAE);
		standsUAE.insert(temp);
		std::pair<std::string, std::vector<Stand>> temp2(airport.m_icao, thisstandsABY);
		standsABY.insert(temp2);
		std::pair<std::string, std::vector<Stand>> temp3(airport.m_icao, thisstandsPAX);
		standsPAX.insert(temp3);
		std::pair<std::string, std::vector<Stand>> temp4(airport.m_icao, thisstandsCARGO);
		standsCARGO.insert(temp4);
		std::pair<std::string, std::vector<Stand>> temp5(airport.m_icao, thisstandsCARGOspec);
		standsCargoSpecial.insert(temp5);
		std::pair<std::string, std::vector<Stand>> temp6(airport.m_icao, thisstandsLOWCOST);
		standsLOWCOST.insert(temp6);
		std::pair<std::string, std::vector<Stand>> temp7(airport.m_icao, thisstandsGA);
		standsGA.insert(temp7);
		std::pair<std::string, std::vector<Stand>> temp8(airport.m_icao, thisstandsVIP);
		standsVIP.insert(temp8);
		std::pair<std::string, std::vector<Stand>> temp9(airport.m_icao, thisstandsOverflow);
		standsOverflow.insert(temp9);
		std::pair<std::string, std::vector<Stand>> temp10(airport.m_icao, thisstandsETD);
		standsETD.insert(temp10);

	}
	/////////////////////////////////////////////////////////////////////////////////////
    //---------------------------3. Route restriction files
	/////////////////////////////////////////////////////////////////////////////////////
	dir = directory+"Routings";
	int numberOfFIRs = 0;
	for (auto entry : std::filesystem::directory_iterator(dir))
	{
		if (!entry.is_directory())
		{
			std::smatch m;
			std::string name = entry.path().string();
			if (std::regex_search(name, m, icao))
			{
				auto path = std::filesystem::path(name);
				if (std::filesystem::exists(name))
				{
					try
					{
						io::CSVReader<10, io::trim_chars<' '>, io::no_quote_escape<','>> in2(name);
						in2.read_header(io::ignore_extra_column, "TYPE", "COPN", "COPX", "EVENODD", "ROUTE", "LEVELS", "ONLY_AVAIL_DEST", "NOT_AVAIL_DEST", "ONLY_AVAIL_DEP", "NOT_AVAIL_DEP");
						std::string type, copn, copx, evenodd, route, levels, onlydest, notdest, onlydep, notdep;
						FIR newFIR(m.str(1));
						while (in2.read_row(type, copn, copx, evenodd, route, levels, onlydest, notdest, onlydep, notdep))
						{
							auto types = splitStringAtDelimiter(type, ':');
							for (auto& elem : types)
							{
								auto copns = splitStringAtDelimiter(copn, ':');
								auto copxs = splitStringAtDelimiter(copx, ':');
								for (auto& copnelem : copns)
								{
									for (auto& copxelem : copxs)
									{
										Route temp(*elem.begin(), copnelem, copxelem, evenodd, route, levels, onlydest, notdest, onlydep, notdep);
										newFIR.add_Route(temp);
									}
								}
							}
						}
						allFIRs.insert(std::make_pair(m.str(1), newFIR));
						std::string logstring;
						logstring = "Successfully parsed FIR data for ";
						logstring += m[0];
						logstring += ".";
						LOG_F(INFO, logstring.c_str());
						numberOfFIRs++;
					}
					catch (const std::exception&)
					{
						std::string logstring;
						logstring = "Something went wrong for file ";
						logstring += m[0];
						logstring += ". Skipping ...";
						LOG_F(WARNING, logstring.c_str());
					}
				}
				else
				{
					std::string logstring;
					logstring = "Something went wrong for file ";
					logstring += m[0];
					logstring += ". Skipping ...";
					LOG_F(WARNING, logstring.c_str());
				}
			}
		}
	}
	logstring = "Successfully parsed routing data for " + std::to_string(numberOfFIRs) + " FIRs. Too much good.";
	LOG_F(INFO, logstring.c_str());
	
		//////////////////////////////////////////////////////////////////////////////////////////
	//-----------------------------4. diverse files--------------------------------------
	//////////////////////////////////////////////////////////////////////////////////
	dir = directory;
	dir += "abbreviations.csv";
	if (!this->fileExists(dir))
	{
		LOG_F(WARNING, "Abbreviations file not found. Tag items will not be available.");
		return;
	}
	io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<','>> in10(dir);
	std::string Long, Short;
	in10.read_header(io::ignore_extra_column, "LONG", "SHORT");
	while (in10.read_row(Long, Short))
	{
		abb.insert(std::pair(Long, Short));
	}
	LOG_F(INFO, "Done file reading.");
}
//Destructor
CUAEController::~CUAEController(void)
{
	standmapping.clear();
}

//------------Functions called by Euroscope in various situations-----------------------------
EuroScopePlugIn::CRadarScreen    *CUAEController::OnRadarScreenCreated(const char * sDisplayName,
	bool NeedRadarContent,
	bool GeoReferenced,
	bool CanBeSaved,
	bool CanBeCreated)
{
	if (strcmp(sDisplayName, "Timer") == 0)
		return NULL;
	TimerRadar * myscreen = new TimerRadar(directory);
	return myscreen;
}
void CUAEController::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan,
	EuroScopePlugIn::CRadarTarget RadarTarget,
	int ItemCode,
	int TagData,
	char sItemString[16],
	int * pColorCode,
	COLORREF * pRGB,
	double * pFontSize)
{
	if (!FlightPlan.IsValid())
		return;
	auto fpdata = FlightPlan.GetFlightPlanData();
	std::string dep = fpdata.GetOrigin();
	switch (ItemCode) //when Euroscope wants to know the value of these items we return it. Switch by which one it wants
	{
		
		case TAG_ITEM_RECAT: case TAG_ITEM_RECAT_NOSLASH:
		{
			std::string type = FlightPlan.GetFlightPlanData().GetAircraftInfo();
			if (type.find("/") != std::string::npos)
			{
				std::regex re = std::regex(R"(\/(.{4})\/)");
				std::smatch match;
				if (std::regex_search(type, match, re))
				{
					type = match.str(1);
				}
				else
				{
					std::string firsttype = type.substr(0, 4);
					type = firsttype;
				}
			}
			auto found = recatdict.find(type);
			if (found != recatdict.end())
			{
				std::string out = found->second;
				if (ItemCode == TAG_ITEM_RECAT)
					out = "/" + found->second;
				strcpy(sItemString, out.c_str());
				return;
			}
			else
			{
				char wtc = FlightPlan.GetFlightPlanData().GetAircraftWtc();
				std::string temp;
				switch (wtc)
				{
				case 'J':
				{
					temp = "A";
					break;
				}
				case 'H':
				{
					temp = "B";
					break;
				}
				case 'M':
				{
					temp = "D";
					break;
				}
				case 'L':
				{
					temp = "G";
					break;
				}
				}
				std::string out;
				if (ItemCode == TAG_ITEM_RECAT)
					out = "/" + temp;
				else
					out = temp;
				strcpy(sItemString, out.c_str());
			}
			
			break;
		}
		case TAG_ITEM_STAND:
		{
			auto icao = FlightPlan.GetFlightPlanData().GetDestination();
			if (std::find(activeAirports.begin(), activeAirports.end(), icao) == activeAirports.end())
			{
				std::string logstring = "The airport ";
				logstring += icao;
				logstring += " does not have any data setup. Ignoring it.";

				LOG_F(INFO, logstring.c_str());
				return;
			}

			auto fpdata = FlightPlan.GetFlightPlanData();
			std::string remarks = fpdata.GetRemarks();
			auto dest = fpdata.GetDestination();
			std::smatch match;
			auto foundAirport = std::find(activeAirports.begin(),activeAirports.end(),dest);
			if(foundAirport!=activeAirports.end())
			{
				for (auto re : foundAirport->m_standregex)
				{
					if (std::regex_search(remarks, match, re))
					{
						auto stand = match.str(1);
							auto found2 = data.find(icao);

							if (found2 == data.end()) break;
							auto found = found2->second.find(stand);
							if (found != found2->second.end())
							{
								found->second.isAssigned = true;
									auto temp2 = standmapping.find(icao);

									if (temp2 == standmapping.end())
									{
										std::unordered_map<std::string, Stand> tempp;
										standmapping.insert(std::make_pair(icao, tempp));
									}
								auto copy = standmapping.at(icao);
								//LOG_F(INFO, std::to_string(standmapping.size()).c_str());
								std::pair<std::string, Stand> temp(FlightPlan.GetCallsign(), found->second);
								copy.insert(temp);
								standmapping.at(icao) = copy;
							}
						auto aircraftposition = FlightPlan.GetCorrelatedRadarTarget().GetPosition().GetPosition();
						if (CUAEController::determineAircraftCat(FlightPlan) > found->second.mSize)
						{
							*pColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
							*pRGB = RGB(255, 191, 0);
						}
						if (!FlightPlan.GetCorrelatedRadarTarget().IsValid())
						{
							strcpy(sItemString, found->second.number.c_str());
							return;
						}
						if (!found->second.isEmpty && aircraftposition.DistanceTo(found->second.position) >= TOL)
						{
							auto temp1 = aircraftposition.DistanceTo(found->second.position);
							auto temp2 = aircraftposition.m_Latitude;
							std::string logstring = "Duplicate stand detected. Distance: ";
							logstring += std::to_string(temp1);
							logstring += ". Latitude of aircraft: ";
							logstring += std::to_string(temp2);

							LOG_F(INFO, logstring.c_str());
							*pColorCode = EuroScopePlugIn::TAG_COLOR_EMERGENCY;
						}

						strcpy(sItemString, found->second.number.c_str());
						return;
					}
				}
			}
			break;

		}
		case TAG_ITEM_ROUTE_VALID:
		{
			auto fpdata = FlightPlan.GetFlightPlanData();
			if (FlightPlan.GetClearenceFlag())
			{
				auto cdata = FlightPlan.GetControllerAssignedData();
				cdata.SetFlightStripAnnotation(3, "");
				std::string logstring = "Aircraft ";
				logstring += FlightPlan.GetCallsign();
				logstring += " is skipped because Clearance flag is set.";
				LOG_F(INFO, logstring.c_str());
				return;
			}
			auto test = fpdata.GetPlanType();
			if (strcmp(test, "V") == 0)
			{
				strcpy(sItemString, "");
				return;
			}
			std::string icaodest = fpdata.GetDestination();
			std::string icaodep = fpdata.GetOrigin();
			auto route = FlightPlan.GetExtractedRoute();
	
			std::string valid = isFlightPlanValid(FlightPlan, route, fpdata.GetFinalAltitude());
			
			if (valid == "r") 
			{
				valid = "R";
				*pColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
				*pRGB = RGB(255, 191, 0);
			}
			else *pColorCode = EuroScopePlugIn::TAG_COLOR_EMERGENCY;

			
			if (strcmp(valid.c_str(), "o") == 0) return;
			strcpy(sItemString, valid.c_str());
			return;
		}
		
		case TAG_ITEM_COPXSHORT:
		{
			std::string copx = FlightPlan.GetNextFirCopxPointName();
			std::string dest = fpdata.GetDestination();
			if (copx.empty())
			{
				strcpy(sItemString, dest.substr(2, 3).c_str());
				return;
			}
			strcpy(sItemString, copx.substr(0, 3).c_str());
			return;
		}
		case TAG_ITEM_APPSHORT:
		{
			std::string dest = fpdata.GetDestination();
			if (abb.empty())
			{
				strcpy(sItemString, "ERR");
				return;
			}
			auto found = abb.find(dest);
			if (found != abb.end())
			{
				strcpy(sItemString, found->second.c_str());
				return;
			}
			else
			{
				std::string SID = fpdata.GetSidName();
				if (SID.length() < 7)
				{
					std::string logstring = "The SID ";
					logstring += SID;
					logstring += " does not contain at least 7 characters and the destination was not in the UAE. Cant do a shorthand for that. Aircraft concerned: ";
					logstring += FlightPlan.GetCallsign();
					LOG_F(WARNING, logstring.c_str());
					return;
				}
				std::string SIDpoint = SID.substr(0, 5);
				auto found2 = abb.find(SIDpoint);
				if (found2 != abb.end())
				{
					std::string output = found2->second;
					output += SID.substr(5, 7);
					strcpy(sItemString, output.c_str());
					return;
				}
				strcpy(sItemString, "ERR");
				return;
			}
		}
	}
}
std::string CUAEController::isFlightPlanValid(EuroScopePlugIn::CFlightPlan fp, EuroScopePlugIn::CFlightPlanExtractedRoute route, int level, bool showHelp)
{
	bool cruisevalid = true;
	bool furtherRouteAvail = true;
	auto filed_points = getRoutePoints(route);
	auto fpdata = fp.GetFlightPlanData();
	std::vector<Route> allRoutes, validRouting;
	std::string curFIRICAO;
	FIR currentFIR("");
	//first: check departure routing
	for (auto& curFIR : allFIRs)
	{
		allRoutes = curFIR.second.getAllRoutesfromCOPN(fpdata.GetOrigin());
		if (!allRoutes.empty())
		{
			curFIRICAO = curFIR.second.ICAOabb;
			currentFIR = curFIR.second;
			break;
		}
			
	}
	if (allRoutes.empty())
	{
		std::string logstring = "There was no departure routing specified for aircraft ";
		logstring += fp.GetCallsign();
		logstring += " with filed routing from ";
		logstring += fpdata.GetOrigin();
		logstring += " to ";
		logstring += fpdata.GetDestination();
		logstring += ". The filed routing was ";
		logstring += fpdata.GetRoute();
		logstring += ".";
		LOG_F(WARNING, logstring.c_str());
		return "R";
	}
	filed_points.insert(filed_points.begin(), fpdata.GetOrigin());
	filed_points.push_back(fpdata.GetDestination());
	auto currentCOPN = filed_points.begin();
	
	auto validRoute = currentFIR.isValidInThisFIRUntil(fp, std::distance(currentCOPN,filed_points.end()), currentCOPN);
	if (!validRoute.isValidForLevel(fp.GetFinalAltitude()))
	{
		cruisevalid = false;
		std::string logstring = "Found matching route " + validRoute.routingATS + " for " + fp.GetCallsign() + " in " + currentFIR.ICAOabb;
		logstring += ". However it was not valid for the filed level.";
		LOG_F(INFO, logstring.c_str());
	}
	if (validRoute.getCOPN() == "ERROR")
	{		
		auto cdata = fp.GetControllerAssignedData();
		std::string annotation = *currentCOPN + "," + currentFIR.ICAOabb;
		bool success = cdata.SetFlightStripAnnotation(3, annotation.c_str());
		if (validRouting.size() > 0) return "r";
		else return "R";
	}
	validRouting.push_back(validRoute);
	while (validRouting.back().getCOPX() != fpdata.GetDestination())  //validRouting.back().points.back() != filed_points.rbegin()[1]
	{
		for (auto& curFIR : allFIRs)
		{
			allRoutes = curFIR.second.getAllRoutesfromCOPN(validRouting.back().getCOPX());
			if (!allRoutes.empty()&&curFIRICAO!=curFIR.second.ICAOabb)
			{
				curFIRICAO = curFIR.second.ICAOabb;
				currentFIR = curFIR.second;
				furtherRouteAvail = true;
				break;
			}
			furtherRouteAvail = false;
		}
		if (!furtherRouteAvail)
		{
			auto dest = fp.GetFlightPlanData().GetDestination();
			for (auto& curFIR : allFIRs)
			{
				auto copxs = curFIR.second.COPXs;
				if (copxs.find(dest)!= copxs.end())
				{
					
					auto cdata = fp.GetControllerAssignedData();
					std::string annotation = *currentCOPN + "," + curFIR.second.ICAOabb;
					cdata.SetFlightStripAnnotation(3, annotation.c_str());
					return "R";
					
				}
			}
			return "o";
		}
		validRoute = currentFIR.isValidInThisFIRUntil(fp, std::distance(currentCOPN, filed_points.end()), currentCOPN);
		if (!validRoute.isValidForLevel(fp.GetFinalAltitude()))
		{
			cruisevalid = false;
			std::string logstring = "Found matching route " + validRoute.routingATS + " for " + fp.GetCallsign() + " in " + currentFIR.ICAOabb;
			logstring += ". However it was not valid for the filed level.";
			LOG_F(INFO, logstring.c_str());
		}
		if (validRoute.getCOPN() == "ERROR")
		{			
			auto cdata = fp.GetControllerAssignedData();
			std::string annotation = *currentCOPN + "," + currentFIR.ICAOabb;
			bool success = cdata.SetFlightStripAnnotation(3, annotation.c_str());
			if (validRouting.size() > 0) return "r";
			else return "R";
		}
		validRouting.push_back(validRoute);
	}
	if (!cruisevalid)
		return "L";
	return "o";
	
}
void  CUAEController::OnTimer(int Counter)
{
	if (Counter % 5 == 0)
	{
		cleanupStands();
	}
}
inline  bool CUAEController::OnCompileCommand(const char * sCommandLine)
{
	//show the t/o sequence list if we type the command
	if (std::strcmp(sCommandLine, ".showtolist") == 0)
	{
		m_TOSequenceList.ShowFpList(true);
		return true;
	}
	if (std::strcmp(sCommandLine, ".showstands") == 0)
	{

		for (auto airport : activeAirports)
		{
			std::string out;
			auto found = data.find(airport.m_icao);
			for (auto it : found->second)
			{
				if (!it.second.isEmpty || it.second.isAssigned)
				{
					out += it.first;
					out += " ";
				}
			}
			DisplayUserMessage("Occupied Stands", airport.m_icao.c_str(), out.c_str(), true, true, true, true, true);
		}


		return true;
	}
	return false;
}
void CUAEController::OnRadarTargetPositionUpdate(EuroScopePlugIn::CRadarTarget rt)
{
	if (rt.IsValid())
	{
		if (rt.GetPosition().GetPressureAltitude() > 1000) return;
		auto position = rt.GetPosition().GetPosition();
		auto standlist = getStandOfAircraft(position);
		for (auto stand : standlist)
		{
			auto icao = stand.mICAO;
			auto found = data.find(icao);
			if (found == data.end()) return;
			if (stand.number != "ZZZ") {
				found->second.at(stand.number).isEmpty = false;
				auto mystand = found->second.at(stand.number);
				auto code = mystand.mAirlinecode;
				markStandsasOccupied(mystand, code, icao);
			}
		}
		
	}
}
inline void CUAEController::OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area)
{
	//handle our registered functions
	EuroScopePlugIn::CFlightPlan  fp;
	CString                         str;


	// get the flightplan we are dealing with
	fp = FlightPlanSelectASEL();
	auto fpdata = fp.GetFlightPlanData();
	std::string dep = fpdata.GetOrigin();

	if (!fp.IsValid())
		return;

	// select it from the sequence

	// switch by the function ID
	switch (FunctionId)
	{
	case TAG_FUNC_EDIT:
		OpenPopupEdit(Area,
			TAG_FUNC_MANUAL_FINISH,
			"");
		LOG_F(INFO, "Opening textbox for stand assignment.");
		break;
	case TAG_FUNC_MANUAL_FINISH:
	{
		LOG_F(INFO, "Processing textinput as stand.");
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			return;
		auto fpdata = fp.GetFlightPlanData();
		std::string remarks = fpdata.GetRemarks();
		auto dest = fp.GetFlightPlanData().GetDestination();
		std::smatch match;
		for (auto airport : activeAirports)
		{
			if (strcmp(dest, airport.m_icao.c_str()) == 0)
			{
				for (auto re : airport.m_standregex)
				{
					if (std::regex_search(remarks, match, re))
						return;
				}
			}
		}
		LOG_F(INFO, "Aircraft can be modified. We are processing aircraft: ");
		LOG_F(INFO, fp.GetCallsign());
		std::string input = sItemString;
		std::transform(input.begin(), input.end(), input.begin(), ::toupper);
		
		auto found = data.find(dest);
		if (found == data.end()) return;
		auto found2 = found->second.find(input);
		if (found2 == found->second.end())
		{
			std::string logstring1 = "Stand " + input + " does not exist at "+dest+".";
			DisplayUserMessage("ARBControllerpack", "", logstring1.c_str(), true, true, true, true, true);
			return;
		}
		std::string logstring;
		logstring = "Stand ";
		logstring += found2->second.number;
		logstring += " found.";
		LOG_F(INFO, logstring.c_str());
		found2->second.isAssigned = true;
		std::string code = found2->second.mAirlinecode;
		auto it = found2->second;
		if (code == "UAE" && standsUAE.find(dest) != standsUAE.end())
		{
			for (auto &temp : standsUAE.at(dest))
			{
				if (temp.number == it.number)
					temp.isAssigned = true;
			}
		}
		if (code == "ETD" && standsETD.find(dest) != standsETD.end())
		{
			for (auto &temp : standsETD.at(dest))
			{
				if (temp.number == it.number)
					temp.isAssigned = true;
			}
		}
		if ((code == "PAX" || code == "ABY") && standsPAX.find(dest) != standsPAX.end())
		{
			for (auto &temp : standsPAX.at(dest))
			{
				if (temp.number == it.number)
					temp.isAssigned = true;
			}
		}

		if ((code == "CARGO" || code == "CLC") && standsCARGO.find(dest) != standsCARGO.end())
		{
			for (auto &temp : standsCARGO.at(dest))
			{
				if (temp.number == it.number)
					temp.isAssigned = true;
			}
		}

		if ((code == "LWC" || code == "CLC") && standsLOWCOST.find(dest) != standsLOWCOST.end())

		{
			for (auto &temp : standsLOWCOST.at(dest))
			{
				if (temp.number == it.number)
					temp.isAssigned = true;
			}
		}
		if (code == "GA" && standsGA.find(dest) != standsGA.end())
		{
			for (auto &temp : standsGA.at(dest))
			{
				if (temp.number == it.number)
					temp.isAssigned = true;
			}
		}
		if (code == "VIP" && standsVIP.find(dest) != standsVIP.end())
		{
			for (auto &temp : standsVIP.at(dest))
			{
				if (temp.number == it.number)
					temp.isAssigned = true;
			}
		}
		if ((code == "ABY" || code == "PAX") && standsABY.find(dest) != standsABY.end())
		{
			for (auto &temp : standsABY.at(dest))
			{
				if (temp.number == it.number)
					temp.isAssigned = true;
			}
		}
		if (code == "CARGO1" && standsCargoSpecial.find(dest) != standsCargoSpecial.end())
		{
			for (auto &temp : standsCargoSpecial.at(dest))
			{
				if (temp.number == it.number)
					temp.isAssigned = true;
			}
		}
		if (code == "ALL" && standsOverflow.find(dest) != standsOverflow.end())
		{
			for (auto &temp : standsOverflow.at(dest))
			{
				if (temp.number == it.number)
					temp.isAssigned = true;
			}
		}
		auto found3 = standmapping.find(dest);
		std::unordered_map<std::string, Stand> copy;
		if (found3 != standmapping.end())
			copy = found3->second;
		std::pair<std::string, Stand> temp2(fp.GetCallsign(), found2->second);
		copy.insert(temp2);
		if (found3 == standmapping.end())
		{
			std::pair<std::string, std::unordered_map<std::string, Stand>> temp3(dest, copy);
			standmapping.insert(temp3);
			LOG_F(INFO, "Standmapping was empty");
		}
		else
		{
			standmapping.at(dest) = copy;
			LOG_F(INFO, "Standmapping was not empty");
		}
			
		
		
		remarks += "/STAND" + input;
		fpdata.SetRemarks(remarks.c_str());
		bool successful = fpdata.AmendFlightPlan();
		if(successful)
			LOG_F(INFO, "FP amend successful");
		else
			LOG_F(INFO, "FP amend NOT successful");
		break;


	}
	case TAG_FUNC_CLEAR:
	{
		auto dest = fp.GetFlightPlanData().GetDestination();
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "")!=0)
			return;
		LOG_F(INFO, "Clear Stand called on aircraft: ");
		LOG_F(INFO, fp.GetCallsign());
		if (std::find(activeAirports.begin(), activeAirports.end(), dest) == activeAirports.end()) return;
		auto airportstanddata = data.find(dest);
		auto temp1 = standmapping.find(dest);
		if (temp1 == standmapping.end()) return;
		auto copy = temp1->second;
		auto stand = copy.find(fp.GetCallsign());
		if (stand == copy.end()) return;
		auto fpdata = fp.GetFlightPlanData();
		LOG_F(INFO, "Standmapping size before clear: ");
		LOG_F(INFO, std::to_string(standmapping.at(dest).size()).c_str());
		copy.erase(fp.GetCallsign());
		standmapping.at(dest) = copy;
		LOG_F(INFO, "Standmapping size after clear: ");
		LOG_F(INFO, std::to_string(standmapping.at(dest).size()).c_str());
		std::string remarks = fpdata.GetRemarks();
		auto foundAirport = std::find(activeAirports.begin(), activeAirports.end(), dest);
		if (foundAirport != activeAirports.end())
		{
			for (auto re : foundAirport->m_standregex)
			{
				remarks = std::regex_replace(remarks, re, "");
			}
		}
		else
		{
			std::string logstring = "Couldn't clear stand for aircraft ";
			logstring += fp.GetCallsign();
			logstring += " because ";
			logstring += dest;
			logstring += " was not an active Airport. Ask Nils to add it to the code.";
			LOG_F(WARNING, logstring.c_str());
		}
		fpdata.SetRemarks(remarks.c_str());
		bool successful = fpdata.AmendFlightPlan();
		if (successful)
			LOG_F(INFO, "FP amend successful");
		else
			LOG_F(INFO, "FP amend NOT successful");

		return;
	}
	case TAG_FUNC_ASSIGN_POPUP:
	{
		OpenPopupList(Area, "Assign Stand", 1);
		auto dest = fp.GetFlightPlanData().GetDestination();

		AddPopupListElement("Assign Auto", "", TAG_FUNC_ASSIGN_AUTO);
		if (strcmp(dest, "OMDB") == 0)
			AddPopupListElement("Assign UAE", "", TAG_FUNC_ASSIGN_UAE);
		if (strcmp(dest, "OMSJ") == 0)
		{
			AddPopupListElement("Assign ABY", "", TAG_FUNC_ASSIGN_ABY);
			AddPopupListElement("Assign SQC/GEC/UPS", "", TAG_FUNC_ASSIGN_CARGO2);
		}
		if (strcmp(dest, "OMAA") == 0)
			AddPopupListElement("Assign ETD", "", TAG_FUNC_ASSIGN_ETD);
		AddPopupListElement("Assign CARGO", "", TAG_FUNC_ASSIGN_CARGO);
		AddPopupListElement("Assign PAX", "", TAG_FUNC_ASSIGN_PAX);
		AddPopupListElement("Assign LOWCOST", "", TAG_FUNC_ASSIGN_LOWCOST);
		AddPopupListElement("Assign VIP", "", TAG_FUNC_ASSIGN_VIP);
		AddPopupListElement("Assign GA", "", TAG_FUNC_ASSIGN_GA);
		AddPopupListElement("Clear", "", TAG_FUNC_CLEAR);
		LOG_F(INFO, "Opening stand assignment dialog.");
		break;
	}
	case TAG_FUNC_ASSIGN_AUTO:
	{
		LOG_F(INFO, "Auto assign selected.");
		std::string logstring = "Processing aircraft: ";
		logstring += fp.GetCallsign();
		LOG_F(INFO, logstring.c_str());
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		auto icao = fp.GetFlightPlanData().GetDestination();
		auto fpdata = fp.GetFlightPlanData();
		if (!this->isDestValid(fp.GetCallsign(), fpdata)) return;
		
		std::string callsign = fp.GetCallsign();
		std::string remarks = fp.GetFlightPlanData().GetRemarks();
		if (remarks.find("Cargo") != std::string::npos || remarks.find("CARGO") != std::string::npos || remarks.find("cargo") != std::string::npos || remarks.find("freight") != std::string::npos || remarks.find("Freight") != std::string::npos || remarks.find("FREIGHT") != std::string::npos)
		{
			std::string logstring;
			logstring = "Cargo Callsign through remarks for " + callsign + ", because remarks are " + remarks;
			LOG_F(INFO, logstring.c_str());
			goto CARGO;
		}
		if (callsign.length() < 3)
		{
			LOG_F(INFO, "Callsign less then 3 characters. Treathing as PAX.");
			goto PAX;
		}
		std::string op = callsign.substr(0, 3);
		std::regex number = std::regex(R"(.*\d.*)");
		std::smatch match;
		auto test = fp.GetFlightPlanData().GetPlanType();
		if (std::regex_search(op, match, number) || strcmp(test, "V") == 0)
		{
			std::string logstring;
			logstring = "Assigning GA stand for " + callsign + " because we found a number in the first three characters of the callsign or the flightrules are VFR.";
			LOG_F(INFO, logstring.c_str());
			goto GA;
		}
		auto found2 = callsignmap.find(icao);
		if (found2 == callsignmap.end()) return;
		auto found = found2->second.find(op);
		if (found != found2->second.end())
		{
			auto assignment = found->second;
			char L = 'L';
			auto wtc = fp.GetFlightPlanData().GetAircraftWtc();
			if (wtc == L && assignment != "VIP")
			{
				std::string logstring;
				logstring = "Detected GA Callsign " + callsign;
				LOG_F(INFO, logstring.c_str());
				goto GA;
			}
			if (assignment == "UAE")
			{
				std::regex uaecargo = std::regex(R"(UAE9\d{3})");
				std::smatch match;
				if (std::regex_search(callsign, match, uaecargo))
				{
					std::string logstring;
					logstring = "Detected SkyCargo for Callsign " + callsign;
					LOG_F(INFO, logstring.c_str());
					goto CARGO1;
				}
				std::string logstring;
				logstring = "Detected Emirates Callsign " + callsign;
				LOG_F(INFO, logstring.c_str());
				goto UAE;
			}
			if (assignment == "LWC")
			{
				std::string logstring;
				logstring = "Detected lowcost Callsign " + callsign;
				LOG_F(INFO, logstring.c_str());
				goto LWC;
			}
			if (assignment == "ETD")
			{
				std::string logstring;
				logstring = "Detected Etihad Callsign " + callsign;
				LOG_F(INFO, logstring.c_str());
				goto ETD;
			}
			if (assignment == "ABY")
			{
				std::string logstring;
				logstring = "Detected Air Arabia Callsign " + callsign;
				LOG_F(INFO, logstring.c_str());
				goto ABY;
			}
			if (assignment == "VIP")
			{
				std::string logstring;
				logstring = "Detected VIP Callsign " + callsign;
				LOG_F(INFO, logstring.c_str());
				goto VIP;
			}
			if (assignment == "CARGO")
			{
				std::string logstring;
				logstring = "Detected Cargo Callsign " + callsign;
				LOG_F(INFO, logstring.c_str());
				goto CARGO;
			}
			if (assignment == "CARGO1")
			{
				std::string logstring;
				logstring = "Detected Special Cargo Callsign " + callsign;
				LOG_F(INFO, logstring.c_str());
				goto CARGO1;
			}
			if (assignment == "GA")
			{
				std::string logstring;
				logstring = "Detected GA Callsign " + callsign;
				LOG_F(INFO, logstring.c_str());
				goto GA;
			}
		}
		logstring = "Could not find any rule to assign " + callsign + " so we treated it as a normal international carrier.";
		LOG_F(INFO, logstring.c_str());
		goto PAX;
		break;
	}
	case TAG_FUNC_ASSIGN_CARGO:
	{
	CARGO:
		LOG_F(INFO, "Cargo assignment in progress.");
		std::string logstring = "Processing aircraft: ";
		logstring += fp.GetCallsign();
		LOG_F(INFO, logstring.c_str());
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		auto icao = fp.GetFlightPlanData().GetDestination();
		auto fpdata = fp.GetFlightPlanData();
		std::string remarks = fpdata.GetRemarks();
		if (!this->isDestValid(fp.GetCallsign(), fpdata)) return;
		auto found = standmapping.find(icao);
		std::unordered_map<std::string, Stand> copy;
		if (found != standmapping.end())
		{
			LOG_F(INFO, "Standmapping for destination is not empty.");
			auto found2 = found->second.find(fp.GetCallsign());
			if (found2 != found->second.end())
				return;
			copy = standmapping.at(icao);
		}
		auto size = determineAircraftCat(fp);
		logstring.clear();
		logstring = "Aircraft parking code is ";
		logstring += size;
		LOG_F(INFO, logstring.c_str());
		auto standshere = standsCARGO.find(icao);
		if (standshere == standsCARGO.end()) break;
		auto stand = extractRandomStand(standshere->second, size, icao);
		if (stand.number == "Z00")
			break;
		logstring.clear();
		logstring = "Valid stand returned. It is ";
		logstring += stand.number;
		LOG_F(INFO, logstring.c_str());
		data.at(icao).at(stand.number).isAssigned = true;
		std::pair<std::string, Stand> temp(fp.GetCallsign(), stand);
		copy.insert(temp);
		if (found == standmapping.end())
		{
			LOG_F(INFO, "Standmapping for destination is empty.");
			std::pair<std::string, std::unordered_map<std::string, Stand>> temp2(icao, copy);
			standmapping.insert(temp2);
		}
		else
			standmapping.at(icao) = copy;
		for (auto &temp : standsCARGO.at(icao))
		{
			if (temp.number == stand.number)
			{
				LOG_F(INFO, "Stand found in CARGO list.");
				temp.isAssigned = true;
				break;
			}
		}
		for (auto &temp : standsLOWCOST.at(icao))
		{
			if (temp.number == stand.number)
			{
				LOG_F(INFO, "Stand found in LOWCOST list.");
				temp.isAssigned = true;
				break;
			}
		}
		remarks += "/STAND" + stand.number;
		fpdata.SetRemarks(remarks.c_str());
		bool successful = fpdata.AmendFlightPlan();
		if (successful)
			LOG_F(INFO, "FP amend successful");
		else
			LOG_F(INFO, "FP amend NOT successful");
		break;
		
	}
	case TAG_FUNC_ASSIGN_PAX:
	{
	PAX:
		LOG_F(INFO, "PAX assignment in progress.");
		std::string logstring = "Processing aircraft: ";
		logstring += fp.GetCallsign();
		LOG_F(INFO, logstring.c_str());
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		auto icao = fp.GetFlightPlanData().GetDestination();
		auto fpdata = fp.GetFlightPlanData();
		std::string remarks = fpdata.GetRemarks();
		if (!this->isDestValid(fp.GetCallsign(), fpdata)) return;
		auto found = standmapping.find(icao);
		std::unordered_map<std::string, Stand> copy;
		if (found != standmapping.end())
		{
			LOG_F(INFO, "Standmapping for destination is not empty.");
			auto found2 = found->second.find(fp.GetCallsign());
			if (found2 != found->second.end())
				break;
			copy = standmapping.at(icao);
		}
		auto size = determineAircraftCat(fp);
		logstring.clear();
		logstring = "Aircraft parking code is ";
		logstring += size;
		LOG_F(INFO, logstring.c_str());
		auto standshere = standsPAX.find(icao);
		if (standshere == standsPAX.end()) break;
		auto stand = extractRandomStand(standshere->second, size, icao);
		if (stand.number == "Z00")
			return;
		logstring.clear();
		logstring = "Valid stand returned. It is ";
		logstring += stand.number;
		LOG_F(INFO, logstring.c_str());
		data.at(icao).at(stand.number).isAssigned = true;
		std::pair<std::string, Stand> temp(fp.GetCallsign(), stand);
		copy.insert(temp);
		if (found == standmapping.end())
		{
			std::pair<std::string, std::unordered_map<std::string, Stand>> temp2(icao, copy);
			standmapping.insert(temp2);
		}
		else
			standmapping.at(icao) = copy;
		for (auto &temp : standsPAX.at(icao))
		{
			if (temp.number == stand.number)
				temp.isAssigned = true;
		}
		for (auto &temp : standsABY.at(icao))
		{
			if (temp.number == stand.number)
				temp.isAssigned = true;
		}
		
		remarks += "/STAND" + stand.number;
		fpdata.SetRemarks(remarks.c_str());
		bool successful = fpdata.AmendFlightPlan();
		if (successful)
			LOG_F(INFO, "FP amend successful");
		else
			LOG_F(INFO, "FP amend NOT successful");
		break;
	}
	case TAG_FUNC_ASSIGN_UAE:
	{
	UAE:
		LOG_F(INFO, "UAE assignment in progress.");
		std::string logstring = "Processing aircraft: ";
		logstring += fp.GetCallsign();
		LOG_F(INFO, logstring.c_str());
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		auto icao = fp.GetFlightPlanData().GetDestination();
		auto fpdata = fp.GetFlightPlanData();
		std::string remarks = fpdata.GetRemarks();
		if (!this->isDestValid(fp.GetCallsign(), fpdata)) return;
		auto found = standmapping.find(icao);
		std::unordered_map<std::string, Stand> copy;
		if (found != standmapping.end())
		{
			auto found2 = found->second.find(fp.GetCallsign());
			if (found2 != found->second.end())
				break;
			copy = standmapping.at(icao);
		}
		auto size = determineAircraftCat(fp);
		logstring.clear();
		logstring = "Aircraft parking code is ";
		logstring += size;
		LOG_F(INFO, logstring.c_str());
		auto standshere = standsUAE.find(icao);
		if (standshere == standsUAE.end()) break;
		auto stand = extractRandomStand(standshere->second, size, icao);
		if (stand.number == "Z00")
			break;
		logstring.clear();
		logstring = "Valid stand returned. It is ";
		logstring += stand.number;
		LOG_F(INFO, logstring.c_str());
		data.at(icao).at(stand.number).isAssigned = true;
		std::pair<std::string, Stand> temp(fp.GetCallsign(), stand);
		copy.insert(temp);
		if (found == standmapping.end())
		{
			std::pair<std::string, std::unordered_map<std::string, Stand>> temp2(icao, copy);
			standmapping.insert(temp2);
		}
		else
			standmapping.at(icao) = copy;
		for (auto &temp : standsUAE.at(icao))
		{
			if (temp.number == stand.number)
				temp.isAssigned = true;
		}
		
		remarks += "/STAND" + stand.number;
		fpdata.SetRemarks(remarks.c_str());
		bool successful = fpdata.AmendFlightPlan();
		if (successful)
			LOG_F(INFO, "FP amend successful");
		else
			LOG_F(INFO, "FP amend NOT successful");
		break;
	}
	case TAG_FUNC_ASSIGN_ABY:
	{
	ABY:
		LOG_F(INFO, "ABY assignment in progress.");
		std::string logstring = "Processing aircraft: ";
		logstring += fp.GetCallsign();
		LOG_F(INFO, logstring.c_str());
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		auto icao = fp.GetFlightPlanData().GetDestination();
		auto fpdata = fp.GetFlightPlanData();
		std::string remarks = fpdata.GetRemarks();
		if (!this->isDestValid(fp.GetCallsign(), fpdata)) return;
		auto found = standmapping.find(icao);
		std::unordered_map<std::string, Stand> copy;
		if (found != standmapping.end())
		{
			auto found2 = found->second.find(fp.GetCallsign());
			if (found2 != found->second.end())
				break;
			copy = standmapping.at(icao);
		}
		auto size = determineAircraftCat(fp);
		logstring.clear();
		logstring = "Aircraft parking code is ";
		logstring += size;
		LOG_F(INFO, logstring.c_str());
		auto standshere = standsABY.find(icao);
		auto standsherePAX = standsPAX.find(icao);
		std::vector<Stand> joined;
		joined.reserve(standshere->second.size() + standsherePAX->second.size());
		joined.insert(joined.end(), standshere->second.begin(), standshere->second.end());
		joined.insert(joined.end(), standsherePAX->second.begin(), standsherePAX->second.end());
		if (standshere == standsABY.end()) break;
		//ABY parks everywhere in sharjah
		auto stand = extractRandomStand(joined, size, icao);
		if (stand.number == "Z00")
			break;
		logstring.clear();
		logstring = "Valid stand returned. It is ";
		logstring += stand.number;
		LOG_F(INFO, logstring.c_str());
		data.at(icao).at(stand.number).isAssigned = true;
		std::pair<std::string, Stand> temp(fp.GetCallsign(), stand);
		copy.insert(temp);
		if (found == standmapping.end())
		{
			std::pair<std::string, std::unordered_map<std::string, Stand>> temp2(icao, copy);
			standmapping.insert(temp2);
		}
		else
			standmapping.at(icao) = copy;
		for (auto &temp : standsABY.at(icao))
		{
			if (temp.number == stand.number)
				temp.isAssigned = true;
		}
		for (auto &temp : standsPAX.at(icao))
		{
			if (temp.number == stand.number)
				temp.isAssigned = true;
		}

		remarks += "/STAND" + stand.number;
		fpdata.SetRemarks(remarks.c_str());
		bool successful = fpdata.AmendFlightPlan();
		if (successful)
			LOG_F(INFO, "FP amend successful");
		else
			LOG_F(INFO, "FP amend NOT successful");
		break;
	}
	case TAG_FUNC_ASSIGN_VIP:
	{
	VIP:
		LOG_F(INFO, "VIP assignment in progress.");
		std::string logstring = "Processing aircraft: ";
		logstring += fp.GetCallsign();
		LOG_F(INFO, logstring.c_str());
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		auto icao = fp.GetFlightPlanData().GetDestination();
		auto fpdata = fp.GetFlightPlanData();
		std::string remarks = fpdata.GetRemarks();
		if (!this->isDestValid(fp.GetCallsign(), fpdata)) return;
		auto found = standmapping.find(icao);
		std::unordered_map<std::string, Stand> copy;
		if (found != standmapping.end())
		{
			auto found2 = found->second.find(fp.GetCallsign());
			if (found2 != found->second.end())
				break;
			copy = standmapping.at(icao);
		}
		auto size = determineAircraftCat(fp);
		logstring.clear();
		logstring = "Aircraft parking code is ";
		logstring += size;
		LOG_F(INFO, logstring.c_str());
		auto standshere = standsVIP.find(icao);
		if (standshere == standsVIP.end()) break;
		auto stand = extractRandomStand(standshere->second, size, icao);
		if (stand.number == "Z00")
			break;
		logstring.clear();
		logstring = "Valid stand returned. It is ";
		logstring += stand.number;
		LOG_F(INFO, logstring.c_str());
		data.at(icao).at(stand.number).isAssigned = true;
		std::pair<std::string, Stand> temp(fp.GetCallsign(), stand);
		copy.insert(temp);
		if (found == standmapping.end())
		{
			std::pair<std::string, std::unordered_map<std::string, Stand>> temp2(icao, copy);
			standmapping.insert(temp2);
		}
		else
			standmapping.at(icao) = copy;
		for (auto &temp : standsVIP.at(icao))
		{
			if (temp.number == stand.number)
				temp.isAssigned = true;
		}

		remarks += "/STAND" + stand.number;
		fpdata.SetRemarks(remarks.c_str());
		bool successful = fpdata.AmendFlightPlan();
		if (successful)
			LOG_F(INFO, "FP amend successful");
		else
			LOG_F(INFO, "FP amend NOT successful");
		break;
	}
	case TAG_FUNC_ASSIGN_LOWCOST:
	{
	LWC:
		LOG_F(INFO, "LOWCOST assignment in progress.");
		std::string logstring = "Processing aircraft: ";
		logstring += fp.GetCallsign();
		LOG_F(INFO, logstring.c_str());
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		auto icao = fp.GetFlightPlanData().GetDestination();
		auto fpdata = fp.GetFlightPlanData();
		std::string remarks = fpdata.GetRemarks();
		if (!this->isDestValid(fp.GetCallsign(), fpdata)) return;
		auto found = standmapping.find(icao);
		std::unordered_map<std::string, Stand> copy;
		if (found != standmapping.end())
		{
			auto found2 = found->second.find(fp.GetCallsign());
			if (found2 != found->second.end())
				break;
			copy = standmapping.at(icao);
		}
		auto size = determineAircraftCat(fp);
		logstring.clear();
		logstring = "Aircraft parking code is ";
		logstring += size;
		LOG_F(INFO, logstring.c_str());
		auto standshere = standsLOWCOST.find(icao);
		if (standshere == standsLOWCOST.end()) break;
		auto stand = extractRandomStand(standshere->second, size, icao);
		if (stand.number == "Z00")
			break;
		logstring.clear();
		logstring = "Valid stand returned. It is ";
		logstring += stand.number;
		LOG_F(INFO, logstring.c_str());
		data.at(icao).at(stand.number).isAssigned = true;
		std::pair<std::string, Stand> temp(fp.GetCallsign(), stand);
		copy.insert(temp);
		if (found == standmapping.end())
		{
			std::pair<std::string, std::unordered_map<std::string, Stand>> temp2(icao, copy);
			standmapping.insert(temp2);
		}
		else
			standmapping.at(icao) = copy;
		for (auto &temp : standsCARGO.at(icao))
		{
			if (temp.number == stand.number)
				temp.isAssigned = true;
		}
		for (auto &temp : standsLOWCOST.at(icao))
		{
			if (temp.number == stand.number)
				temp.isAssigned = true;
		}

		remarks += "/STAND" + stand.number;
		fpdata.SetRemarks(remarks.c_str());
		bool successful = fpdata.AmendFlightPlan();
		if (successful)
			LOG_F(INFO, "FP amend successful");
		else
			LOG_F(INFO, "FP amend NOT successful");
		break;
	}
	case TAG_FUNC_ASSIGN_GA:
	{
	GA:
		LOG_F(INFO, "GA assignment in progress.");
		std::string logstring = "Processing aircraft: ";
		logstring += fp.GetCallsign();
		LOG_F(INFO, logstring.c_str());
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		auto icao = fp.GetFlightPlanData().GetDestination();
		auto fpdata = fp.GetFlightPlanData();
		std::string remarks = fpdata.GetRemarks();
		if (!this->isDestValid(fp.GetCallsign(), fpdata)) return;
		auto found = standmapping.find(icao);
		std::unordered_map<std::string, Stand> copy;
		if (found != standmapping.end())
		{
			auto found2 = found->second.find(fp.GetCallsign());
			if (found2 != found->second.end())
				break;
			copy = standmapping.at(icao);
		}
		auto size = determineAircraftCat(fp);
		logstring.clear();
		logstring = "Aircraft parking code is ";
		logstring += size;
		LOG_F(INFO, logstring.c_str());
		auto standshere = standsGA.find(icao);
		if (standshere == standsGA.end()) break;
		auto stand = extractRandomStand(standshere->second, size, icao);
		if (stand.number == "Z00")
			break;
		logstring.clear();
		logstring = "Valid stand returned. It is ";
		logstring += stand.number;
		LOG_F(INFO, logstring.c_str());
		data.at(icao).at(stand.number).isAssigned = true;
		std::pair<std::string, Stand> temp(fp.GetCallsign(), stand);
		copy.insert(temp);
		if (found == standmapping.end())
		{
			std::pair<std::string, std::unordered_map<std::string, Stand>> temp2(icao, copy);
			standmapping.insert(temp2);
		}
		else
			standmapping.at(icao) = copy;
		for (auto &temp : standsGA.at(icao))
		{
			if (temp.number == stand.number)
				temp.isAssigned = true;
		}

		remarks += "/STAND" + stand.number;
		fpdata.SetRemarks(remarks.c_str());
		bool successful = fpdata.AmendFlightPlan();
		if (successful)
			LOG_F(INFO, "FP amend successful");
		else
			LOG_F(INFO, "FP amend NOT successful");
		break;
	}
	case TAG_FUNC_ASSIGN_ETD:
	{
	ETD:
		LOG_F(INFO, "ETD assignment in progress.");
		std::string logstring = "Processing aircraft: ";
		logstring += fp.GetCallsign();
		LOG_F(INFO, logstring.c_str());
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		auto icao = fp.GetFlightPlanData().GetDestination();
		auto fpdata = fp.GetFlightPlanData();
		std::string remarks = fpdata.GetRemarks();
		if (!this->isDestValid(fp.GetCallsign(), fpdata)) return;
		auto found = standmapping.find(icao);
		std::unordered_map<std::string, Stand> copy;
		if (found != standmapping.end())
		{
			auto found2 = found->second.find(fp.GetCallsign());
			if (found2 != found->second.end())
				break;
			copy = standmapping.at(icao);
		}
		auto size = determineAircraftCat(fp);
		logstring.clear();
		logstring = "Aircraft parking code is ";
		logstring += size;
		LOG_F(INFO, logstring.c_str());
		auto standshere = standsETD.find(icao);
		if (standshere == standsETD.end()) break;
		auto stand = extractRandomStand(standshere->second, size, icao);
		if (stand.number == "Z00")
			break;
		logstring.clear();
		logstring = "Valid stand returned. It is ";
		logstring += stand.number;
		LOG_F(INFO, logstring.c_str());
		data.at(icao).at(stand.number).isAssigned = true;
		std::pair<std::string, Stand> temp(fp.GetCallsign(), stand);
		copy.insert(temp);
		if (found == standmapping.end())
		{
			std::pair<std::string, std::unordered_map<std::string, Stand>> temp2(icao, copy);
			standmapping.insert(temp2);
		}
		else
			standmapping.at(icao) = copy;
		for (auto &temp : standsETD.at(icao))
		{
			if (temp.number == stand.number)
				temp.isAssigned = true;
		}

		remarks += "/STAND" + stand.number;
		fpdata.SetRemarks(remarks.c_str());
		bool successful = fpdata.AmendFlightPlan();
		if (successful)
			LOG_F(INFO, "FP amend successful");
		else
			LOG_F(INFO, "FP amend NOT successful");
		break;
	}
	case TAG_FUNC_ASSIGN_CARGO2:
	{
	CARGO1:
		LOG_F(INFO, "Special Cargo assignment in progress.");
		std::string logstring = "Processing aircraft: ";
		logstring += fp.GetCallsign();
		LOG_F(INFO, logstring.c_str());
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		auto icao = fp.GetFlightPlanData().GetDestination();
		auto fpdata = fp.GetFlightPlanData();
		std::string remarks = fpdata.GetRemarks();
		if (!this->isDestValid(fp.GetCallsign(), fpdata)) return;
		auto found = standmapping.find(icao);
		std::unordered_map<std::string, Stand> copy;
		if (found != standmapping.end())
		{
			auto found2 = found->second.find(fp.GetCallsign());
			if (found2 != found->second.end())
				break;
			copy = standmapping.at(icao);
		}
		auto size = determineAircraftCat(fp);
		logstring.clear();
		logstring = "Aircraft parking code is ";
		logstring += size;
		LOG_F(INFO, logstring.c_str());
		auto standshere = standsCargoSpecial.find(icao);
		if (standshere == standsCargoSpecial.end()) break;
		auto stand = extractRandomStand(standshere->second, size, icao);
		if (stand.number == "Z00")
			break;
		logstring.clear();
		logstring = "Valid stand returned. It is ";
		logstring += stand.number;
		LOG_F(INFO, logstring.c_str());
		data.at(icao).at(stand.number).isAssigned = true;
		std::pair<std::string, Stand> temp(fp.GetCallsign(), stand);
		copy.insert(temp);
		if (found == standmapping.end())
		{
			std::pair<std::string, std::unordered_map<std::string, Stand>> temp2(icao, copy);
			standmapping.insert(temp2);
		}
		else
			standmapping.at(icao) = copy;
		for (auto &temp : standsVIP.at(icao))
		{
			if (temp.number == stand.number)
				temp.isAssigned = true;
		}

		remarks += "/STAND" + stand.number;
		fpdata.SetRemarks(remarks.c_str());
		bool successful = fpdata.AmendFlightPlan();
		if (successful)
			LOG_F(INFO, "FP amend successful");
		else
			LOG_F(INFO, "FP amend NOT successful");
		break;
	}
	case TAG_FUNC_ROUTING_OPT:
	{
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		std::string handler = "Routing info for ";
		handler += fp.GetCallsign();
		auto cdata = fp.GetControllerAssignedData();
		std::string flightstrip = cdata.GetFlightStripAnnotation(3);
		auto data = splitStringAtDelimiter(flightstrip, ',');
		if (data.size() != 2)
		{
			std::string logstring = "Somehow the flightstrip did not contain the data we expected. We got " + flightstrip +" for aircraft ";
			logstring += fp.GetCallsign();
			LOG_F(WARNING, logstring.c_str());
			return;
		}
		std::string message = "Routing is invalid after " + *data.begin() + " in " + *data.rbegin() + " FIR. Valid routes from " + *data.begin();
		auto currentFIR = allFIRs.find(*data.rbegin());
		if (currentFIR == allFIRs.end()) return;
		DisplayUserMessage(handler.c_str(), "ARBControllerPack", message.c_str(), true, true, true, true, false);
		auto messageRoutes = currentFIR->second.getAllRoutesfromCOPN(*data.begin());
		for (auto& temp : messageRoutes)
		{
			std::string tempMessage = temp.routingATS;
			switch (temp.m_type)
			{
			case 0: { tempMessage += ". Valid for transits."; break; }
			case 1: { tempMessage += ". Valid for arrivals into "; tempMessage += temp.m_copx; break; }
			case 2: { tempMessage += ". Valid for departures from "; tempMessage += temp.m_copn; break; }
			}
			tempMessage += " Restrictions: ";
			if (!temp.levelrestriction.empty())
				tempMessage += "Level restrictions:";
			for (auto& elem : temp.levelrestriction)
			{
				tempMessage += " ";
				tempMessage += elem;
				
			}
			tempMessage += "; ";
			if (!temp.notForArrivalInto.empty())
			{
				tempMessage += "Not for destinations:";
				for (auto& elem : temp.notForArrivalInto)
				{
					tempMessage += " ";
					tempMessage += elem;
				}
				tempMessage += "; ";
			}
			if (!temp.notforDepFrom.empty())
			{
				tempMessage += "Not for departures from:";
				for (auto& elem : temp.notforDepFrom)
				{
					tempMessage += " ";
					tempMessage += elem;
				}
				tempMessage += "; ";
			}
			if (!temp.onlyForArrivalInto.empty())
			{
				tempMessage += "Only for destinations:";
				for (auto& elem : temp.onlyForArrivalInto)
				{
					tempMessage += " ";
					tempMessage += elem;
				}
				tempMessage += "; ";
			}
			if (!temp.onlyforDepFrom.empty())
			{
				tempMessage += "Only for departures from:";
				for (auto& elem : temp.onlyforDepFrom)
				{
					tempMessage += " ";
					tempMessage += elem;
				}
				tempMessage += "; ";
			}
			DisplayUserMessage(handler.c_str(), "ARBControllerPack", tempMessage.c_str(), true, true, true, true, false);
		}
		
		break;
	}
	case TAG_FUNC_ROUTING:
	{
		if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerCallsign(), "") != 0)
			break;
		std::string handler = "Routing suggestion for ";
		std::string message;
		handler += fp.GetCallsign();
		auto cdata = fp.GetControllerAssignedData();
		std::string flightstrip = cdata.GetFlightStripAnnotation(3);
		auto data = splitStringAtDelimiter(flightstrip, ',');
		if (data.size() != 2)
		{
			std::string logstring = "Somehow the flightstrip did not contain the data we expected. We got " + flightstrip + " for aircraft ";
			logstring += fp.GetCallsign();
			LOG_F(WARNING, logstring.c_str());
			return;
		}
		std::string dep, dest;
		dep = fp.GetFlightPlanData().GetOrigin();
		dest = fp.GetFlightPlanData().GetDestination();
		auto currentFIR = allFIRs.find(*data.rbegin());
		if (currentFIR == allFIRs.end()) return;
		auto copn = *data.begin();
		std::string filedCOPX;
		auto copxs = currentFIR->second.COPXs;
		auto copns = currentFIR->second.COPNs;
		auto routesCOPN = currentFIR->second.getAllRoutesfromCOPN(copn);
		auto routesDEP = currentFIR->second.getAllRoutesfromCOPN(dep);
		auto routePoints = getRoutePoints(fp.GetExtractedRoute());
		//case error is in arriving FIR
		if (copxs.find(dest) != copxs.end())
		{
			for (auto& elem : routesCOPN)
			{
				if (elem.m_type == 1 && elem.getCOPX() == dest)
				{
					message = "Flights from " + copn + " to " + dest + " shall use ";
					message += elem.routingATS + " in "+ *data.rbegin() + " FIR.";
					DisplayUserMessage(handler.c_str(), "ARBControllerPack", message.c_str(), true, true, true, true, false);
					return;
				}
			}
			auto routesToDest = currentFIR->second.getAllRoutesToCOPX(dest);
			message = "Flights to " + dest + " shall route via one of the options below in "+ *data.rbegin() + " FIR: ";
			DisplayUserMessage(handler.c_str(), "ARBControllerPack", message.c_str(), true, true, true, true, false);
			for (auto& elem : routesToDest)
			{
				if (std::find(elem.notforDepFrom.begin(), elem.notforDepFrom.end(), dep) != elem.notforDepFrom.end())
					continue;
				message = elem.routingATS;
				DisplayUserMessage(handler.c_str(), "ARBControllerPack", message.c_str(), true, true, true, true, false);
				
			}
			return;
		}
		//check if a copx has been filed and if yes extract it
		auto itcopn = std::find(routePoints.begin(), routePoints.end(), copn);
		for (itcopn; itcopn != routePoints.end(); itcopn++)
		{
			auto found = std::find(copxs.begin(), copxs.end(), *itcopn);
			if (found != copxs.end())
			{
				filedCOPX = *found;
				break;
			}

		}
		//case error is in departing FIR
		if (copns.find(dep) != copns.end())
		{
			if (!filedCOPX.empty())
			{
				for (auto& elem : routesDEP)
				{
					if (elem.m_type == 2 && elem.getCOPX() == filedCOPX)
					{
						message = "Flights from " + dep + " to " + filedCOPX + " shall use ";
						message += elem.routingATS + " in " + *data.rbegin() + " FIR.";
						DisplayUserMessage(handler.c_str(), "ARBControllerPack", message.c_str(), true, true, true, true, false);
						return;
					}
				}
			}
			message = "Flights from " + dep + " shall route via one of the options below in " + *data.rbegin() + " FIR: ";
			DisplayUserMessage(handler.c_str(), "ARBControllerPack", message.c_str(), true, true, true, true, false);
			for (auto& elem : routesDEP)
			{
				if (std::find(elem.onlyForArrivalInto.begin(), elem.onlyForArrivalInto.end(), dest) != elem.onlyForArrivalInto.end())
					continue;
				DisplayUserMessage(handler.c_str(), "ARBControllerPack",elem.routingATS.c_str(), true, true, true, true, false);
			}
			
			return;
		}
		//case transit error
		if (!filedCOPX.empty())
		{
			for (auto& elem : routesCOPN)
			{
				if (elem.m_type == 0 && elem.getCOPX() == filedCOPX)
				{
					message = "Flights from " + copn + " to " + filedCOPX + " shall use ";
					message += elem.routingATS + " in " + *data.rbegin() + " FIR.";
					DisplayUserMessage(handler.c_str(), "ARBControllerPack", message.c_str(), true, true, true, true, false);
					return;
				}
			}
			DisplayUserMessage(handler.c_str(), "ARBControllerPack", message.c_str(), true, true, true, true, false);
					return;
		}
		message = "Flights from " + copn + " shall route via one of the options below in " + *data.rbegin() + " FIR: ";
		DisplayUserMessage(handler.c_str(), "ARBControllerPack", message.c_str(), true, true, true, true, false);
		for (auto& elem : routesDEP)
		{
			if (std::find(elem.onlyForArrivalInto.begin(), elem.onlyForArrivalInto.end(), dest) != elem.onlyForArrivalInto.end())
				continue;
			message = elem.routingATS;
			DisplayUserMessage(handler.c_str(), "ARBControllerPack", message.c_str(), true, true, true, true, false);
			return;
		}
		message = "Smart suggestions for fixing the route failed. Please use your own brain by selecting an option from the routematrix or from the list that appears by right clicking the error symbol in the flightplan list.";
		DisplayUserMessage(handler.c_str(), "ARBControllerPack", message.c_str(), true, true, true, true, false);
		return;
	}
	
	}// switch by the function ID
}
//--------------------------Helper functions-----------------------------------------
std::vector<Stand> CUAEController::getStandOfAircraft(EuroScopePlugIn::CPosition position)
{
	std::vector<Stand> returnvalue;
	returnvalue.push_back(Stand("ZZZ", "N000-00-00.0", "E000-00-00.0", "PAX", "", "", "F", "yes", "OMDB"));
	for (auto airport : activeAirports)
	{
		auto found = data.find(airport.m_icao);
		if (found == data.end()) continue;
		for (auto stand : found->second)
		{
			auto distance = position.DistanceTo(stand.second.position);
			if (distance < TOL)
			{
				returnvalue.push_back(stand.second);
			}
				
			if (distance > 5)
				break;
		}
	}
	return returnvalue;
}
void CUAEController::cleanupStands()
{
	int i = 0;
	int j = 0;
	for (auto airport : activeAirports)
	{
		for (auto& it : data.at(airport.m_icao))
		{
			

				if (!it.second.isEmpty)
				{
					auto first = RadarTargetSelectFirst();
					if (first.GetPosition().GetPosition().DistanceTo(it.second.position) < TOL)
						goto outer2;
					auto temp = RadarTargetSelectNext(first);
					while (temp.IsValid())
					{
						if (temp.GetPosition().GetPosition().DistanceTo(it.second.position) < TOL)
						{
							goto outer2;
						}

						temp = RadarTargetSelectNext(temp);
					}
					i++;
					it.second.isEmpty = true;
					std::string logstring = "Stand ";
					logstring += it.second.number;
					logstring += " is now empty again.";
					LOG_F(INFO, logstring.c_str());
				}
				if (it.second.isAssigned)
				{
					
						auto first = FlightPlanSelectFirst();

						std::string remarks = first.GetFlightPlanData().GetRemarks();
						std::smatch match;
						for (auto regex : airport.m_standregex)
						{
							if (std::regex_search(remarks, match, regex))
							{
								if (it.second.number == match.str(1))
									goto outer2;
							}
						}
						auto temp = FlightPlanSelectNext(first);
						while (temp.IsValid())
						{
							remarks = temp.GetFlightPlanData().GetRemarks();
							for (auto regex : airport.m_standregex)
							{
								if (std::regex_search(remarks, match, regex))
								{
									if (it.second.number == match.str(1))
										goto outer2;
								}
							}
							temp = FlightPlanSelectNext(temp);
						}
						it.second.isAssigned = false;
						j++;
						std::string logstring = "Stand ";
						logstring += it.second.number;
						logstring += " no longer assigned. Removing it...";
						LOG_F(INFO, logstring.c_str());
						for (auto &temp : standmapping.at(airport.m_icao))
						{
							if (temp.second.number == it.second.number)
							{
								standmapping.at(airport.m_icao).erase(temp.first);
							}
								
						}
						std::string code = it.second.mAirlinecode;
						if (code == "UAE")
						{
							for (auto &temp : standsUAE.at(airport.m_icao))
							{
								if (temp.number == it.second.number)
									temp.isAssigned = false;
							}
						}
						if (code == "ETD")
						{
							for (auto &temp : standsETD.at(airport.m_icao))
							{
								if (temp.number == it.second.number)
									temp.isAssigned = false;
							}
						}
						if (code == "PAX" || code == "ABY")
						{
							for (auto &temp : standsPAX.at(airport.m_icao))
							{
								if (temp.number == it.second.number)
									temp.isAssigned = false;
							}
						}
						if (code == "ABY" || code == "PAX")
						{
							for (auto &temp : standsABY.at(airport.m_icao))
							{
								if (temp.number == it.second.number)
									temp.isAssigned = false;
							}
						}
						if (code == "CARGO1")
						{
							for (auto &temp : standsCargoSpecial.at(airport.m_icao))
							{
								if (temp.number == it.second.number)
									temp.isAssigned = false;
							}
						}

						if (code == "CARGO" || code == "CLC")
						{
							for (auto &temp : standsCARGO.at(airport.m_icao))
							{
								if (temp.number == it.second.number)
									temp.isAssigned = false;
							}
						}

						if (code == "LWC" || code == "CLC")

						{
							for (auto &temp : standsLOWCOST.at(airport.m_icao))
							{
								if (temp.number == it.second.number)
									temp.isAssigned = false;
							}
						}
						if (code == "GA")
						{
							for (auto &temp : standsGA.at(airport.m_icao))
							{
								if (temp.number == it.second.number)
									temp.isAssigned = false;
							}
						}
						if (code == "VIP")
						{
							for (auto &temp : standsVIP.at(airport.m_icao))
							{
								if (temp.number == it.second.number)
									temp.isAssigned = false;
							}
						}
						if (code == "ALL")
						{
							for (auto &temp : standsOverflow.at(airport.m_icao))
							{
								if (temp.number == it.second.number)
									temp.isAssigned = false;
							}
						}
					

				}
			outer2:
				continue;
			
		}
		
		
		
	}

	std::string logstring = "CleanupStands finished with: ";
	logstring += std::to_string(i);
	logstring += " new empty stands and ";
	logstring += std::to_string(j);
	logstring += " no longer assigned stands";
	LOG_F(INFO, logstring.c_str());
	return;
}
Stand CUAEController::extractRandomStand(std::vector<Stand> stands, char size, std::string icao)
{
	LOG_F(INFO, "Extracting stand...");
	LOG_F(INFO, "Following stands are set as occupied or already assigned: ");
	std::string logstring;
	for (auto element : stands)
	{
		if (element.isAssigned || !element.isEmpty)
		{
			logstring += element.number;
			logstring += " ";
		}
	}
	LOG_F(INFO, logstring.c_str());
	std::shuffle(std::begin(stands), std::end(stands), std::random_device());
	for (auto stand : stands)
	{
		if (!stand.isAssigned && stand.isEmpty && stand.isInFlytampa && stand.mSize >= size)
		{
			return stand;
		}

	}
	std::shuffle(std::begin(standsOverflow.at(icao)), std::end(standsOverflow.at(icao)), std::random_device());
	for (auto stand : standsOverflow.at(icao))
	{
		if (!stand.isAssigned && stand.isEmpty && stand.isInFlytampa && stand.mSize >= size)
		{
			return stand;
		}
	}
	DisplayUserMessage("StandAssigner", icao.c_str(), "Error assigning stand", true, true, true, true, true);
	auto errorval = standsOverflow.at("OMDB").at(0);
	errorval.number = "Z00";
	return errorval;
}
char CUAEController::determineAircraftCat(EuroScopePlugIn::CFlightPlan fp)
{
	char wtc = fp.GetFlightPlanData().GetAircraftWtc();
	std::string actype = fp.GetFlightPlanData().GetAircraftFPType();
	auto found = parkingdict.find(actype);
	if (found != parkingdict.end())
		return found->second.c_str()[0];
	else {
		if (wtc == 'L')
			return 'B';
		if (wtc == 'J')
			return 'F';
		if (wtc == 'H')
			return 'E';
		if (wtc == 'M')
			return 'C';
		LOG_F(ERROR, "Something went very wrong. Aircraft WTC could not be determined.");
		return 'E';
	}
}
void CUAEController::readStandFile(std::string dir, std::string airport)
{
	io::CSVReader<8, io::trim_chars<' '>, io::no_quote_escape<','>> in(dir);
	in.read_header(io::ignore_extra_column, "Standnumber", "latitude", "longitude", "airlinecode", "neighbor1", "neighbor2", "size", "flytampa");
	std::string StandNumber, lat, Long, airlinecode, neighbor1, neighbor2, size, flytampa;
	std::unordered_map<std::string, Stand> thisdata;
	while (in.read_row(StandNumber, lat, Long, airlinecode, neighbor1, neighbor2, size, flytampa))
	{
		Stand temp = Stand(StandNumber, lat, Long, airlinecode, neighbor1, neighbor2, size, flytampa, airport);
		std::pair<std::string, Stand> temp2(StandNumber, temp);
		thisdata.insert(temp2);
	}
	std::pair<std::string, std::unordered_map<std::string, Stand>> temp3(airport, thisdata);
	data.insert(temp3);
	std::string logstring;
	logstring = "Stand file read for ";
	logstring += airport;
	LOG_F(INFO, logstring.c_str());
}
void CUAEController::readCallsignFile(std::string dir, std::string airport)
{
	//reading callsign mapping
	io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<','>> in(dir);
	in.read_header(io::ignore_extra_column, "Callsign", "ToAssign");
	std::string Callsign, ToAssign;
	std::unordered_map<std::string, std::string> thiscallsignmap;
	while (in.read_row(Callsign, ToAssign))
	{
		std::pair<std::string, std::string> temp3(Callsign, ToAssign);
		thiscallsignmap.insert(temp3);
	}
	std::pair<std::string, std::unordered_map<std::string, std::string>> temp(airport, thiscallsignmap);
	callsignmap.insert(temp);
	std::string logstring;
	logstring = "Callsignmapping file read for ";
	logstring += airport;
	LOG_F(INFO, logstring.c_str());
}
void markStandsasOccupied(Stand mystand,std::string code, std::string icao)
{
	if (code == "UAE")
	{
		for (auto &temp : standsUAE.at(icao))
		{
			if (temp.number == mystand.number)
				temp.isEmpty = false;
		}
	}
	if (code == "ETD")
	{
		for (auto &temp : standsETD.at(icao))
		{
			if (temp.number == mystand.number)
				temp.isEmpty = false;
		}
	}
	if (code == "ABY" || code == "PAX")
	{
		for (auto &temp : standsABY.at(icao))
		{
			if (temp.number == mystand.number)
				temp.isEmpty = false;
		}
	}
	if (code == "PAX" || code == "ABY")
	{
		for (auto &temp : standsPAX.at(icao))
		{
			if (temp.number == mystand.number)
				temp.isEmpty = false;
		}
	}
	if (code == "CARGO1")
	{
		for (auto &temp : standsCargoSpecial.at(icao))
		{
			if (temp.number == mystand.number)
				temp.isEmpty = false;
		}
	}
	if (code == "CARGO" || code == "CLC")
	{
		for (auto &temp : standsCARGO.at(icao))
		{
			if (temp.number == mystand.number)
				temp.isEmpty = false;
		}
	}

	if (code == "LWC" || code == "CLC")
	{
		for (auto &temp : standsLOWCOST.at(icao))
		{
			if (temp.number == mystand.number)
				temp.isEmpty = false;
		}
	}
	if (code == "GA")
	{
		for (auto &temp : standsGA.at(icao))
		{
			if (temp.number == mystand.number)
				temp.isEmpty = false;
		}
	}
	if (code == "VIP")
	{
		for (auto &temp : standsVIP.at(icao))
		{
			if (temp.number == mystand.number)
				temp.isEmpty = false;
		}
	}
	if (code == "ALL")
	{
		for (auto &temp : standsOverflow.at(icao))
		{
			if (temp.number == mystand.number)
				temp.isEmpty = false;
		}
	}
}
bool CUAEController::isDestValid(std::string callsign,EuroScopePlugIn::CFlightPlanData data)
{
	auto foundAirport = std::find(activeAirports.begin(), activeAirports.end(), data.GetDestination());
	if (foundAirport == activeAirports.end())
	{
		std::string logstring = "Couldn't assign stand for aircraft ";
		logstring += callsign;
		logstring += " because ";
		logstring += data.GetDestination();
		logstring += " was not an active Airport. Ask Nils to add it to the code.";
		std::string displaystring = "An error occured during stand assignment. The aircraft ";
		displaystring += callsign;
		displaystring += " had a non configured destination ";
		displaystring += data.GetDestination();
		displaystring += " . Report this to Suprojit Paul or Nils Dornbusch.";
		DisplayUserMessage("ARBControllerPack", "", displaystring.c_str(), true, true, true, true, true);
		LOG_F(WARNING, logstring.c_str());
		return false;
	}
	else
	{
		LOG_F(INFO, "Destination is valid!");
		std::string remarks = data.GetRemarks();
		for (auto re : foundAirport->m_standregex)
		{
			std::smatch match;
			if (std::regex_search(remarks, match, re))
			{
				LOG_F(WARNING, "Aircraft not editable.");
				return false;
			}
		}
		return true;
	}
	
		
}
void WayPointNotFound(std::string name)
{
	std::string logstring = "Could not find ";
	logstring += name;
	LOG_F(ERROR, logstring.c_str());
	//throw std::invalid_argument(logstring.c_str());
}
void AirwayWaypointConnectionNotFound(std::string pointname, std::string airwayname)
{
	std::string logstring = "Airway/Waypoint mismatch with fix " + pointname + " and airway " + airwayname;
	LOG_F(ERROR, logstring.c_str());
	//throw std::invalid_argument(logstring.c_str());
}

std::vector<Waypoint>  parseATSPointsFromString(std::string route, std::vector<Waypoint> correctPoints)
{
	std::vector<Waypoint> points;
	std::stringstream ss(route);
	std::string buf;
	std::vector<std::string> atsrouting;
	std::vector<std::string> possibleDiff;
	std::vector<std::string> avoidPoint;
	std::regex ruleD("\\\d");
	//split string into vector on space
	while (ss >> buf)
		atsrouting.push_back(buf);
	auto currentPoint = atsrouting.begin();
	auto cP = fixes.find_waypoint(*currentPoint);
	points.push_back(cP);
	if (!correctPoints.empty() && cP.m_name != correctPoints.begin()->m_name)
		return points;
	
	auto correctCP = correctPoints.begin();
	if (!correctPoints.empty())
		correctCP++;
	while (atsrouting.size()>1 &&currentPoint != (atsrouting.end() - 1))
	{	
		std::string airway = *(currentPoint + 1);
		if (currentPoint + 2 == atsrouting.end())
		{
			
		}
		std::string nextPoint = *(currentPoint + 2);
		std::string searchPoint = *currentPoint;
		auto cP = fixes.find_waypoint(*currentPoint);
		Waypoint sP = cP;
		if (airway == "DCT")
		{
			auto fix = fixes.find_waypoint(nextPoint);
			if (fix.m_name == "ERROR")
			{
				if (std::find(WaypointErrors.begin(), WaypointErrors.end(), nextPoint) == WaypointErrors.end())
				{
					WaypointErrors.push_back(nextPoint);
					std::string logstring("Could not find " + nextPoint + " in database. Treating it like a waypoint without airway connections.");
					LOG_F(INFO, logstring.c_str());
					
				}
				fix.m_name = nextPoint;
			}
			points.push_back(fix);
			//if we have a deviation from the correct points right here already or reached the end of the required routing we exit.
			if (!correctPoints.empty() && (correctCP == correctPoints.end() || fix.m_name != correctCP->m_name))
				return points;
			if (!correctPoints.empty())
				correctCP++;
			currentPoint += 2;
			continue;
		}
		//cater for the case of implicit direct
		else if (!std::regex_search(airway.begin(), airway.end(), ruleD))
		{
			nextPoint = airway;
			auto fix = fixes.find_waypoint(nextPoint);
			if (fix.m_name == "ERROR")
			{
				std::string logstring("Could not find " + nextPoint + " in database. Treating it like a waypoint without airway connections.");
				LOG_F(INFO, logstring.c_str());
				fix.m_name = nextPoint;
			}
			
			points.push_back(fix);
			if (!correctPoints.empty() && (correctCP == correctPoints.end() || fix.m_name != correctCP->m_name))
				return points;
			if (!correctPoints.empty())
				correctCP++;
			currentPoint += 1;
			continue;
		}
		while (searchPoint != nextPoint)
		{
			auto pointsOnAirway = sP.getNextPointNameOnAirway(airway);
			if (pointsOnAirway.size() > 1)
			{
				
				for (auto elem : pointsOnAirway)
				{
					sP = fixes.find_waypoint(elem);
					if (sP.m_name == "ERROR")
						WayPointNotFound(elem);
					//only on airway change two valid paths from one intersection
					if (searchPoint != *currentPoint)
					{
						if (points.size() == 1 && points.back() == sP)
							continue;
						else if (points.size() > 1 && *(points.end() - 2) == sP)
							continue;
						else 
						{
							searchPoint = elem;
							break;
						}
					}
					else
					{
						auto foundaP = std::find(avoidPoint.begin(), avoidPoint.end(), elem);
						if (points.size() == 1 && points.back() == sP)
							continue;
						if (avoidPoint.size() > 10)
							AirwayWaypointConnectionNotFound(searchPoint,airway);
						else if (points.size() > 1 && ( * (points.end() - 2) == sP)|| foundaP != avoidPoint.end())
							continue;
						else 
						{
							possibleDiff.push_back(searchPoint);
							searchPoint = elem;
							break;
						}						
					}
										
				}

				points.push_back(sP);
				if (!correctPoints.empty() && (correctCP == correctPoints.end()))
					return points;
				if (!correctPoints.empty())
					correctCP++;
			}
			else if (pointsOnAirway.size() == 1 && pointsOnAirway.back() != "ERROR" && (points.size() == 0 || points.size() ==1 || (points.size()>1 && points.rbegin()[1] != pointsOnAirway.back())))
			{
				searchPoint = pointsOnAirway.back();
				sP = fixes.find_waypoint(searchPoint);
				if (sP.m_name == "ERROR") 
				{
					WayPointNotFound(searchPoint);
					return points;
				}
					
				points.push_back(sP);
				if (!correctPoints.empty() && (correctCP == correctPoints.end() ||sP.m_name != correctCP->m_name))
					return points;
				if(!correctPoints.empty())
					correctCP++;
			}
			else
			{
				if (possibleDiff.empty())
				{
					AirwayWaypointConnectionNotFound(points.back().m_name, airway);
					return points;
				}
					
				auto found = std::find(points.begin(), points.end(), possibleDiff.back());
				avoidPoint.push_back((found + 1)->m_name);
				points.erase(found + 1, points.end());
				searchPoint = possibleDiff.back();
				possibleDiff.pop_back();
				sP = fixes.find_waypoint(searchPoint);
			}
		}
		currentPoint += 2;
	}
	
	return points;
}
std::vector<std::string> splitStringAtDelimiter(std::string string, char delimiter)
{
	std::vector<std::string> result;
	if (string != "NONE")
	{
		std::stringstream ss(string);
		std::string item;
		while (std::getline(ss, item, delimiter))
			result.push_back(item);
	}
	
	return result;
}