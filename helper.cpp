#include <iostream>
#include "helper.h"
using namespace std;

unsigned long rdownloaded = 0;
int CHttpGet::m_nCount;
DWORD CHttpGet::m_nFileLength = 0;

// parseURL and get the host address, file path and filename.
void ParseURL(CString URL, CString &host, CString &path, CString &filename)
{
	URL.TrimLeft();
	URL.TrimRight();
	CString str = URL;
	CString strFind = _T("http://");
	int m;
	int n = str.Find(strFind);
	if (n != -1) {
		str.Delete(0, n + strFind.GetLength());
	}

	n = str.Find('/');
	host = str.Left(n);
	m = str.ReverseFind('/');
	path = str.Mid(n);
	filename = str.Right(str.GetLength() - m - 1);
}

bool DownloadHelper(CString strUrl,
	CString strWriteFileName,
	unsigned long *& downloaded,
	unsigned long & totalSize,
	int nThread
)
{
	CHttpGet b;
	CString strHostAddr;
	CString strHttpAddr;
	CString strHttpFilename;

	//monitor the downloaded data
	downloaded = &rdownloaded;

	ParseURL(strUrl, strHostAddr, strHttpAddr, strHttpFilename);
	printf("\nDownload %s\nFrom %s\n", strHttpFilename, strHttpAddr);
	strWriteFileName += strHttpFilename;

	if (!b.HttpDownLoad(strHostAddr, strHttpAddr, strHttpFilename, strWriteFileName, nThread, totalSize))
		return false;
	return true;
}

//---------------------------------------------------------------------------
CHttpGet::CHttpGet()
{
	m_nFileLength = 0;
}

//---------------------------------------------------------------------------
CHttpGet::~CHttpGet()
{
}

//---------------------------------------------------------------------------
BOOL CHttpGet::HttpDownLoad(
	CString strHostAddr,
	CString strHttpAddr,
	CString strHttpFilename,
	CString strWriteFileName,
	int nSectNum,
	DWORD &totalSize)
{
	ASSERT(nSectNum>0 && nSectNum <= 50);
	int nHostPort = 80;

	SOCKET hSocket;
	hSocket = ConnectHttp(strHostAddr, nHostPort);
	if (hSocket == INVALID_SOCKET) return TRUE;

	// send Http header, get content length
	if(!SendHttpHeader(hSocket, strHostAddr, strHttpAddr, strHttpFilename, 0,0))
		printf("File request failed.");
	closesocket(hSocket);

	totalSize = CHttpGet::m_nFileLength;

	m_nCount = 0;                                    // reset the counter.
	sectinfo = new CHttpSect[nSectNum];              // set memory for http info.
	DWORD nSize = m_nFileLength / nSectNum;          // calculate the length for a single section.

	int i;
	// No more than 50 threads are allowed.
	CWinThread* pthread[50];
	for (i = 0; i<nSectNum; i++)
	{
		sectinfo[i].szHostAddr = strHostAddr;        
		sectinfo[i].nHostPort = nHostPort;		     
		sectinfo[i].szHttpAddr = strHttpAddr;        
		sectinfo[i].szHttpFilename = strHttpFilename;


		// Indexing the temporary files.
		CString strTempFileName;
		strTempFileName.Format("%s_%d", strWriteFileName, i);
		sectinfo[i].szDesFilename = strTempFileName;   // Saved file name.

		if (i<nSectNum - 1) {
			sectinfo[i].nStart = i*nSize;              // downloading start point.
			sectinfo[i].nEnd = (i + 1)*nSize;          // downloading end point
		}
		else {
			sectinfo[i].nStart = i*nSize;              // start point of the last section
			sectinfo[i].nEnd = m_nFileLength;          // end point of the last section
		}
		pthread[i] = AfxBeginThread(ThreadDownLoad, &sectinfo[i]);

	}

	HANDLE hThread[50];
	for (int ii = 0; ii < nSectNum; ii++)
		hThread[ii] = pthread[ii]->m_hThread;

	// wait for all downloading threads to terminate.
	WaitForMultipleObjects(nSectNum, hThread, TRUE, INFINITE);

	// if unfinished return false and reconnect.
	if (m_nCount != nSectNum)
		return FALSE;
	else
		rdownloaded = totalSize;

	FILE *fpwrite;
	errno_t err;

	// open file writing.
	if ((err = fopen_s(&fpwrite, strWriteFileName, "w+b")) != 0) {
		TRACE("File open error!\n");
		return FALSE;
	}

	for (i = 0; i<nSectNum; i++) {
		FileCombine(&sectinfo[i], fpwrite);
	}

	fclose(fpwrite);

	delete[] sectinfo;

	return TRUE;
}

