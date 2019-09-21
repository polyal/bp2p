#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>

using namespace std;

class Utils{
	public:
		// application type utils
		static const string applicationDir;
		static string getHomeDir();
		static string getApplicationPath();

		// string utils
		static void tokenize(const string& text, const string& sep, vector<string>& tokens);

		// file utils
		static bool doesFileExist(const string& filename);
		static void converToFullpath(const string& relPath, string& fullPath);
		static int isRegulaFile(const string& path);
		static int listFileInDir(const string& dirName, vector<string>& filenames);

		// hex and byte utils
		static string bytesToHex(char* bytes, int len);
		static unsigned int value(char c);
		static char* hexToBytes(const string& strhex, int* size);
};

#endif
