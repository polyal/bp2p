#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <fstream>
#include <unordered_set>

using namespace std;

class Utils{
public:
	// application type utils
	inline static const string applicationDir = "/bp2p/";
	static string getHomeDir();
	static string getApplicationPath();

	// string utils
	static void tokenize(const string& text, const string& sep, vector<string>& tokens);

	// file utils
	static bool doesFileExist(const string& filename);
	static void converToFullpath(const string& relPath, string& fullPath);
	static int isRegulaFile(const string& path);
	static int listFileInDir(const string& dirName, vector<string>& filenames);
	static unsigned long long filesize(const char* filename);

	// hex and byte utils
	static string bytesToHex(char* bytes, int len);
	static unsigned int value(char c);
	static char* hexToBytes(const string& strhex, int* size);

	// random numbers
	static int grnd(int min, int max);

	template <typename T>
	static unordered_set<T> filterDuplicates(const vector<T>& list)
	{
		unordered_set<T> newItems;
		for (auto const& item : list){
			newItems.insert(item);
		}
		return newItems;
	}
};

#endif
