#ifndef _DATA_THREAD_
#define _DATA_THREAD_


#include <iostream>
#include <fstream>
#include <vector>

#include <lame.h>
#include "sndfile.h"
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

/* 
 * the overlapped and main data section sizes 
 */
const static int BLOCK_B_SIZE = (int)1152 * 40;
const static int BLOCK_E_SIZE = BLOCK_B_SIZE;
const static int BLOCK_SIZE = (int)(1152 * 2 * 210);

struct frame_detail
{
	short int * pcm_r;
	short int * pcm_l;
	int frame_num;
	int frame_pos;

	frame_detail()
	{
		pcm_r = new short int[1152];
		pcm_l = new short int[1152];

	}

	~frame_detail()
	{
		delete[] pcm_r;
		delete[] pcm_l;
	}
};


struct DataSharing
{
	DataSharing();
	~DataSharing();

	float * buf;
	int nwav_frame;
	unsigned char * mp3_buf;
	int write_mp3_data;
	int nsample_raw_sata;
	int error_status;
	SF_INFO	* sfinfo;
	int bitrate;
	unsigned char flush_condition;
	lame_global_flags *plame_flag;

	std::ofstream * mp3_output;
	unsigned char should_run;
	unsigned char should_close_the_mp3;

	float * extra_padding_previous;
	float * extra_padding_next;
	int nsample_extra_data;

	short int * pcm_l;
	short int * pcm_r;

	int nsample_decoded_buffer;

	int cut_location_from_begin;

	int multithread_analyse;

	std::vector <frame_detail *> frame_data;
};

#endif