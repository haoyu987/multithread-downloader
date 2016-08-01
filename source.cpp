// DownloadTest.cpp : The main entry point for the application.
//
#include <afx.h>
#include <afxwin.h>   
#include "helper.h"
#include "MyDownload.h"
#include <stdio.h>
#include <iostream>
using namespace std;



UINT ThreadCount(void* pParam);

unsigned long temp = 0;
unsigned long *downloaded = &temp;
unsigned long totalSize = 1024;

//called upon finishing.
void testFunc() {
	cout << "Finished!" << endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	////////////////////////////////////////////////////////////////////////////
	//DownloadHelper example:
	//void(*func)();
	//func = testFunc;
	//MyDownload downloadHelper;
	//downloadHelper.setOnFinish(testFunc);
	//	downloadHelper.addDownloadTask("http://www.nyu.edu/content/dam/nyu/nyuToday/images/homepage/home%2020160714_Brademas.jpg","C:\\Users\\hao\\Desktop\\");
	//downloadHelper.addDownloadTask("http://www.nyu.edu/content/dam/nyu/nyuToday/images/homepage/home%2020160310_honorarydegree.jpg", "C:\\Users\\hao\\Desktop\\");
	//downloadHelper.startDownload();
	//downloadHelper.join();

	////////////////////////////////////////////////////////////////////////////
	//use DownloaderHelper

	//with speed display and progress indicator.

	AfxBeginThread(ThreadCount, 0);
//	bool success = DownloadHelper("http://www.nyu.edu/content/dam/nyu/nyuToday/images/homepage/home%2020160714_Brademas.jpg", "C:\\Users\\hao\\Desktop\\", downloaded, totalSize, 4);
	bool success = DownloadHelper("http://download.winzip.com/winzip205.exe", "C:\\Users\\hao\\Desktop\\", downloaded, totalSize, 4);
//	bool success = DownloadHelper("http://www.nyu.edu/content/dam/nyu/nyuToday/images/homepage/home%2020160310_honorarydegree.jpg", "C:\\Users\\hao\\Desktop\\",downloaded,totalSize,4);

	if (!success) printf("Download failed.");

	cin.get();
	return 0;
}

UINT ThreadCount(void* pParam)
{
	unsigned long temp = 0;
	while (1) {
		cout << "total: " << totalSize << "KB downloaded:" << ((double)(*downloaded)*100 / (double)totalSize) << "%  speed " << (*downloaded - temp) / 1024 << "KB/S " << endl;
		temp = *downloaded;
		if (*downloaded == totalSize)
		{
			cout << "finished!" << endl;
			return 0;
		}
		Sleep(2000);
	}
	return 0;
}
