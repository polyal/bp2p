#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include "utils.h"

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