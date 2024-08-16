//Parameter Class

//#pragma once

#ifndef PARAMETERS_H
#define PARAMETERS_H

//#include <cstdio>
//#include <memory>
//#include <vector>
//#include <cmath>
//#include "minwindef.h"
//
//#include <cstring>
//#include <iostream>
//
//#include "winuser.h"
//#include "winreg.h"

#ifdef _DEBUG
#include "DBG_print.h"
#endif


#include "Xml.h"  // has define for Ctring
#include "tinyxml2.h"
#include "myHelper.h"

//using namespace tinyxml2;
//using namespace std;

#define	MY_HEX_BUF_LEN	12

class Parameters
{
public:
	explicit Parameters(){
		mySet_mskEHM = 0;
	};

private:
	bool       bSimulate  = false;
	bool       bReverseX  = false;
	bool       bReverseY  = false;
	bool       bAutoSave  = false;
	bool       bAutoClose = false;
	

	bool       bColorByTwitch = true;      //turn on for EHM
	bool       bColorByBase   = false;     //turn on for ECT


	bool       bSaveVideo      = false;
	bool       bCompressVideo  = false;
	bool       bSaveTraceStats = false;
	//Controls of Stimulation Board
	bool       bExternalTrigger = true;
	bool       bUvFlash = true;
	bool       bStimulate = false;
	unsigned   rr = 0;

	double      peakThresh = 0.6;
	double      noiseThresh = 8;

	double      gain = 25.0;
	//Dimensions MyrPlate
	int         rows = 8;
	int         cols = 6;
	double      xGrid = 18.0; // wellWidth  in MM;
	double      yGrid =  9.0; // wellHeight in MM;

	// part of Well used to analysis
	double      ratioRows = 0.4;
	double      ratioCols = 0.5;


	unsigned    binningFactor = 1;
	unsigned    pixThresh = 127; // global fg/bg cutoff

	double      pixUse = 0.02;   // as default use 2% of brightest pixels

	double      frameRate = 50.0;
	int64_t     xOffset = 0;
	int64_t     yOffset = 0;
	int64_t     imgWidth = 0;
	int64_t     imgHeight = 0;


	int         exposureTime = 100;
	unsigned    slowBufferSize = 501;
	unsigned    fastBufferSize = 15;
	unsigned    peakBufferSize = 21;
	uint64_t    cameraClock = 1000000000;  //aca4024 has a 1GHz internal Clock
	unsigned    autoSave  = 0;   //auto safe time in Minutes
	
	int        imageBuffer = 16;  //number of images to be kept in circ buffer for Video / maxNumBuffer


	unsigned	arduinoCOM = 4;

	double      duration = 130;   //in seconds
	double      saveTime = 120;
	double      interval = 1;     //in hours
	int         measurements = 1;


	bool       bUseDb = false;                     //bool do use Database
	CString    dbHost = L"mysql-unigoe.gwdg.de";   //url or IP to Database
	unsigned   dbPort = 3307;                      //database Port e.g. 3306
	CString    dbName = L"contractionDb";          //database Name e.g. contractionDb
	CString    dbUser = L"analyzeMmt";             //database Username with write priviledges tp forces table
	CString    dbPass = L"ana!zeMe@mnt";           //database password, TDO: store secretely:)




	CString outFileVar = L"FOC_OUTFILE";

	CString exportPath = L"C:\\Labhub\\FOC48_DATA";

	CString suffix = L"";
	CString expName = L"";
	CString meaName = L"";

	int mySet_mskEHM; // 0x12345678

public:

	int		get_mySet_mskEHM(void){ return mySet_mskEHM; }
	int		GetArduinoCOM(void) { return arduinoCOM; };
	void	SetArduinoCOM(int iIn) { arduinoCOM = iIn; };

	double      ledIntensity = 0.5;
	
	double  GetLedIntensity(void) { return ledIntensity; };
	void    SetLedIntensity(double dIn) { ledIntensity = dIn; }

	uint8_t GetPixThresh(void) { return pixThresh; };
	void    SetPixThresh(uint8_t iIn) { pixThresh = iIn; };

