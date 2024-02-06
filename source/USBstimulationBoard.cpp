
// ---------------------------------------------------------------------------
#include <windows.h>
#include "USBstimulationBoard.h"
//#include "SerialPort.h"
#include "comList.h"
#include <algorithm>

#include "DBG_print.h"

//#define MY_MAX_CMD_COUNT	11   // increase if new commamd is needed, defined in h file, we skip here

/*
MY_ARDUINO_STATUS ssCMD[MY_MAX_CMD_COUNT] = {
	
	{48,  "getDeviceName"}, //0
	
	{49,  "getStatus"},		//1
	{85,  "UV_on"},			//2
	{117, "UV_off"},		//2
	{87,  "UV_TRIGGER"},	//Goto trigger mode

	{83, "stimulation_on"},	//"S"
	{115, "stimulation_off"},//"s"
	
	{22, "setStimulationFrequency"},    //7,Frequency in Herz
	{23, "setStimulationDuration"},     //8,Durtion of Stimulation
	{24, "setPulseOnDuration"},         //9,if PulseOn + PulseOff > StimulationDuration multiple pulses are sent
	{25, "setPulseOffDuration"}		//10
};


const char* myID2STR(const int nID)   // search all IDs find the string name of the ID
{
	for (int i = 0; i < MY_MAX_CMD_COUNT; i++) {
		if (ssCMD[i].ID == nID)
			return(ssCMD[i].strName);
	}
	return(NULL);
}


#define MY_MAX_CMD_BUF_LENGTH	50		
char* myCAPSTR(const char* In, char* Out, const int n= MY_MAX_CMD_BUF_LENGTH) {
	int j = strlen(In);
	if (j >= n)
		j = n - 1;  // last is 0
	for (int i = 0; i < j; i++) {
		Out[i] = myCAP(In[i]);
	}
	Out[j] = 0; // c string, last one must 0
	return Out;
}

const int mySTR2ID(const char *strName)   // search all names find the ID
{
	for (int i = 0; i < MY_MAX_CMD_COUNT; i++) {
		if (strcmp(ssCMD[i].strName, strName)==0)
			return(ssCMD[i].ID);
	}
	return -1;
}

const int mySTR2ID_CAP(const char* strName)   // search all names find the ID, regardless the captial letters
{
	char A[MY_MAX_CMD_BUF_LENGTH], B[MY_MAX_CMD_BUF_LENGTH];
	myCAPSTR(strName,B);
	for (int i = 0; i < MY_MAX_CMD_COUNT; i++) {
		if (strcmp(myCAPSTR(ssCMD[i].strName, A), B) == 0)
			return(ssCMD[i].ID);
	}
	return -1;
}
*/

bool __fastcall myBuildCOM(const int nCOM, char* str) // using format \\.\COMx for com port in local computer even if port number is >9.
{
	//str[0]='\\';str[1]='\\';str[2]='\\';str[3]='\\';str[4]='.';str[5]='\\';str[6]='\\';
	//str[7]='C'; str[8]='O'; str[9]='M';
	str[0] = '\\'; str[1] = '\\'; str[2] = '.'; str[3] = '\\'; str[4] = 'C'; str[5] = 'O'; str[6] = 'M';
	int n = 0;
	if (nCOM < 0) n = 0;
	else n = nCOM;
	if (n > 999)n = 999; // to be sure, we only use 1000 ports
	if (n < 10) {
		str[7] = '0' + n; str[8] = 0; str[9] = 0; return true;
	}
	else if (n < 100) {
		str[7] = '0' + n / 10; str[8] = '0' + n % 10; str[9] = 0; str[10] = 0; return true;
	}
	else  if (n < 1000) {
		str[9] = n % 10; n = n / 10;
		str[8] = n % 10; n = n / 10;
		str[7] = n % 10;
		str[10] = 0; str[11] = 0; return true;
	}
	return false;  // this will not run forever.
}

//---------------------------------------------------------------------------
// Following are small functions for non-block COM IO
//---------------------------------------------------------------------------
#define hfUseDTR 0x01
#define hfUseRTS 0x02
#define hfRequireDSR 0x04
#define hfRequireCTS 0x08

typedef struct _tagTCommPort {
	HANDLE hPort;
	OVERLAPPED ReadOL;
	OVERLAPPED WriteOL;
} TCommPort;

