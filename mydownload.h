#pragma once
// MyDownload.h: interface for the MyDownload class.
//
//////////////////////////////////////////////////////////////////////
#ifndef mydownload___
#define mydownload___

#include <vector>
#include <string>
#include <iostream>
using namespace std;
#include "Thread.h"

//number of threads for every task
#define THREAD_COUNT 3
//reconnect interval
#define RECONNECT_INTERVAL 10000	

class MyDownload : public Thread
{
public:
	void * run(void *);
	bool startDownload();
	bool addDownloadTask(const char* remoteUrl, const char* localFolder);
	MyDownload();
	virtual ~MyDownload();
	//set the function when finish, take the function pointer as parameter
	void setOnFinish(void(*func)());
private:
	//check whether the file exists.
	//take the index as parameter
	bool exist(int index);
	//vector stores URLs
	vector<string> downloadListRemoteURLs;
	//vector stores file path
	vector<string> downloadListLocalFolders;
	//function called on finish
	void(*onFinish)();

};

bool existInVector(vector<string>& array, string& str);

#endif //
