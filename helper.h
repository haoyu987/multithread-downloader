#pragma once

#ifndef Mydownload___
#define Mydownload___

#include "stdafx.h"

class  CHttpSect
{
public:
	CString  szHostAddr;      // Host address
	int      nHostPort;       // Host port
	CString  szHttpAddr;      // Http file address
	CString  szHttpFilename;  // Http filename
	CString  szDesFilename;   // downloaded filename
	DWORD    nStart;          // start of segment
	DWORD    nEnd;            // end of segment
};

class  CHttpGet
{
public:
	CHttpGet();
	virtual ~CHttpGet();

private:
	CHttpSect *sectinfo;

public:
	static DWORD m_nFileLength;

private:
	static SOCKET ConnectHttp(CString strHostAddr, int nPort);
	static BOOL SendHttpHeader(SOCKET hSocket, CString strHostAddr,
		CString strHttpAddr, CString strHttpFilename, DWORD nPos);
	static DWORD GetHttpHeader(SOCKET sckDest, char *str);
	static BOOL SocketSend(SOCKET sckDest, CString szHttp);
	static DWORD GetFileLength(char *httpHeader);

public:
	BOOL HttpDownLoad(
		CString strHostAddr,
		CString strHttpAddr,
		CString strHttpFilename,
		CString strWriteFileName,
		// int nSectNum,
		DWORD &totalSize);
};


class CDealSocket
{
public:
	CDealSocket();
	virtual ~CDealSocket();

public:
	SOCKET GetConnect(CString host, int port);
	SOCKET Listening(int port);
	CString GetResponse(SOCKET hSock);
};

extern CDealSocket dealsocket;


bool MyDownload(CString strUrl,
	CString strWriteFileName,
	//unsigned long *& downloaded,
	unsigned long & totalSize
//	int nThread = 5
	);

#endif
