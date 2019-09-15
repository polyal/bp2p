#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "utils.h"

const string Utils::applicationDir = "/bp2p/";

void Utils::tokenize(const string& text, const string& sep, vector<string>& tokens){
	size_t pos = 0;
	size_t prevPos = 0;

	while ((pos = text.find(sep, pos)) != string::npos){
		int len = pos - prevPos;

		string token = text.substr(prevPos, len);
		tokens.push_back(token);

		pos = pos + 2;
		prevPos = pos;
	}

	// get last string in the list
	if (pos > 0){
		string token = text.substr(prevPos);
		tokens.push_back(token);
	}
}

string Utils::getHomeDir(){
	string home;
	char* cstrHome = getenv("HOME");
	
	if (cstrHome == NULL) home = "";
	else home = cstrHome;

	return home;
}

string Utils::getApplicationPath(){
	string home = getHomeDir();
	return home + applicationDir;
}

bool Utils::doesFileExist(const string& filename){
	struct stat st;   
  	return (stat (filename.c_str(), &st) == 0); 
}

void Utils::converToFullpath(const string& relPath, string& fullPath){
	const char* cstrRelPath;
	char* cstrFullPath = NULL;
	fullPath = "";

	cstrRelPath = relPath.c_str();
	cstrFullPath = realpath(cstrRelPath, NULL);

	if (cstrFullPath != NULL){
		fullPath = cstrFullPath;
		free(cstrFullPath);
	}
}

int Utils::isRegulaFile(const string& path)
{
    struct stat pathStat;
    stat(path.c_str(), &pathStat);
    return S_ISREG(pathStat.st_mode);
}

int Utils::listFileInDir(const string& dirName, vector<string>& filenames){
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir (dirName.c_str())) != NULL) {
	  while ((ent = readdir (dir)) != NULL) {
	  	string filename {ent->d_name};
	  	string fullpath = dirName + filename;

	  	if (isRegulaFile(fullpath)){
	  		filenames.push_back(filename);
	  	}
	  }

	  closedir (dir);
	} else {
	  cout << "Error: Couldn't open dir" << endl;
	  return errno;
	}

	return 0;
}