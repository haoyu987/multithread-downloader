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
