#pragma once

#ifndef Mydownload___
#define Mydownload___

#include "stdafx.h"

class  CHttpSect
{
public:
	// CString  szProxyAddr;     // 理服务器地址.
	CString  szHostAddr;      // Host地址.
	// int      nProxyPort;      // 代理服务端口号.
	int      nHostPort;       // Host端口号.
	CString  szHttpAddr;      // Http文件地址.
	CString  szHttpFilename;  // Http文件名.
	CString  szDesFilename;   // 下载后的文件名.
	DWORD    nStart;          // 分割的起始位置.
	DWORD    nEnd;            // 分割的起始位置.
	// DWORD    bProxyMode;      // 下载模态. 
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
//	CString strProxy = "",
//	int nProxyPort = 8080,
//	int nThread = 5
	);

#endif