//---------------------------------------------------------------------------
BOOL CHttpGet::FileCombine(CHttpSect *pInfo, FILE *fpwrite)
{
	FILE *fpread;
	errno_t err;

	// open file reading.
	if ((err = fopen_s(&fpread, pInfo->szDesFilename, "r+b")) != 0) {
		printf("%s open failed.\n", pInfo->szDesFilename);
		return FALSE;
	}

	DWORD nPos = pInfo->nStart;

	// set the start point
	fseek(fpwrite, nPos, SEEK_SET);

	int c;
	// write the data into file
	while ((c = fgetc(fpread)) != EOF)
	{
		fputc(c, fpwrite);
		nPos++;
		if (nPos == pInfo->nEnd) break;
	}

	fclose(fpread);
	DeleteFile(pInfo->szDesFilename);

	return TRUE;
}

//---------------------------------------------------------------------------
UINT CHttpGet::ThreadDownLoad(void* pParam)
{
	CHttpSect *pInfo = (CHttpSect*)pParam;
	SOCKET hSocket;

	hSocket = ConnectHttp(pInfo->szHostAddr, pInfo->nHostPort);
	if (hSocket == INVALID_SOCKET) return 1;

	// calculate the size of temporary file so downloading can resume after break.
	DWORD nFileSize = myfile.GetFileSizeByName(pInfo->szDesFilename);
	DWORD nSectSize = (pInfo->nEnd) - (pInfo->nStart);

	// this file has finished downloading.
	if (nFileSize >= nSectSize) {
		CHttpGet::m_nCount++;  // increase the counter.
		return 0;
	}

	FILE *fpwrite = myfile.GetFilePointer(pInfo->szDesFilename);
	if (!fpwrite) return 1;

	// set the content range and send http request.
	SendHttpHeader(hSocket, pInfo->szHostAddr, pInfo->szHttpAddr,
		pInfo->szHttpFilename, pInfo->nStart + nFileSize, pInfo->nEnd);

	// set the start point.
	fseek(fpwrite, nFileSize, SEEK_SET);

	DWORD nLen;
	DWORD nSumLen = 0;
	char szBuffer[1024];

	do
	{
		if (nSumLen >= nSectSize - nFileSize) break;
		nLen = recv(hSocket, szBuffer, sizeof(szBuffer), 0);

		//asynchronized atom operation.
		rdownloaded += nLen;

		if (nLen == SOCKET_ERROR) {
			TRACE("Read error!\n");
			fclose(fpwrite);
			return 1;
		}

		nSumLen += nLen;

		// write the data in buffer into the file.		
		fwrite(szBuffer, nLen, 1, fpwrite);
	} while (nLen > 0);

	fclose(fpwrite);      // close file writing.
	closesocket(hSocket); // close socket.
	CHttpGet::m_nCount++; // increase counter.
	return 0;
}

//---------------------------------------------------------------------------
SOCKET CHttpGet::ConnectHttp(CString strHostAddr, int nPort)
{
	TRACE("connecting\n");
	SOCKET hSocket = dealsocket.GetConnect(strHostAddr, nPort);
	if (hSocket == INVALID_SOCKET)
		return INVALID_SOCKET;

	return hSocket;
}

// format and send http header.
BOOL CHttpGet::SendHttpHeader(SOCKET hSocket, CString strHostAddr,
	CString strHttpAddr, CString strHttpFilename, DWORD nPos, DWORD nEnd)
{
	CString sTemp;
	char cTmpBuffer[1024];

	// Line1: path and version.
	sTemp.Format("GET %s HTTP/1.1\r\n", strHttpAddr);

	// Line2: host address.
	sTemp.AppendFormat("Host: %s\r\n", strHostAddr);

	// Line3: connection
	sTemp.AppendFormat("Connection: keep-alive\r\n");

	// Line4: file type.
	sTemp.AppendFormat("Accept: */*\r\n");

	// Line5: browser.
	sTemp.AppendFormat("User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36\r\n");

	sTemp.AppendFormat("Accept - Encoding: gzip, deflate, sdch\r\nAccept-Language: zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n");

	// Line6: referer.
	sTemp.AppendFormat("Referer: %s%s\r\n", strHostAddr, strHttpAddr);

	// Line7: set content range.
	if(nEnd != 0)
	{
		sTemp.AppendFormat("Range: bytes=%d-%d\r\n", nPos, nEnd);
	}

	// LastLine:
	sTemp.AppendFormat("\r\n");

	// printf("%s", sTemp);
	// send http request.
	if (!SocketSend(hSocket, sTemp)) return FALSE;

	// get the returned http header.
	int i = GetHttpHeader(hSocket, cTmpBuffer);
	if (!i)
	{
		TRACE("Failed to get HTTP header!\n");
		return FALSE;
	}

	// check whether the file is available.
	sTemp = cTmpBuffer;
	// printf("%s", sTemp);
	if (sTemp.Find("404") != -1) return FALSE;

	// get the content length.
	if (nEnd == 0) {
		m_nFileLength = GetFileLength(cTmpBuffer);
	}

	return TRUE;
}

