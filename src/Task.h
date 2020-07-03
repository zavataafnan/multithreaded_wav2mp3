#ifndef CTask_H
#define CTask_H


#include <iostream>
#include <fstream>

#include <lame.h>
#include "sndfile.h"
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <vector>
#include <stack>
#include "datathread.h"


#define NAX_NUM_THREADS   32



class CTask
{

public:
    explicit CTask();
	~CTask();


private:

	DataSharing * pdata_sharing[NAX_NUM_THREADS];
    int nCoreNumber;
    pthread_t threads[NAX_NUM_THREADS];
	std::ofstream * mp3Obj[NAX_NUM_THREADS];

	std::stack <std::string> filelist;
	SNDFILE	*current_file;
	SF_INFO	current_sfinfo;
	int readcount;
	std::ofstream * current_mp3;


public:

    void convert_wav_to_mp3(SNDFILE * infile, std::string output_filename, SF_INFO sfinfo);
    void SetAndAllocateBufferForData(int number);

};

#endif // CTask_H