	double  Gain(double   dIn = -1.0) { if (dIn >= 0) gain = dIn;    return gain; };
	int     PixThresh(int   iIn = -1.0) { if (iIn >= 0) pixThresh = iIn;    return pixThresh; };
	int     RR(int          iIn = -1.0) { if (iIn >= 0) rr = iIn;    return rr; };
	double  PixUse(double   dIn = -1.0) { if (dIn >= 0) pixUse = dIn;    return pixUse; };
	double  SaveTime(double dIn = -1.0) { if (dIn >= 0) saveTime = dIn;  return saveTime; };
	double  Duration(double dIn = -1.0) { if (dIn >= 0) duration = dIn;   return duration; };
	double  Interval(double dIn = -1.0) { if (dIn >= 0) interval = dIn;   return interval; };
	double  LedIntensity(double dIn = -1.0) { if (dIn >= 0) ledIntensity = dIn;   return ledIntensity; };
	int     Measurements(int iIn = -1.0) { if (iIn >= 0) measurements = iIn;  return measurements; };
	int     ArduinoCOM(int iIn = -1.0) { if (iIn >= 0) arduinoCOM = iIn;  return arduinoCOM; };
	bool    ReverseX(int    iIn = -1) { if (iIn >= 0) bReverseX = iIn > 0; return bReverseX; };
	bool    ReverseY(int    iIn = -1) { if (iIn >= 0) bReverseY = iIn > 0; return bReverseY; };
	double  FrameRate(double dIn = -1.0) { if (dIn >= 0) frameRate = dIn;    return frameRate; };
	//CString ExportPath(void)                  { return exportPath; };
	CString ExportPath(CString sIn = L"") { if (!sIn.IsEmpty()) exportPath = sIn;  return exportPath; };
	//CString Suffix(void)					  { return suffix; };
	CString Suffix(CString sIn = L"") { if (!sIn.IsEmpty()) suffix = sIn;  return suffix; };
	CString ExpName(CString sIn = L"") { if (!sIn.IsEmpty()) expName = sIn;  return expName; };
	CString MeaName(CString sIn = L"") { if (!sIn.IsEmpty()) meaName = sIn;  return meaName; };

	CString FilePath() { return exportPath + "\\" + ExpName(); };

	bool    UvFlash(int iIn = -1) { if (iIn >= 0) bUvFlash = iIn > 0; return bUvFlash; };
	bool    Stimulate(int iIn = -1) { if (iIn >= 0) bStimulate = iIn > 0; return bStimulate; };

	double  RatioRows(double dIn = -1.0) { if (dIn >= 0) ratioRows = dIn;  return ratioRows; };
	double  RatioCols(double dIn = -1.0) { if (dIn >= 0) ratioCols = dIn;  return ratioCols; };

	double  NoiseThresh(double dIn = -1.0) { if (dIn >= 0) noiseThresh = dIn;  return noiseThresh; };
	double  PeakThresh(double dIn = -1.0) { if (dIn >= 0)  peakThresh = dIn;   return peakThresh; };


	double  XGrid(double dIn = -1.0) { if (dIn >= 0) xGrid = dIn;  return xGrid; };
	double  YGrid(double dIn = -1.0) { if (dIn >= 0) yGrid = dIn;  return yGrid; };

	double  Pixel_per_mm() { return ImgWidth() / (ratioCols + cols - 1) / xGrid; };

	int     ImageBuffer(int iIn = -1.0) { if (iIn >= 0) imageBuffer = iIn;  return imageBuffer; };

	int     BinningFactor(int iIn = -1.0) { if (iIn >= 0) binningFactor = iIn;  return binningFactor; };
	uint64_t  CameraClock(int iIn = -1.0) { if (iIn >= 0) cameraClock = iIn;  return cameraClock; };

	uint64_t ImgWidth(int    iIn = -1) { if (iIn >= 0) imgWidth = iIn; return imgWidth; };
	uint64_t ImgHeight(int    iIn = -1) { if (iIn >= 0) imgHeight = iIn; return imgHeight; };
	uint64_t XOffset(int    iIn = -1) { if (iIn >= 0) xOffset = iIn; return xOffset; };
	uint64_t YOffset(int    iIn = -1) { if (iIn >= 0) yOffset = iIn; return yOffset; };
	uint32_t ExposureTime(int    iIn = -1) { if (iIn >= 0) exposureTime = iIn; return exposureTime; };
	uint32_t FastBufferSize(int    iIn = -1) { if (iIn >= 0) fastBufferSize = iIn; return fastBufferSize; };
	uint32_t SlowBufferSize(int    iIn = -1) { if (iIn >= 0) slowBufferSize = iIn; return slowBufferSize; };
	uint32_t PeakBufferSize(int    iIn = -1) { if (iIn >= 0) peakBufferSize = iIn; return peakBufferSize; };

	bool    AutoSave(int    iIn = -1)  { if (iIn >= 0) bAutoSave = iIn > 0; return  bAutoSave; };
	bool    AutoClose(int    iIn = -1) { if (iIn >= 0) bAutoClose = iIn > 0; return  bAutoClose; };