//---------------------------------------------------------------------------
DWORD CHttpGet::GetHttpHeader(SOCKET sckDest, char *str)
{
	BOOL bResponsed = FALSE;
	DWORD nResponseHeaderSize;

	if (!bResponsed)
	{
		char c = 0;
		int nIndex = 0;
		BOOL bEndResponse = FALSE;
		while (!bEndResponse && nIndex < 1024)
		{
			recv(sckDest, &c, 1, 0);
			str[nIndex++] = c;
			if (nIndex >= 4)
			{
				if (str[nIndex - 4] == '\r' &&
					str[nIndex - 3] == '\n' &&
					str[nIndex - 2] == '\r' &&
					str[nIndex - 1] == '\n')
					bEndResponse = TRUE;
			}
		}

		str[nIndex] = 0;
		nResponseHeaderSize = nIndex;
		bResponsed = TRUE;
	}

	return nResponseHeaderSize;
}

//---------------------------------------------------------------------------
DWORD CHttpGet::GetFileLength(char *httpHeader)
{
	CString strHeader;
	CString strFind = _T("Content-Length:");
	int local;
	strHeader = CString(httpHeader);
	local = strHeader.Find(strFind, 0);
	local += strFind.GetLength();
	strHeader.Delete(0, local);
	local = strHeader.Find("\r\n");

	if (local != -1) {
		strHeader = strHeader.Left(local);
	}

	return atoi(strHeader);
}

//---------------------------------------------------------------------------
BOOL CHttpGet::SocketSend(SOCKET sckDest, CString szHttp)
{
	int iLen = szHttp.GetLength();
	if (send(sckDest, szHttp, iLen, 0) == SOCKET_ERROR)
	{
		closesocket(sckDest);
		TRACE("Send failed. Error: %d\n", WSAGetLastError());
		return FALSE;
	}

	return TRUE;
}

CDealSocket dealsocket;

// initiates use of the Winsock DLL and check if the WinSock DLL is supported.
CDealSocket::CDealSocket()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		TRACE("WSAStartup failed:%d\n", err);
		return;
	}

	if (wsaData.wVersion != wVersionRequested) {
		TRACE("WinSock version not supported\n");
		WSACleanup();
		return;
	}

}

//---------------------------------------------------------------------------
CDealSocket::~CDealSocket()
{
	// terminates use of the Winsock 2 DLL (Ws2_32.dll).
	WSACleanup();
}

//---------------------------------------------------------------------------
CString CDealSocket::GetResponse(SOCKET hSock)
{
	char szBufferA[MAX_RECV_LEN];  	// ASCII string buffer. 
	int	iReturn;					// return value of recv.

	CString szError;
	CString strPlus;
	strPlus.Empty();

	while (1)
	{
		// receive data from socket.
		iReturn = recv(hSock, szBufferA, MAX_RECV_LEN, 0);
		szBufferA[iReturn] = 0;
		strPlus += szBufferA;

		TRACE(szBufferA);

		if (iReturn == SOCKET_ERROR)
		{
			szError.Format("No data is received, recv failed. Error: %d",
				WSAGetLastError());
			MessageBox(NULL, szError, TEXT("Client"), MB_OK);
			break;
		}
		else if (iReturn<MAX_RECV_LEN) {
			TRACE("Finished receiving data\n");
			break;
		}
	}

	return strPlus;
}

