#pragma once
#ifndef _THREAD_SPECIFICAL_H__
#define _THREAD_SPECIFICAL_H__

#define    WIN32_LEAN_AND_MEAN   // prevent windows.h import winsock.h which may clash with winsock2.h
#include   <windows.h>   

static unsigned int __stdcall threadFunction(void * object);

class Thread {
	friend unsigned int __stdcall threadFunction(void * object);
public:
	Thread();
	virtual ~Thread();
	int start(void * pra = NULL);// thread start, taking void pointer as parameter.
	void stop();
	void* join();// Wait until current thread terminates.
	void detach();
	static void sleep(unsigned int delay);// sleep current thread for some specified time.

protected:
	virtual void * run(void * param) = 0;// call thread function.

private:
	HANDLE threadHandle;
	bool started;
	bool detached;
	void * param;
	unsigned int threadID;
};

#endif
