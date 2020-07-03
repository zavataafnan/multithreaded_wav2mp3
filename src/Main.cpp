/**
	* wavetomp3convertor is a console application convert the 
	* wav audio format file to mp3.

	* modified lame library to support multi thread wave convector 
	* is linked as a static library.

	* pthread4w is used as POSIX library for windows and default POSIX for
	* Linux distributional. 

	@written by: Mostafa jabaroutimoghaddam
*/

#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <chrono>
//just use the thread lib to find the appropriate number of logical core in the system...
#include <thread>

#include "Task.h"


struct path_leaf_string
{
	std::string operator()(const std::filesystem::directory_entry& entry) const
	{
		return entry.path().string();
	}
};

/*
* read the files in the directory.
*/

void read_directory(const std::string& name, std::vector<std::string>& v)
{
	std::filesystem::path p(name);
	std::filesystem::directory_iterator start(p);
	std::filesystem::directory_iterator end;
	std::transform(start, end, std::back_inserter(v), path_leaf_string());
}

int main(int argc, char *argv[])
{
	if (argc != 2) {

		std::cout << "Usage: wave folder to convert is missing!!!" << std::endl;
		return 1;
	}

	std::string foldername = argv[1];

	std::vector<std::string> filelist;
	
	if (std::filesystem::is_directory(std::filesystem::path(foldername)))
	{
		read_directory(foldername, filelist);
	}
	else
	{
		std::cout << "the folder is not exist..." << std::endl;
		return 1;
	}

	unsigned int ncorenumber = std::thread::hardware_concurrency();
	SNDFILE	*file;
	SF_INFO	sfinfo;

	CTask * jobhandler = new CTask();

	/*
	* set the available logical core for multi thread run.
	*/

	jobhandler->SetAndAllocateBufferForData(ncorenumber);

	
	for (auto const& filename : filelist) {
		
		std::filesystem::path p(filename);
		if (p.extension() == std::filesystem::path(std::string(".wav")))
		{
			
			std::filesystem::path outputfilename(filename);
			outputfilename.replace_extension(".mp3");

			
			if ((file = sf_open(filename.c_str(), SFM_READ, &sfinfo)) == NULL)
			{
				std::cout << "unable to open WAV file. It may be currupted or is not supported." << std::endl;
				std::cout << sf_strerror(NULL) << std::endl;
			}
			else
			{
				/* 
				* analyze the wav file. 
				*/
				std::cout << "the file name is " << filename.c_str() << std::endl;
				std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

				jobhandler->convert_wav_to_mp3(file, outputfilename.string(), sfinfo);

				std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
				std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
			}

			sf_close(file);
		}
	}
	
	delete jobhandler;

	std::cout << "finish the process.." << std::endl;
}

