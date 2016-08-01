// MyDownload.cpp: implementation of the MyDownload class.
//
//////////////////////////////////////////////////////////////////////

#include <afxsock.h>
#include "helper.h"
#include "Mydownload.h"
#include <io.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MyDownload::MyDownload()
{
	onFinish = NULL;
}

MyDownload::~MyDownload()
{

}

//add download task, take the URL as identifier
bool MyDownload::addDownloadTask(const char* remoteUrl, const char* localFolder)
{
	string remoteUrlString(remoteUrl);
	string localFolderString(localFolder);

	if (!existInVector(downloadListRemoteURLs, remoteUrlString)) {
		downloadListRemoteURLs.push_back(remoteUrlString);
		downloadListLocalFolders.push_back(localFolderString);
		return true;
	}
	else {
		return false;
	}
}

//check whether the task exists.
bool existInVector(vector<string>& array, string& str) {
	for (int k = 0; k<array.size(); k++) {
		if (array[k].compare(str) == 0)
			return true;
	}
	return false;
}

//start download
bool MyDownload::startDownload()
{
	this->start();
	return true;
}

//refactoring multithread downloading
void * MyDownload::run(void *)
{
	unsigned long temp = 0;
	unsigned long *downloaded = &temp;
	unsigned long totalSize = 1024;
	bool ret;

	while (downloadListRemoteURLs.size()>0)
	{
		cout << downloadListRemoteURLs[0] << endl;
		//default thread number is 3. The number can be changed but must be consistent.
		while (true) {
			//loop until the task is finished.
			ret = DownloadHelper(downloadListRemoteURLs[0].data(),
				downloadListLocalFolders[0].data(), downloaded, totalSize, THREAD_COUNT);
			if (ret) {
				cout << "True" << endl;
			}
			if (!exist(0)) {
				//file doesn't exist.
				cout << "Task failed,reconnecting..." << endl;
				Sleep(RECONNECT_INTERVAL);	//reconnecting after 10 seconds.
			}
			else {
				//download finished. delete the first task.
				vector<string>::iterator startIterator = downloadListRemoteURLs.begin();
				downloadListRemoteURLs.erase(startIterator);
				startIterator = downloadListLocalFolders.begin();
				downloadListLocalFolders.erase(startIterator);
				break;
			}
		}
	}
	if (onFinish != NULL) {
		onFinish();
	}
	return NULL;
}

//check if the file is already in the list.
bool MyDownload::exist(int index)
{
	string fileName = downloadListRemoteURLs[index].substr(downloadListRemoteURLs[index].find_last_of("/") + 1);
	string file(downloadListLocalFolders[index].data());
	file.append('\\' + fileName);
	return (_access(file.data(), 0) == 0);;
}

//function pointer fired upon task finish
void MyDownload::setOnFinish(void(*func)()) {
	onFinish = func;
}