	bool    ColorByTwitch(int    iIn = -1) { if (iIn >= 0) bColorByTwitch = iIn > 0; return  bColorByTwitch; };
	bool    ColorByBase(int    iIn = -1) { if (iIn >= 0)   bColorByBase = iIn > 0; return  bColorByBase; };


	bool    Simulate(int    iIn = -1)     { if (iIn >= 0) bSimulate = iIn > 0; return bSimulate; };
	bool    ExternalTrigger(int iIn = -1) { if (iIn >= 0) bExternalTrigger = iIn > 0; return bExternalTrigger; };
	
	bool     UseDb(int    iIn = -1)        { if (iIn >= 0) bUseDb = iIn > 0; return bUseDb; };
	CString  DbHost() { return dbHost; };
	unsigned DbPort() { return dbPort; };
	CString  DbName() { return dbName; }; 
	CString  DbUser() { return dbUser; };
	CString  DbPass() { return dbPass; };

	bool    SaveVideo(int    iIn = -1) { if (iIn >= 0) bSaveVideo = iIn > 0; return  bSaveVideo; };
	bool    CompressVideo(int    iIn = -1) { if (iIn >= 0) bCompressVideo = iIn > 0; return  bCompressVideo; };
	bool    SaveTraceStats(int    iIn = -1) { if (iIn >= 0) bSaveTraceStats = iIn > 0; return  bSaveTraceStats; };

	BOOL Parameters::read(CString inParFile)

