
#include "datathread.h"

DataSharing::DataSharing()
{
	buf = new float[(BLOCK_SIZE + BLOCK_E_SIZE + BLOCK_E_SIZE) * 2];
	//as the LAME recommendation for considering 
	//the output buffer in the worst case
	mp3_buf = new unsigned char[(BLOCK_SIZE + BLOCK_E_SIZE + BLOCK_E_SIZE) * 3/4 + 7200];
	//std::cout << "DataSharing construction is calling..." << std::endl;

	extra_padding_previous = new float[BLOCK_E_SIZE * 2];
	extra_padding_next = new float[BLOCK_B_SIZE * 2];

	pcm_r = NULL;
	pcm_l = NULL;

}
DataSharing::~DataSharing()
{
	delete[] buf; 
	delete[] mp3_buf; 
	delete[] extra_padding_next;
	delete[] extra_padding_previous;

	if(pcm_r != NULL)
		delete[] pcm_r;
	if (pcm_l != NULL)
		delete[] pcm_l;

	buf = NULL;
	mp3_buf = NULL;
	//std::cout << "DataSharing destruction is calling..." << std::endl;
}