void SetDtrLine(HANDLE comm, BOOL on);
BOOL OpenPort(const char *comdevice, TCommPort &cp);
BOOL ClosePort(TCommPort &cp);
BOOL SendChar(TCommPort &cp, int ch);
DWORD SendString(TCommPort &cp, const char *sz);
DWORD ReadString(TCommPort &cp, char *sz, const int size);

void HWFlowEnable(TCommPort &cp, DWORD options)
{
	if (cp.hPort == INVALID_HANDLE_VALUE) return;
	DCB dcb;
	GetCommState(cp.hPort, &dcb);
	if ((options & hfUseRTS) == hfUseRTS) {
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	}
	else {
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
	}
	if ((options & hfUseDTR) == hfUseDTR) {
		dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
	}
	else {
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
	}
	COMMPROP prop;
	GetCommProperties(cp.hPort, &prop);
	dcb.XoffLim = WORD(prop.dwCurrentRxQueue / 4); //Shutdown when 1/4 is remaining
	dcb.XonLim = WORD(prop.dwCurrentRxQueue / 4); //Resume when 1/4 is full
	dcb.fOutxCtsFlow = ((options & hfRequireCTS) == hfRequireCTS);
	dcb.fOutxDsrFlow = ((options & hfRequireDSR) == hfRequireDSR);
	SetCommState(cp.hPort, &dcb);
}

void SetupCommState(TCommPort &cp, DWORD Baud, BOOL Flow)
{
	DCB dcb;
	GetCommState(cp.hPort, &dcb);
	if (Baud) {
		dcb.BaudRate = Baud;
	}
	dcb.fBinary = TRUE; //
	dcb.fParity = FALSE;
	dcb.fOutxCtsFlow = Flow;
	dcb.fOutxDsrFlow = FALSE; //
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;
	dcb.fErrorChar = FALSE;
	dcb.fNull = FALSE;
	dcb.fRtsControl = Flow ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_ENABLE;
	if (Flow) {
		COMMPROP prop;
		GetCommProperties(cp.hPort, &prop);
		dcb.XoffLim = WORD(prop.dwCurrentRxQueue / 4); //Shutdown when 1/4 is remaining
		dcb.XonLim = WORD(prop.dwCurrentRxQueue / 4); //Resume when 1/4 is full
	}
	dcb.fAbortOnError = FALSE;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY; //ODDPARITY, NOPARITY
	dcb.StopBits = ONESTOPBIT;
	SetCommState(cp.hPort, &dcb);

	COMMTIMEOUTS cto;
	if (!GetCommTimeouts(cp.hPort, &cto)) {
		printf("*** error %d\n", GetLastError());
	}

	cto.ReadIntervalTimeout = MAXDWORD;
	cto.ReadTotalTimeoutMultiplier = 0;
	cto.WriteTotalTimeoutMultiplier = 0;
	cto.WriteTotalTimeoutConstant = 0;

	SetCommTimeouts(cp.hPort, &cto);
}

void SetDtrLine(HANDLE comm, BOOL on)
{
	EscapeCommFunction(comm, on ? SETDTR : CLRDTR);
}

