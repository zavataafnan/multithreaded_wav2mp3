/**
	* CTask class handle the encoding and multi threading
	* approach.

	@written by: Mostafa jabaroutimoghaddam
*/
#include <filesystem>

#include "Task.h"
#include "datathread.h"

   
 


/* 
* using mutex for sychronization of std::cout in the error situation.
*/
static pthread_mutex_t func_mutex = PTHREAD_MUTEX_INITIALIZER;

lame_global_flags* init_lame_global_flag(DataSharing *pdata)
{
	lame_global_flags* plame_flag;
	plame_flag = lame_init();
	if (NULL == plame_flag)
	{
		pthread_mutex_lock(&func_mutex);
		std::cout << "ERROR in lame_init()";
		pthread_mutex_unlock(&func_mutex);
		pdata->error_status = 1;
		return NULL;
	}

	/* Set encoding parameters */
	lame_set_num_channels(plame_flag, pdata->sfinfo->channels);

	/* set the bitrate to 128k */
	lame_set_brate(plame_flag, 128);

	
	/* set 5 for good quality */
	lame_set_quality(plame_flag, 5);


	if (pdata->sfinfo->channels == 1)
		lame_set_mode(plame_flag, MONO);
	else
		lame_set_mode(plame_flag, STEREO);

	/* the encoder is in CBR mode not VBR! */
	lame_set_bWriteVbrTag(plame_flag, 0);

	//lame_set_VBR(plame_flag, vbr_off);

	if (lame_init_params(plame_flag) != 0)
	{
		pthread_mutex_lock(&func_mutex);
		std::cout << "ERROR in lame_init_params()";
		pthread_mutex_unlock(&func_mutex);
		lame_close(plame_flag); plame_flag = NULL;
		pdata->error_status = 1;
		return NULL;
	}


	return plame_flag;

}

void* worker(void* arg)
{
	DataSharing *pdata = (DataSharing *)arg;
	pdata->error_status = 0;


	lame_global_flags* plame_flag = init_lame_global_flag(pdata);

	/* 
	* for encoding the extra part encoding 
	and pass the main_data_begin to the main part 
	*/
	lame_global_flags* plame_flag_overlapped = init_lame_global_flag(pdata);


	lame_set_previous_internal_flag(plame_flag, plame_flag_overlapped);
	lame_set_next_chunk_of_data(plame_flag,
		(BLOCK_SIZE + BLOCK_B_SIZE) / pdata->sfinfo->channels / 1152, 
		BLOCK_B_SIZE / pdata->sfinfo->channels / 1152);

	lame_set_next_chunk_of_data(plame_flag_overlapped,
		(BLOCK_SIZE + BLOCK_B_SIZE) / pdata->sfinfo->channels / 1152,
		BLOCK_B_SIZE / pdata->sfinfo->channels / 1152);

	lame_set_multithread_overlapped(plame_flag, pdata->multithread_analyse);

	

	const double sc16 = (double)0x7FFF + 0.4999999999999999;
	short int * converted_data = new short int[pdata->nsample_raw_sata];

	short int * converted_overlapped_data = new short int[pdata->nsample_extra_data];


	/*
	* data read in float from the sndfile library. In this way, all bitrate formats
	* are support without using switch case for each format.
	* input of the lame_encode_buffer is short int type, the float data should be
	* scale to short int range.
	*/


	
	{
		for (int i = 0; i < pdata->nsample_raw_sata; i++)
			converted_data[i] = (short int)((double)pdata->buf[i] * sc16);

		for (int i = 0; i < pdata->nsample_extra_data; i++)
			converted_overlapped_data[i] = (short int)((double)pdata->extra_padding_next[i] * sc16);

		/*
		* first encode the overlapped section and then encode the main part. 
		* pass the header information of header to the main encoding part.
		*/
		if (pdata->sfinfo->channels == 1)
		{
			pdata->write_mp3_data = lame_encode_buffer(plame_flag_overlapped,
				converted_overlapped_data, NULL,
				pdata->nsample_extra_data / pdata->sfinfo->channels,
				pdata->mp3_buf, BLOCK_SIZE * 4);


			pdata->write_mp3_data = lame_encode_buffer(plame_flag,
				converted_data, NULL,
				pdata->nsample_raw_sata / pdata->sfinfo->channels,
				pdata->mp3_buf, BLOCK_SIZE * 4);

			lame_get_multithread_parameters(plame_flag, &pdata->cut_location_from_begin);

		}
		else
		{
			pdata->write_mp3_data = lame_encode_buffer_interleaved(plame_flag_overlapped,
				converted_overlapped_data,
				pdata->nsample_extra_data / pdata->sfinfo->channels,
				pdata->mp3_buf, (BLOCK_SIZE + BLOCK_E_SIZE + BLOCK_E_SIZE) * 3 / 4 + 7200);

			pdata->write_mp3_data = lame_encode_buffer_interleaved(plame_flag,
				converted_data,
				(pdata->nsample_raw_sata) / pdata->sfinfo->channels,
				pdata->mp3_buf, (BLOCK_SIZE + BLOCK_E_SIZE + BLOCK_E_SIZE) * 3 / 4 + 7200);

			lame_get_multithread_parameters(plame_flag, &pdata->cut_location_from_begin);

		}
	} 

	lame_close(plame_flag);
	lame_close(plame_flag_overlapped);
	delete[] converted_data;
	delete[] converted_overlapped_data;

	return 0;

}