	{
		tinyxml2::XMLDocument doc;
		CString key;

		doc.LoadFile(CStringA(inParFile).GetBuffer());
		//
		tinyxml2::XMLNode* para = doc.FirstChildElement("Parameters");
		tinyxml2::XMLElement* elem = NULL;
		int iVal;
		double dVal;
		float fVal;

		elem = para->FirstChildElement("bSimulate");
		if (elem != NULL) elem->QueryBoolText(&bSimulate);

		elem = para->FirstChildElement("binningFactor");
		if (elem != NULL)  elem->QueryUnsignedText(&binningFactor);

		elem = para->FirstChildElement("imgWidth");
		if (elem != NULL)	elem->QueryInt64Text(&imgWidth);

		elem = para->FirstChildElement("imgHeight");
		if (elem != NULL)	elem->QueryInt64Text(&imgHeight);

		elem = para->FirstChildElement("xOffset");
		if (elem != NULL)	elem->QueryInt64Text(&xOffset);

		elem = para->FirstChildElement("yOffset");
		if (elem != NULL)	elem->QueryInt64Text(&yOffset);

		elem = para->FirstChildElement("cameraClock");
		if (elem != NULL)	elem->QueryUnsigned64Text(&cameraClock);

		elem = para->FirstChildElement("pixThresh");
		if (elem != NULL)	elem->QueryUnsignedText(&pixThresh);

		elem = para->FirstChildElement("RR");
		if (elem != NULL)	elem->QueryUnsignedText(&rr);


		elem = para->FirstChildElement("peakThresh");
		if (elem != NULL) elem->QueryDoubleText(&peakThresh);

		elem = para->FirstChildElement("noiseThresh");
		if (elem != NULL) elem->QueryDoubleText(&noiseThresh);

		elem = para->FirstChildElement("ledIntensity");
		if (elem != NULL) elem->QueryDoubleText(&ledIntensity);

		elem = para->FirstChildElement("pixUse");
		if (elem != NULL)	elem->QueryDoubleText(&pixUse);

		elem = para->FirstChildElement("duration");
		if (elem != NULL)	elem->QueryDoubleText(&duration);

		elem = para->FirstChildElement("saveTime");
		if (elem != NULL)	elem->QueryDoubleText(&saveTime);

		elem = para->FirstChildElement("interval");
		if (elem != NULL)	elem->QueryDoubleText(&interval);

		elem = para->FirstChildElement("measurements");
		if (elem != NULL)	elem->QueryIntText(&measurements);

		elem = para->FirstChildElement("ratioRows");
		if (elem != NULL)	elem->QueryDoubleText(&ratioRows);

		elem = para->FirstChildElement("ratioCols");
		if (elem != NULL)	elem->QueryDoubleText(&ratioCols);

		elem = para->FirstChildElement("exposureTime");
		if (elem != NULL)  elem->QueryIntText(&exposureTime);

		elem = para->FirstChildElement("Gain");
		if (elem != NULL)	elem->QueryDoubleText(&gain);

		elem = para->FirstChildElement("autoSave");
		if (elem != NULL) elem->QueryBoolText(&bAutoSave);

		elem = para->FirstChildElement("autoClose");
		if (elem != NULL) elem->QueryBoolText(&bAutoClose);

		elem = para->FirstChildElement("saveVideo");
		if (elem != NULL) elem->QueryBoolText(&bSaveVideo);

		elem = para->FirstChildElement("colorByTwitch");
		if (elem != NULL) elem->QueryBoolText(&bColorByTwitch);

		elem = para->FirstChildElement("colorByBase");
		if (elem != NULL) elem->QueryBoolText(&bColorByBase);


		elem = para->FirstChildElement("saveTraceStats");
		if (elem != NULL) elem->QueryBoolText(&bSaveTraceStats);
		
		
		elem = para->FirstChildElement("compressVideo");
		if (elem != NULL) elem->QueryBoolText(&bCompressVideo);

		
		elem = para->FirstChildElement("useDb");
		if (elem != NULL) elem->QueryBoolText(&bUseDb);

		elem = para->FirstChildElement("dbHost");
		if (elem != NULL)  dbHost= CString(elem->GetText());

		elem = para->FirstChildElement("dbPort");
		if (elem != NULL)	elem->QueryUnsignedText(&dbPort);


		elem = para->FirstChildElement("dbName");
		if (elem != NULL)  dbName = CString(elem->GetText()); 
		
		elem = para->FirstChildElement("dbPass");
		if (elem != NULL)  dbPass = CString(elem->GetText());


		
		elem = para->FirstChildElement("externalTrigger");
		if (elem != NULL) elem->QueryBoolText(&bExternalTrigger);


		elem = para->FirstChildElement("uvFlash");
		if (elem != NULL) elem->QueryBoolText(&bUvFlash);

		elem = para->FirstChildElement("reverseX");
		if (elem != NULL)	elem->QueryBoolText(&bReverseX);

		elem = para->FirstChildElement("reverseY");
		if (elem != NULL)	elem->QueryBoolText(&bReverseY);

		elem = para->FirstChildElement("frameRate");
		if (elem != NULL)	elem->QueryDoubleText(&frameRate);

		elem = para->FirstChildElement("fastBufferSize");
		if (elem != NULL)	elem->QueryUnsignedText(&fastBufferSize);

		elem = para->FirstChildElement("slowBufferSize");
		if (elem != NULL)	elem->QueryUnsignedText(&slowBufferSize);

		elem = para->FirstChildElement("imageBuffer");
		if (elem != NULL)	elem->QueryIntText(&imageBuffer);
		
		elem = para->FirstChildElement("peakBufferSize");
		if (elem != NULL)	elem->QueryUnsignedText(&peakBufferSize);

		elem = para->FirstChildElement("arduinoPort");
		if (elem != NULL)	elem->QueryUnsignedText(&arduinoCOM);

		elem = para->FirstChildElement("suffix");
		if (elem != NULL)  Suffix(CString(elem->GetText()));

		elem = para->FirstChildElement("exportPath");
		if (elem != NULL)  ExportPath(CString(elem->GetText()));

		elem = para->FirstChildElement("mySet_mskEHM");
		if (elem != NULL)
		{
			char strTrim[MY_HEX_BUF_LEN];
			int n = MyTrimCopy2(strTrim, MY_HEX_BUF_LEN, elem->GetText());
			if (myStr2Num2(iVal, strTrim, n))// support 0xNNNN for hex 16
				mySet_mskEHM = iVal;
		}
		return true;
	}

	BOOL setEnvironmentC(CString value) {

		CRegKey cKey;
		
		long bResult = cKey.Create(HKEY_CURRENT_USER, _T("Environment"));

		if (ERROR_SUCCESS == bResult) {
			bResult = cKey.SetStringValue(outFileVar, value);
		}
		return bResult;

		return true;
	}