BOOL OpenPort(const int nComDev, TCommPort &cp)
{
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	ZeroMemory(&cp, sizeof(cp));
	char szDevice[50];
	//sprintf(szDevice, "\\\\.\\COM%d", nComDev);
	myBuildCOM(nComDev, szDevice);
	cp.hPort = CreateFileA(szDevice,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		&sa,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		0);

	if (cp.hPort == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	SetDtrLine(cp.hPort, TRUE);
	//SetDtrLine(cp.hPort, FALSE);

	cp.ReadOL.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	cp.WriteOL.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//SetupCommState(cp, CBR_9600, TRUE);
	//SetupCommState(cp, CBR_9600, FALSE);
	//SetupCommState(cp, CBR_4800, TRUE);
	//SetupCommState(cp, CBR_57600, TRUE);
	SetupCommState(cp, CBR_115200, TRUE);
	HWFlowEnable(cp, hfUseRTS | hfRequireCTS);
	//HWFlowEnable(cp, 0); // false hfUseRTS
	return TRUE;
}

BOOL ClosePort(TCommPort &cp)
{
	SetDtrLine(cp.hPort, FALSE);
	if (cp.ReadOL.hEvent) CloseHandle(cp.ReadOL.hEvent);
	if (cp.WriteOL.hEvent) CloseHandle(cp.WriteOL.hEvent);
	CloseHandle(cp.hPort);
	ZeroMemory(&cp, sizeof(cp));
	return TRUE;
}

BOOL SendChar(TCommPort &cp, int ch)
{
	return TransmitCommChar(cp.hPort, ch);
}

DWORD SendString(TCommPort &cp, const char *sz, const int nLen = -1)
{
	int i = nLen;
	if (i < 1) i = (int) strlen(sz);
	DWORD dw = 0;
	if (!WriteFile(cp.hPort, sz, i, &dw, &cp.WriteOL)) {
		if (GetLastError() == ERROR_IO_PENDING) {
			// Waiting for data
			if (GetOverlappedResult(cp.hPort, &cp.WriteOL, &dw, TRUE)) {
				// Write complete, reset event
				ResetEvent(cp.WriteOL.hEvent);
			}
		}
	}
	return dw;
}

DWORD ReadString(TCommPort &cp, char *sz, const int size)
{
	COMSTAT cstat;
	DWORD cerr;
	DWORD dw = 0;
	DWORD t1 = GetTickCount();
	if (ClearCommError(cp.hPort, &cerr, &cstat) && cstat.cbInQue)
	{
		dw = size;
		dw = cstat.cbInQue;
		if (!ReadFile(cp.hPort, sz, size, &dw, &cp.ReadOL)) {
			if (GetLastError() == ERROR_IO_PENDING) {
				// Waiting for data
				if (GetOverlappedResult(cp.hPort, &cp.ReadOL, &dw, TRUE)) {
					// Read complete, reset event
					ResetEvent(cp.ReadOL.hEvent);
				}
			}
		}
		//sz[dw] = 0;
		if (dw) {
			DWORD t2 = GetTickCount();
			printf("**** Read: %d byte, Rate: %.3f\n", dw, (double)dw / (t2 - t1 + 1));
		}

	}
	return dw;
}

#define MY_READ_INTERVAL_TIMEOUT	50
#define MY_BLOCK_READING_TIME		25
#define MY_MAIN_TIMEOUT				2000

//int __fastcall mySend2(const int nCOM, const char cValue, int &nRead, char *pBuf, const int nLen){
//	nRead = 0;
//	while (1){
//		TCommPort cp = { 0 };
//		if (!OpenPort(nCOM, cp)) {
//			printf("Open port error: %d\n", GetLastError());
//			break;
//		}
//
//		pBuf[0] = cValue; SendString(cp, pBuf, 1); //#OK
//		//SendChar(cp, cValue);
//
//		char sz[1024 * 4];
//		int nTimeOut = MY_MAIN_TIMEOUT;
//		int nSleep = MY_BLOCK_READING_TIME;
//		while (nTimeOut>0 && nLen > nRead)
//		{
//			Sleep(nSleep);
//			// CNT send
//			int i = ReadString(cp, sz, sizeof(sz)-1);
//			int j;
//			if (i > 0) {
//				printf("get %d bytes.\n", i);
//				if (i > nLen - nRead)j = nLen - nRead;
//				else j = i;
//				for (int k = 0; k < j; k++)
//					pBuf[k + nRead] = sz[k];
//				nRead += j;
//			}
//			else
//				nTimeOut -= nSleep;
//		}
//		ClosePort(cp);
//		break;
//	}
//	return nRead;
//}

int __fastcall mySend4(const int nCOM, const char *sValue, int &nInOutLen, char *pBuf, const int nBufLen){

	while (1){
		TCommPort cp = { 0 };
		if (!OpenPort(nCOM, cp)) {
			printf("Open port error: %d\n", GetLastError());
			nInOutLen = 0;
			break;
		}
		if (nInOutLen>0)
			SendString(cp, sValue, nInOutLen); //#OK
		//SendChar(cp, cValue);

		nInOutLen = 0;
		char sz[1024 * 4];
		int nTimeOut = MY_MAIN_TIMEOUT;
		int nSleep = MY_BLOCK_READING_TIME;
		int nAfterRead = 0;
		while (nTimeOut > 0 && nBufLen > nInOutLen)
		{
			Sleep(nSleep);
			// CNT send
			int i = ReadString(cp, sz, sizeof(sz)-1);
			int j;
			if (i > 0) {
				printf("get %d bytes, total %d.\n", i, nInOutLen + i);
				if (i > nBufLen - nInOutLen)j = nBufLen - nInOutLen;
				else j = i;
				for (int k = 0; k < j; k++)
					pBuf[k + nInOutLen] = sz[k];
				nInOutLen += j;
				nAfterRead = 0;
			}
			nTimeOut -= nSleep;
			if (nInOutLen > 0){
				nAfterRead += nSleep;
				if (nAfterRead >= MY_READ_INTERVAL_TIMEOUT)break;
			}
		}
		ClosePort(cp);
		break;
	}
	return nInOutLen;
}


int __fastcall myClear(const int nCOM){
	int nInOutLen = 0;
	while (1){
		TCommPort cp = { 0 };
		if (!OpenPort(nCOM, cp)) {
			printf("Open port error: %d\n", GetLastError());
			break;
		}
	
		char sz[1024 * 4];
		int nTimeOut = 57;
		int nSleep = MY_BLOCK_READING_TIME;
		int nAfterRead = 0;
		while (nTimeOut > 0 && nInOutLen == 0)
		{
			Sleep(nSleep);
			// CNT send
			int i = ReadString(cp, sz, sizeof(sz)-1);
			//int j;
			if (i > 0) {
				printf("get %d bytes, total %d.\n", i, nInOutLen + i);
				nInOutLen += i;
				nAfterRead = 0;
			}
			nTimeOut -= nSleep;
			if (nInOutLen > 0){
				nAfterRead += nSleep;
				if (nAfterRead >= MY_READ_INTERVAL_TIMEOUT)break;
			}
		}
		ClosePort(cp);
		break;
	}
	return nInOutLen;
}


USBstimulationBoard::USBstimulationBoard(int iPortNum) {
	myInit();
	setPort(iPortNum);
};

void USBstimulationBoard::setPort(int iPortNum) {
	//if (arduino)
	//	close();
	portNum = iPortNum;	
	myBuildCOM(iPortNum, portName);
	//open();
}

USBstimulationBoard::~USBstimulationBoard() {
	//if (arduino)
	//	delete(arduino);
	//delete(statusMap);
};

bool USBstimulationBoard::setStatus(const char *sStatus)
{
	bool bOK = false;
	return bOK;
}


int USBstimulationBoard::setUV(bool bUV, bool bFlash)
{
	//const char *sCmd = bUV ? "U" : "u";
	//const char sCmd = bUV ? 'a' : 'b';	
	const char* sCmd;
	//open();
	if (bUV)
		if (bFlash)
			sCmd = "F\0";
		else
			sCmd = "U\0";
	else
		sCmd = "u\0";
	//
	//
	int ret = write(sCmd, 2);
	//Sleep(ARDUINO_WAIT_TIME);	
	//close(); 
	//
	return ret;
}

int USBstimulationBoard::set(const char* sCmd, int iVal)
{
	//open();
	char sCmdBuf[32];
	int len = sprintf_s(sCmdBuf, 31, "%c \\%d", sCmd[0], iVal);

	int ret = write(sCmdBuf, len);
	//Sleep(ARDUINO_WAIT_TIME);
	//close();
	return ret;
}


int USBstimulationBoard::write(const char* sCmd, unsigned len)
{
	//open(); 
	//int ret = arduino->write(sCmd, len);
	//Sleep(ARDUINO_WAIT_TIME);
	//close();

	int nLen = len;
	int ret = mySend4(portNum, sCmd, nLen, strRead, 254); //strRead[ret] = 0;
	return ret;
}

char* USBstimulationBoard::read()
{
	//open();
	////int SerialPort::read(const char* buffer, unsigned int buf_size)	
	//int ret = arduino->read(strRead, strReadLength);
	//Sleep(ARDUINO_WAIT_TIME);
	//close();
	return strRead;
}
int glReturn;
char* USBstimulationBoard::write_read(const char* sCmd, unsigned len, int &RetCount)
{	// unsigned len = 1, int &RetCount = glReturn
	//open();
	//int ret = arduino->write(sCmd, len);
	//Sleep(ARDUINO_WAIT_TIME);
	//ret = arduino->read(strRead, strReadLength);
	//Sleep(ARDUINO_WAIT_TIME);
	//close();
	//if (!ret == 1)
	//	strRead[0] = 0;
	////
	RetCount = write(sCmd, len);
	return strRead;
}

int USBstimulationBoard::RR(int rr) {
	char sCmd[32];
	int ret;
	int len = sprintf_s(sCmd, 31, "S \\%d", rr);
    ret = write(sCmd, len);
	return ret;
}


// setStimulation bStim=(On/Off) bFreq=(Permanent / Frequency) 

int USBstimulationBoard::setStimulation(bool bStim, bool bFreq)
{
	const char* sCmd;
	int ret;
	sCmd = bStim ? "S\0" : "s\0";
	ret = write(sCmd, 2);
	//Sleep(ARDUINO_WAIT_TIME);
	sCmd = bFreq ? "p\0" : "P\0";
	ret = write(sCmd, 2);
	//Sleep(ARDUINO_WAIT_TIME);
	
	return ret;
}


// setTrigger bTrig(On/Off) bExt(external / internal)

int USBstimulationBoard::setTrigger(bool bFlash, bool bExt)
{
	const char* sCmd;
	int ret;

	sCmd = bFlash ? "F\0" : "f\0";
	//Sleep(ARDUINO_WAIT_TIME);
	ret = write(sCmd, 2); 
	sCmd = bExt ? "T\0" : "t\0";
	ret = write(sCmd, 2);
	//Sleep(ARDUINO_WAIT_TIME);

	return ret;
}


//bool USBstimulationBoard::my_open() {
//	//if (arduino != NULL)
//	//				close();
//	//arduino = new SerialPort(portName);
//	return 1;
//}
//
//bool USBstimulationBoard::my_close() {
//	//if (arduino) {
//	//	if (arduino->isConnected())
//	//		arduino->close();
//	//	delete(arduino);
//	//}
//	//arduino = NULL;
//	//
//	return 1;
//}

// this function is not used by others
const char *USBstimulationBoard::getStatus()
{
	//int n;
	const char *sCmd = "0\0";
	//
	//
	int ret = write(sCmd, 2);
	//close();
	//return sCmd;
	return strRead;
}


void removeNewline(char * str, int len)
{
	int j = 0;
	for (int i = 0; i < len; i++) {
		if (str[i] != '\n')
			str[j++] = str[i];		
	}
}


const char* USBstimulationBoard::getDeviceName()
{
	const char *sCmd = "0\0";
	int nReadCount = 0;
	char* str = write_read(sCmd, 2, nReadCount);
	
	if (nReadCount == 0) {
		myDBG_print("getDeviceName: Device on port %d %s did not return a name\n", portNum, portName);
		deviceName[0] = 0;
	} else {
		memcpy(deviceName, str, nReadCount); deviceName[nReadCount] = 0;
		removeNewline(deviceName, deviceNameLength);
		myDBG_print("getDeviceName: Device on port %d %s is %s\n", portNum, portName, deviceName);
	}
	return deviceName;
}

//Capitalize Character for cas-insensitive string compare

inline char myCAP(const char cOne) {
	return cOne & 0xDf; // 0x61 -> 0x41
}

bool mystrcmp(const char* A, const char* B) 
{
	int s = 0;
	int j = (int)strlen(B);
	for (int i = 0; i <= j; i++)
		s += (myCAP(A[i]) == myCAP(B[i])) ? 1 : 0;
	return (s == j);
}

bool USBstimulationBoard::isPortAvailable()
{
	bool bOK = false;
	myCOM_LIST cl;
	//cl.nCOMCount
	for (int i = 0; i < cl.nCOMCount; i++)
		if ( portNum==cl.sCOM_Num[i] ) bOK=true;
		
	if (!bOK) {
		myDBG_print("port COM%d is not found. We have %d ports:\n", portNum, cl.nCOMCount);
		for (int i = 0; i < cl.nCOMCount; i++)
			myDBG_print("%d, COM%d, %s.\n", i, cl.sCOM_Num[i], cl.ppCOM_Name[i]);
	}
	return bOK;
}

bool USBstimulationBoard::isDevice() {
	getDeviceName();	
	bool bOK = mystrcmp(deviceName, deviceRef);
	myDBG_print("isDevice: deviceName on port %d %s is %s, ref is %s bOK %d\n", portNum, portName, deviceName, deviceRef, bOK);
	return bOK;
}

bool USBstimulationBoard::initialize()
{
	if (isPortAvailable())
	{
		if (isDevice())
			return true;
	}
	return (find()>0);
}

int USBstimulationBoard::find()
{
	myCOM_LIST cc;
	//	
	for (int i = 0; i < cc.nCOMCount; i++) {
		printf("finding com port: %d COM%d, %s.\n", i, cc.sCOM_Num[i], cc.ppCOM_Name[i]);		
		if (myFind(cc.ppCOM_Name[i], "Arduino") >= 0) {
			setPort(cc.sCOM_Num[i]);
			//printf("Check if %s matches %s\n", cc.ppCOM_Name[i], "Arduino");
			if (isDevice())				
				break;
			else
				portNum = 0;
		}
	}
	printf("find: returning port: %d, %s.\n", portNum, portName);

	return portNum;
}