CTask::CTask()
{
	nCoreNumber = 1;
}

CTask::~CTask()
{
	for (int i = 0; i < nCoreNumber; i++)
	{
		delete pdata_sharing[i];
		pdata_sharing[i] = NULL;
	}
}

void CTask::SetAndAllocateBufferForData(int number)
{
	/*
	* the maximum number of threads consider as 32. pthread windows version
	* has limitation of 64.
	* DataSharing is allocation in the heap and dependent to the number of threads that
	* run in each epoch.
	*/

	nCoreNumber = number <= NAX_NUM_THREADS ? number : NAX_NUM_THREADS;
	for (int i = 0; i < nCoreNumber; i++)
	{
		pdata_sharing[i] = new DataSharing();
	}
}



void CTask::convert_wav_to_mp3(SNDFILE * infile, std::string output_filename, SF_INFO sfinfo)
{
	sf_count_t readcount = 0;
	int result_code;
	int read_counter_the_last = 0;
	DataSharing * pdata_in_margin_loop = new DataSharing();
	

	std::ofstream mp3(output_filename, std::ios_base::binary | std::ios_base::out);
	int nwav_counter = -1;

	do
	{
		/*
		*  thread number base on the available logical core.
		*/
		int thread_counter = 0;
		readcount = 0;
		int number_of_read_from_wav = 0;

		/*
		* nwav_counter is the number of run with the nCoreNumber thread
		*/
		nwav_counter++;

		/*
		* atart to construct the thread data : #T1 (BEGINNING OVERLAP + MAIN DATA + END OVERLAP) #T2 .....
		*/
		do
		{
			if (nwav_counter == 0 && thread_counter == 0)
			{
				/*
				* for the first run, the number of data that should be run is 2*BLOCK_EXTRA + BLOCK_SIZE
				*/
				number_of_read_from_wav = (BLOCK_B_SIZE + BLOCK_SIZE + BLOCK_E_SIZE) / sfinfo.channels;

				readcount = sf_readf_float(infile, 
					pdata_sharing[thread_counter]->buf, 
					number_of_read_from_wav);
			} 
			else
			{
				/*
				* the number data that should be read is BLOCK_SIE + BLOCK_EXTRA (BLOCK_E_SIZE)
				*/
				number_of_read_from_wav = (BLOCK_SIZE + BLOCK_E_SIZE) / sfinfo.channels;

				readcount = sf_readf_float(infile, 
					pdata_sharing[thread_counter]->buf + BLOCK_E_SIZE
					, number_of_read_from_wav);
			}

			/* 
			* the end of the file reaches. else the number of data for encode should be defined.
			*/
			if (readcount < number_of_read_from_wav)
			{
				
				pdata_sharing[thread_counter]->multithread_analyse = 0;

				pdata_sharing[thread_counter]->nsample_raw_sata = (int)readcount * sfinfo.channels;
				pdata_sharing[thread_counter]->nsample_extra_data = BLOCK_E_SIZE;

				/*
				set the readcount = 0 and exit and run the thread for the available data
				*/
				readcount = 0;
			}
			else
			{
				pdata_sharing[thread_counter]->multithread_analyse = 1;
				pdata_sharing[thread_counter]->nsample_raw_sata = BLOCK_B_SIZE + BLOCK_SIZE + BLOCK_E_SIZE;
				pdata_sharing[thread_counter]->nsample_extra_data = BLOCK_E_SIZE;
			} /*readcount < number_of_read_from_wav*/


			if (thread_counter == 0)
			{
				/*
				* In each run, In the first thread, should fill the extra padding...
				*/
				if (nwav_counter == 0)
				{
					for (int i = 0; i < BLOCK_E_SIZE; i++)
					{
						pdata_sharing[thread_counter]->extra_padding_next[i] =
							pdata_sharing[thread_counter]->buf[i + (BLOCK_B_SIZE + BLOCK_SIZE)];

					}
				}
				else
				{
					for (int i = 0; i < BLOCK_E_SIZE; i++)
					{
						pdata_sharing[thread_counter]->buf[i] = pdata_in_margin_loop->extra_padding_next[i];

						pdata_sharing[thread_counter]->extra_padding_next[i] =
							pdata_sharing[thread_counter]->buf[i + (BLOCK_B_SIZE + BLOCK_SIZE)];
					}
				}
			}
			else /*thread_counter == 0*/
			{
				/*
				fill the extra bytes from the previous thread data.
				*/
				pdata_sharing[thread_counter]->nsample_extra_data = BLOCK_E_SIZE;
				for (int i = 0; i < BLOCK_B_SIZE ; i++)
				{
					pdata_sharing[thread_counter]->buf[i] =
						pdata_sharing[thread_counter - 1]->extra_padding_next[i];

					pdata_sharing[thread_counter]->extra_padding_next[i] =
						pdata_sharing[thread_counter]->buf[i + (BLOCK_SIZE + BLOCK_B_SIZE)];
				}
			} /*thread_counter == 0*/
			thread_counter++;

		} while (readcount > 0 && thread_counter < nCoreNumber);


		/*
		run the thread and wait to finish each epoch.
		*/

		if (thread_counter > 0)
		{
			for (int i = 0; i < thread_counter; ++i)
			{
				pdata_sharing[i]->sfinfo = &sfinfo;
				pdata_sharing[i]->flush_condition = 0;
				pdata_sharing[i]->bitrate = sf_current_byterate(infile);
				result_code = pthread_create(&threads[i], 0, worker, (void*)pdata_sharing[i]);
			}

			/*wait to all threads convert its data. */
			for (int i = 0; i < thread_counter; ++i) {
				result_code = pthread_join(threads[i], 0);
			}

			std::cout << "number of the thread running is : " << thread_counter 
				<< "   **** counter is : " << nwav_counter << std::endl;


			for (int i = 0; i < thread_counter; ++i) 
			{
				int offset = 0;
					if (i == 0)
					{
						if (nwav_counter > 0)
							offset = pdata_in_margin_loop->cut_location_from_begin;
						else
							offset = 0;						
					}
					else
					{
						offset = pdata_sharing[i - 1]->cut_location_from_begin;
					}

					if (i == thread_counter - 1)
					{
						pdata_in_margin_loop->cut_location_from_begin = pdata_sharing[i]->cut_location_from_begin;

						for (int n = 0; n < BLOCK_B_SIZE; n++)
						{
							pdata_in_margin_loop->extra_padding_next[n] = pdata_sharing[i]->extra_padding_next[n];
						}
					}

				mp3.write(reinterpret_cast<char*>(pdata_sharing[i]->mp3_buf + offset), pdata_sharing[i]->write_mp3_data - offset);
			} /*for (unsigned int i = 0; i < thread_counter; ++i) */
		} /*thread_counter > 0*/

	} while (readcount > 0);

	mp3.close();
	delete pdata_in_margin_loop;
	return;
} 

