#pragma once

#ifndef USBSTIMULATIONBOARD_H
#define USBSTIMULATIONBOARD_H


//#include "comList.h";
//#include <atlstr.h>;

typedef struct _MY_ARDRINO_STATUS
{
	int ID;
	char* strName;
	int nArgCount;
	int nArg[10];
}MY_ARDRINO_STATUS,*pMY_ARDRINO_STATUS;

#define ARDUINO_WAIT_TIME 2000

//#define MY_MAX_CMD_COUNT	11   // increase if new commamd is needed

//extern MY_ARDRINO_STATUS ssCMD[]; // used by others if other progem want to access the CMD directly

//const char* myID2STR(const int nID);          // search all IDs find the string of the ID
//const int   mySTR2ID(const char *strName);      // search all names find the ID
//const int   mySTR2ID_CAP(const char* strName);   // search all names find the ID, regardless the captial letters


//class SerialPort;
extern int glReturn;

#define MY_PORT_NAME_LEN	27
#define MY_DEV_NAME_LEN		255

class USBstimulationBoard {
public:

	USBstimulationBoard(int iPortNum = 0);	
	~USBstimulationBoard();

	void myInit(void){
		status = 0;      // number of elements if buffer
		portName[0] = 'C'; portName[1] = 'O'; portName[2] = 'M'; portName[3] = '0'; portName[4] = 0;
	}

	int status;      //= 0 number of elements if buffer
	static const int  portNameLength = MY_PORT_NAME_LEN;
	char portName[MY_PORT_NAME_LEN];// = "COM0";

	const char *deviceRef = "LightStimulator";

	bool bSwitchUv = true;
	bool bStimEle  = false;
	bool bStimOpt = false;
	//
	static const int  deviceNameLength = MY_DEV_NAME_LEN;
	char deviceName[MY_DEV_NAME_LEN];
	//
	//CString portName = "COM0";
	int  portNum = 16;
	
	//myCOM_LIST comList;

	bool initialize();
	int  RR(int rr); 
	int  setStimulation(bool bOn = TRUE, bool bFreq=TRUE);
	int  setUV(bool bOn=TRUE, bool bFlash = FALSE);
	int  setTrigger(bool bOn = TRUE, bool bExt = TRUE);

	int     write(const char *sCmd, unsigned len=1);
	char* read();
	char*   write_read(const char* sCmd, unsigned len = 1, int &RetCount = glReturn);

	int     set(const char* sCmd, int iVal);
	
	static const int  strReadLength = 255; 
	char strRead[strReadLength];
	
	void  setPort(int iPortNum);
	bool  isPortAvailable();
	bool isDevice(); 
	
	//SerialPort* arduino=NULL;
	
	bool    setStatus(const char* sStatus);
	//
	
	const char* getDeviceName();
	const char* getStatus();


private:
	//int i = 0;
	//int size_    = 0;
	//
	//bool my_open();
	//bool my_close();
	//
	int find();

	//CStringArray * statusMap;	

};
























#endif
