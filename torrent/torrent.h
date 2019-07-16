/**********************************************************
**
**    Torrent.h
**    Lorant Polya
**    June 30, 2019
**
**    This file defines all the needed functionality
**    for creating and using torrent files.
**
**    The general idea is to take in a filename or
**    folder and compress it.  Then take the compressed
**    file and split it up into chunks.  The chunks will
**	  be kept track of using this torrent file.
**
**********************************************************/

#include <iostream>
#include <linux/limits.h>
#include <list>
#include <tuple>

using namespace std;

class Torrent {
	private:
		static const int chunkSize = 32768;  //256 kilobyte chunk size
		int numPieces;
		char filename[PATH_MAX];
		char torrentLocation[PATH_MAX];
		list <tuple<char*, bool>> chuncks;

 	public:
 		Torrent();
 		Torrent(const char* filename); // takes the filename of an already existing torrent
 		Torrent(const char* archive, const char** files); // takes a list of files to be packaged


		int createTorrent (char* filename); // takes the filename of a file to be converted into a torrent
};