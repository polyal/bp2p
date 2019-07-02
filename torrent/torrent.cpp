#include "torrent.h"
#include <stdio.h>
#include <iostream>

// determine file/dir
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

int Torrent::createTorrent (char* filename){
	cout << "createTorrent\n";

	return 0;
}

Torrent::Torrent(char* filename){

}

Torrent::Torrent(){
	numPieces = 0;
	filename[0] = '\0';
	torrentLocation[0] = '\0';
}



int main(int argc, char *argv[]){

	Torrent newTorrent(argv[0]);

    cout << "helloworld\n";

    return 0;

}