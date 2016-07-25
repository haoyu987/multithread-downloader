#pragma once

#ifndef downloadhelper___
#define downloadhelper___

#include <afxsock.h>
#define MAX_RECV_LEN           100   // max length of the string received.
#define MAX_PENDING_CONNECTS   4     // The maximum length of the queue of pending connections.

class  CHttpSect
{
public:
	CString  szHostAddr;      // Host address.
	int      nHostPort;       // Host port.
	CString  szHttpAddr;      // Http file address.
	CString  szHttpFilename;  // Http file name.
	CString  szDesFilename;   // Downloaded file name.
	DWORD    nStart;          // Downloading start point.
	DWORD    nEnd;            // Downloading end point. 
};

class  CHttpGet
{
public:
	CHttpGet();
	virtual ~CHttpGet();

private:
	CHttpSect *sectinfo;
	static int m_nCount;
	static UINT ThreadDownLoad(void* pParam);

public:
	static DWORD m_nFileLength;

private:
	static SOCKET ConnectHttp(CString strHostAddr, int nPort);
	static BOOL SendHttpHeader(SOCKET hSocket, CString strHostAddr,
		CString strHttpAddr, CString strHttpFilename, DWORD nPos, DWORD nEnd);
	static DWORD GetHttpHeader(SOCKET sckDest, char *str);
	static BOOL SocketSend(SOCKET sckDest, CString szHttp);
	static DWORD GetFileLength(char *httpHeader);
	BOOL FileCombine(CHttpSect *pInfo, FILE *fpwrite);

public:
	BOOL HttpDownLoad(
		CString strHostAddr,
		CString strHttpAddr,
		CString strHttpFilename,
		CString strWriteFileName,
		int nSectNum,
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


class CMyFile
{
public:
	CMyFile();
	virtual ~CMyFile();

public:
	BOOL FileExists(LPCTSTR lpszFileName);
	FILE* GetFilePointer(LPCTSTR lpszFileName);
	DWORD GetFileSizeByName(LPCTSTR lpszFileName);
	CString GetShortFileName(LPCSTR lpszFullPathName);
};

extern CMyFile myfile;

extern CDealSocket dealsocket;


bool DownloadHelper(CString strUrl,
	CString strWriteFileName,
	unsigned long *& downloaded,
	unsigned long & totalSize,
	int nThread = 5
);

#endif
