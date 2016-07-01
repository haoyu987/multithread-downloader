#include <afx.h>
#include <afxwin.h>   
#include "helper.h"
#include <stdio.h>
#include <iostream>

using namespace std;
int _tmain(int argc, _TCHAR* argv[])
{
	SOCKET hSocket = dealsocket.GetConnect("www.nyu.edu", 80);
	unsigned long totalSize;
	CString strURL("http://www.nyu.edu/content/dam/nyu/nyuToday/images/homepage/home%2020160310_honorarydegree.jpg");
	CString strWriteFileName("C:\\Users\\hao\\Downloads\\");
	bool p = MyDownload(strURL, strWriteFileName,
		//unsigned long *& downloaded,
		totalSize);
	if (p) {
		printf("complete");
	}
	cin.get();
	return 0;
}