//---------------------------------------------------------------------------
SOCKET CDealSocket::GetConnect(CString host, int port)
{

	CHAR buffer[20];
	SOCKET ConnectSocket;
	SOCKADDR_IN saServer;          // Server address.
	SOCKADDR_IN *sockaddr_ipv4;
	ADDRINFO *result = NULL;
	ADDRINFO hints;
	DWORD dwRetval;

	// creates a socket that is bound to a server with TCP/IP protocol.
	ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ConnectSocket == INVALID_SOCKET) {
		TRACE(L"socket function failed. Error: %ld\n", WSAGetLastError());
		return INVALID_SOCKET;
	}

	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	_itoa_s(port, buffer, 10);
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	// call getaddrinfo to get response information about the server
	dwRetval = getaddrinfo(host, buffer, &hints, &result);
	if (dwRetval != 0)
	{
		TRACE("Unable to get the host information. Error: %d\n", dwRetval);
		closesocket(ConnectSocket);
		return INVALID_SOCKET;
	}

	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	// use TCP/IP protocol
	saServer.sin_family = AF_INET;
	// set IP address
	sockaddr_ipv4 = (PSOCKADDR_IN)result->ai_addr;
	saServer.sin_addr = sockaddr_ipv4->sin_addr;
	// set port
	saServer.sin_port = htons(port);

	// connect the socket to server
	if (connect(ConnectSocket, (PSOCKADDR)&saServer,
		sizeof(saServer)) == SOCKET_ERROR)
	{
		TRACE("Connecting to the server failed. Error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		return INVALID_SOCKET;
	}
	freeaddrinfo(result);
	return ConnectSocket;
}

//---------------------------------------------------------------------------
SOCKET CDealSocket::Listening(int port)
{
	SOCKET ListenSocket = INVALID_SOCKET;	// listen to socket.
	SOCKADDR_IN local_sin;				    // local socket address.

	// setup TCP/IP socket.
	if ((ListenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		TRACE("Allocating socket failed. Error: %d\n", WSAGetLastError());
		return INVALID_SOCKET;
	}

	// initialize socket address structure.
	local_sin.sin_family = AF_INET;
	local_sin.sin_port = htons(port);
	local_sin.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind local address to listening socket.
	if (bind(ListenSocket,
		(struct sockaddr *) &local_sin,
		sizeof(local_sin)) == SOCKET_ERROR)
	{
		TRACE("Binding socket failed. Error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		return INVALID_SOCKET;
	}

	// listen to the connection.
	if (listen(ListenSocket, MAX_PENDING_CONNECTS) == SOCKET_ERROR)
	{
		TRACE("Listening to the client failed. Error: %d\n",
			WSAGetLastError());
		closesocket(ListenSocket);
		return INVALID_SOCKET;
	}

	return ListenSocket;
}

CMyFile myfile;

//---------------------------------------------------------------------------
CMyFile::CMyFile()
{
}

//---------------------------------------------------------------------------
CMyFile::~CMyFile()
{
}

//---------------------------------------------------------------------------
BOOL CMyFile::FileExists(LPCTSTR lpszFileName)
{
	DWORD dwAttributes = GetFileAttributes(lpszFileName);
	if (dwAttributes == 0xFFFFFFFF)
		return FALSE;

	if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
		== FILE_ATTRIBUTE_DIRECTORY)
	{
		return FALSE;
	}
	else {
		return TRUE;
	}
}

//---------------------------------------------------------------------------
FILE* CMyFile::GetFilePointer(LPCTSTR lpszFileName)
{
	FILE *fp;
	errno_t err;
	if (FileExists(lpszFileName)) {
		// open existing file.
		err = fopen_s(&fp, lpszFileName, "r+b");
	}
	else {
		// create a new file.
		err = fopen_s(&fp, lpszFileName, "w+b");
	}

	return fp;
}

//---------------------------------------------------------------------------
DWORD CMyFile::GetFileSizeByName(LPCTSTR lpszFileName)
{
	if (!FileExists(lpszFileName)) return 0;
	struct _stat ST;
	// get file length.
	_stat(lpszFileName, &ST);
	UINT nFilesize = ST.st_size;
	return nFilesize;
}

//---------------------------------------------------------------------------
// get file name from full path name.
CString CMyFile::GetShortFileName(LPCSTR lpszFullPathName)
{
	CString strFileName = lpszFullPathName;
	CString strShortName;
	strFileName.TrimLeft();
	strFileName.TrimRight();
	int n = strFileName.ReverseFind('/');
	strShortName = strFileName.Right(strFileName.GetLength() - n - 1);
	return strShortName;
}
