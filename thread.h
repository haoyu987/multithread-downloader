#pragma once
#ifndef _THREAD_SPECIFICAL_H__
#define _THREAD_SPECIFICAL_H__

#define    WIN32_LEAN_AND_MEAN   // prevent windows.h import winsock.h which may clash with winsock2.h
#include   <windows.h>   

static unsigned int __stdcall threadFunction(void *);

class Thread {
	friend unsigned int __stdcall threadFunction(void *);
public:
	Thread();
	virtual ~Thread();
	int start(void * = NULL);// thread start function, taking void pointer as parameter.
	void stop();
	void* join();// Wait until current thread terminates.
	void detach();// do not wait.
	static void sleep(unsigned int);// sleep current thread for some specified time.

protected:
	virtual void * run(void *) = 0;// call thread function.

private:
	HANDLE threadHandle;
	bool started;
	bool detached;
	void * param;
	unsigned int threadID;
};

#endif
