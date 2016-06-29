#include <iostream>
#include "helper.h"
using namespace std;

unsigned long rdownloaded = 0;
DWORD CHttpGet::m_nFileLength = 0;
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

bool MyDownload(CString strUrl,
	CString strWriteFileName,
//	unsigned long *& downloaded,
	unsigned long & totalSize
//	CString strProxy,
//	int nProxyPort,
//	int nThread
	)
{
	CHttpGet b;
	CString strHostAddr;
	CString strHttpAddr;
	CString strHttpFilename;

	//monitor the downloaded data
//	downloaded = &rdownloaded;

	ParseURL(strUrl, strHostAddr, strHttpAddr, strHttpFilename);
	printf("%s\n%s\n",strHttpAddr, strHttpFilename);
	strWriteFileName += strHttpFilename;

	//cout << strHostAddr.GetBuffer(strHostAddr.GetLength()) << endl;
	//cout << strHttpAddr.GetBuffer(strHttpAddr.GetLength()) << endl;
	//cout << strHttpFilename.GetBuffer(strHttpFilename.GetLength()) << endl;
	printf("Before downloading.");
	if (!b.HttpDownLoad(strHostAddr, strHttpAddr, strHttpFilename, strWriteFileName,totalSize))
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
	// int nSectNum,
	DWORD &totalSize)
{
	int nHostPort = 80;

	SOCKET hSocket;
	sectinfo = new CHttpSect;              // allocate memory for information structure
	hSocket = ConnectHttp(strHostAddr, nHostPort);
	if (hSocket == INVALID_SOCKET) return 1;
	// send Http header, get content size

	SendHttpHeader(hSocket, strHostAddr, strHttpAddr, strHttpFilename, 0);
	totalSize = CHttpGet::m_nFileLength;
	DWORD nLen;
	DWORD nSumLen = 0;
	char szBuffer[1024];

	printf("send http header.\n");

	FILE *fpwrite;
	errno_t err;
	// open file writing
	printf("%s\n", strWriteFileName);
	if ((err = fopen_s(&fpwrite, strWriteFileName, "w+b")) != 0) {
		TRACE("File open error!\n");
		return FALSE;
	}

	printf("File Open OK! totalSize %d",totalSize);

	while (1)
	{
		if (nSumLen >= totalSize) break;
		nLen = recv(hSocket, szBuffer, sizeof(szBuffer), 0);

		// atomic operation.
		rdownloaded += nLen;

		if (nLen == SOCKET_ERROR) {
			TRACE("Read error!\n");
			fclose(fpwrite);
			return 1;
		}

		if (nLen == 0) break;
		nSumLen += nLen;
		TRACE("%d\n", nLen);

		// writing	
		fwrite(szBuffer, nLen, 1, fpwrite);
	}

	fclose(fpwrite);      // close file writing
	closesocket(hSocket);

	//totalSize = CHttpGet::m_nFileLength;

	//// sectinfo[i].szProxyAddr = strProxyAddr;      // proxy
	//// sectinfo[i].nProxyPort = nProxyPort;		   // Host address
	//sectinfo[0].szHostAddr = strHostAddr;       // Http file address
	//sectinfo[0].nHostPort = nHostPort;		   // Http file name
	//sectinfo[0].szHttpAddr = strHttpAddr;       // proxy port number
	//sectinfo[0].szHttpFilename = strHttpFilename;// Host port number

	//CString strTempFileName;
	//strTempFileName.Format("%s_%d", strWriteFileName, 0);
	//sectinfo[0].szDesFilename = strTempFileName;
	//// sectinfo[i].bProxyMode = bProxy;		       // mode
	//sectinfo[0].nStart = 0;
	//sectinfo[0].nEnd = m_nFileLength;
	//if (!HttpDownLoad(TEXT(""), 80, strHostAddr, nHostPort, strHttpAddr, strHttpFilename, strWriteFileName, false))
	//// if (!HttpDownLoad(TEXT(""), 80, strHostAddr, nHostPort, strHttpAddr, strHttpFilename, strWriteFileName, nSectNum, false))
	//	return FALSE;

	return TRUE;
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

BOOL CHttpGet::SendHttpHeader(SOCKET hSocket, CString strHostAddr,
	CString strHttpAddr, CString strHttpFilename, DWORD nPos)
{ 
	static CString sTemp;
	char cTmpBuffer[1024];

	// Line1: file path
	sTemp.Format("GET %s HTTP/1.1\r\n", strHttpAddr);
	printf("%s",sTemp);
	if (!SocketSend(hSocket, sTemp)) return FALSE;

	// Line2: host
	sTemp.Format("Host: %s\r\n", strHostAddr);
	printf("%s", sTemp);
	if (!SocketSend(hSocket, sTemp)) return FALSE;

	// Line3:connection
	sTemp = "Connection: keep-alive\r\n";
	printf("%s", sTemp);
	if (!SocketSend(hSocket, sTemp)) return FALSE;

	// Line4:data type.
	sTemp.Format("Accept: */*\r\n");
	printf("%s", sTemp);
	if (!SocketSend(hSocket, sTemp)) return FALSE;

	// Line5:browser type.
	sTemp.Format("User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36\r\n");
	printf("%s", sTemp);
	if (!SocketSend(hSocket, sTemp)) return FALSE;

	//
	sTemp = "Accept - Encoding: gzip, deflate, sdch\r\nAccept-Language: zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n";
	printf("%s", sTemp);
	if (!SocketSend(hSocket, sTemp)) return FALSE;

	// Line6:referer.
	sTemp.Format("Referer: %s%s\r\n", strHostAddr,strHttpAddr);
	printf("%s", sTemp);
	if (!SocketSend(hSocket, sTemp)) return FALSE;

	// Data Range.
	// sTemp.Format("Range: bytes=%d-\r\n", nPos);
	// if (!SocketSend(hSocket, sTemp)) return FALSE;

	// LastLine
	sTemp.Format("\r\n");
	printf("%s", sTemp);
	if (!SocketSend(hSocket, sTemp)) return FALSE;

	// get http header.
	int i = GetHttpHeader(hSocket, cTmpBuffer);
	if (!i)
	{
		TRACE("获取HTTP头出错!\n");
		return 0;
	}

	// check header.
	sTemp = cTmpBuffer;
	printf("%s",sTemp);
	if (sTemp.Find("404") != -1) return FALSE;

	// get the file size
	m_nFileLength = GetFileLength(cTmpBuffer);
	printf("%d\n", m_nFileLength);

	TRACE(CString(cTmpBuffer).GetBuffer(200));

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
		TRACE("WSAStartup failed:%d\n",err);
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
