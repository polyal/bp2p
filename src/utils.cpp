#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <random>
#include <errno.h>
#include "utils.h"

using namespace std;

const string Utils::applicationDir = "/bp2p/";
const string Utils::applicationOutDir = "/bp2p/out/";

void Utils::tokenize(const string& text, const string& sep, vector<string>& tokens)
{
	size_t pos = 0;
	size_t prevPos = 0;
	while ((pos = text.find(sep, pos)) != string::npos){
		int len = pos - prevPos;

		string token = text.substr(prevPos, len);
		tokens.push_back(token);

		pos += sep.size();
		prevPos = pos;
	}
	// get last string in the list
	if (pos > 0){
		string token = text.substr(prevPos);
		tokens.push_back(token);
	}
}

string Utils::getHomeDir()
{
	string home;
	char* cstrHome = getenv("HOME");
	if (cstrHome == NULL) home = "";
	else home = cstrHome;
	return home;
}

string Utils::getExePath()
{
	vector<char> buf(PATH_MAX);
	readlink("/proc/self/exe", buf.data(), buf.size());
	string path{buf.begin(), buf.end()};
	return path;
}

string Utils::getApplicationPath()
{
	string exePath = getExePath();
	size_t pos = exePath.find(applicationOutDir);
	if (pos != string::npos)
		exePath.erase(pos+applicationDir.size(), exePath.size());
	return exePath;
}

bool Utils::doesFileExist(const string& filename)
{
	struct stat st;   
  	return (stat (filename.c_str(), &st) == 0); 
}

void Utils::converToFullpath(const string& relPath, string& fullPath)
{
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

int Utils::listFileInDir(const string& dirName, vector<string>& filenames)
{
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir (dirName.c_str())) != NULL){
	  while ((ent = readdir (dir)) != NULL){
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

unsigned long long Utils::filesize(const char* filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return static_cast<unsigned long long>(in.tellg()); 
}

string Utils::bytesToHex(char* bytes, int len)
{
	std::stringstream digest;
	digest.setf(std::ios_base::hex, std::ios::basefield);
	digest.fill('0');
	for (int i = 0; i<len; i++)
		digest << std::setw(2) << (unsigned int)(unsigned char)(bytes[i]);
	const std::string ret = digest.str();
	return ret;
}

unsigned int Utils::value(char c)
{
	if (c >= '0' && c <= '9') { return c - '0'; }
	if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
	if (c >= 'a' && c <= 'f') { return c - 'a' + 10; }
	return -1;
}

char* Utils::hexToBytes(const string& strhex, int* size)
{
	const char* str = strhex.c_str();
	int bufSize = strhex.length() / 2;
	char* buf = new char[bufSize];
	for (int i = 0; i < bufSize; i++)
		buf[i] = value(str[2 * i]) * 16 + value(str[2 * i + 1]);
	*size = bufSize;
	return  buf;
}

int Utils::grnd(int min, int max)
{
	std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
    return dist(rng);
}