	BOOL setEnvironment(CString subKey,  CString value){
		HKEY key;
		char* myKey = "Environment\\";
		//char* myValue = "FOC48_OUTFILE";
		
		//wchar_t* myValueL = L"FOC48_OUTFILE";


		CStringA subKeyA(subKey); // a helper string
		LPCSTR subKeyL = subKeyA;

		CStringA valueA(value); // a helper string
		LPCSTR valueL = valueA;


		int ret = RegOpenKeyA(HKEY_CURRENT_USER, myKey, &key);

		if (ret != ERROR_SUCCESS)			{
			printf("Unable to open registry key\n");
		}

		char  strTmp[200]; const wchar_t* wsvalue = value;
		long n = sprintf_s(strTmp, 199, "%ls", wsvalue);

		if ((ret = RegSetValueA(key, subKeyL, REG_SZ, valueL, (unsigned long)strlen(strTmp) + 1)) != ERROR_SUCCESS)
		{
			printf("Unable to set registry value value_name\n");
			//myError(this->m_hWnd, "RegSetValueA, unable to set registry value value_name.", ret);
		}
		else {
			printf("value_name was set\n");
		}
		RegFlushKey(key);
		RegCloseKey(key);
		// read check
		for (int i = 0; i < 200; i++)
			strTmp[i] = 0;
		
		ret = RegOpenKeyA(HKEY_CURRENT_USER, myKey, &key);
		if (ret != ERROR_SUCCESS)
		{
			printf("Unable to open registry key\n");
		}
		n = 200;
		ret = RegQueryValueA(key, subKeyL, strTmp, &n);
		if (ret != ERROR_SUCCESS)
		{
			//myError(this->m_hWnd, "RegQueryValue, unable to get value_name.", ret);
		}
		else {
			strTmp[n] = 0; printf("value_name read as %ls\n", strTmp);
		}
		RegCloseKey(key);
		return true;
	}


	BOOL write(CString fName)
	{
		Xml Xml(fName);
		//
		//Xml.openElement(L"Parameter");
		Xml.print(L"pixThresh",			L"%.0f", GetPixThresh());
		Xml.print(L"peakThresh",	    L"%.3f", PeakThresh());
		Xml.print(L"noiseThresh",       L"%.3f", NoiseThresh());
		Xml.print(L"exposureTime",		L"%.0f", ExposureTime());
		Xml.print(L"frameRate",			L"%.1f", FrameRate());
		Xml.print(L"imgWidth",			L"%.0f", (double)ImgWidth());
		Xml.print(L"imgHeight",			L"%.0f", (double)ImgHeight());
		Xml.print(L"xOffset",			L"%.0f", (double)XOffset());
		Xml.print(L"yOffset",			L"%.0f", (double)YOffset());
		Xml.print(L"reverseX",			L"%d",   ReverseX());
		Xml.print(L"reverseY",			L"%d",	 ReverseY());
		Xml.print(L"xGrid",             L"%.1f", XGrid(), CString("well width in mm"));
		Xml.print(L"yGrid",             L"%.1f", YGrid(), CString("well height in mm"));
		Xml.print(L"pixel_per_mm",		L"%.1f", Pixel_per_mm());
		Xml.print(L"pixUse",            L"%.3f", PixUse(), CString("Portion of Pixels considered pole"));

		Xml.print(L"duration",			L"%.1f", Duration());
		Xml.print(L"saveTime",			L"%.1f", SaveTime());
		Xml.print(L"bStimulate",		L"%.1d", Stimulate());
		Xml.print(L"saveVideo",         L"%d",   SaveVideo());
		Xml.print(L"saveTraceStats",    L"%d",   SaveTraceStats(), CString("Save Avg, Dev, Min, Max, NPeak in csv format for ECT Analysis"));
		Xml.print(L"compressVideo",     L"%d",   CompressVideo());



		Xml.print(L"gain",				L"%.1f", Gain());
		Xml.print(L"ratioRows",			L"%.1f", RatioRows());
		Xml.print(L"ratioCols",			L"%.1f", RatioCols()); 
		Xml.print(L"ledIntensity",		L"%.2f", LedIntensity());
		Xml.print(L"slowBufferSize",	L"%.0f", SlowBufferSize());
		Xml.print(L"fastBufferSize",	L"%.0f", FastBufferSize());
		Xml.print(L"peakBufferSize",	L"%.0f", PeakBufferSize());
		Xml.print(L"arduinoPort",		L"%.0f", ArduinoCOM());
		Xml.print(L"autoSafe",			L"%.1f", AutoSave(), CString("Save at End of Measurement Time"));
		Xml.print(L"autoClose",         L"%.1f", AutoClose(), CString("Close after Saving"));
		Xml.print(L"useDb",             L"%.1f", UseDb(), CString("Save Data to MySQL Database. Default: false"));
		Xml.print(L"imageBuffer",       L"%.1f", ImageBuffer(), CString("Number of Images kapt for Video Recording"));
		
		Xml.print(L"exportPath",		L"%ls",   ExportPath());
		Xml.print(L"Date",				L"%ls",   myDateStamp());
		Xml.print(L"Time",				L"%ls",   myTimeStamp());
		Xml.print(L"ExperimentName",    L"%ls",   ExpName());
		Xml.print(L"MeasurementName",   L"%ls",   MeaName());
		Xml.closeElement();
		Xml.close();
		//
		printf("Done writing Parameter\n");
		//
		setEnvironmentC(fName);

		return TRUE;
	}
};    //end class Parameters



#endif