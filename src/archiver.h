/**********************************************************
**
**    archiver.h
**    Lorant Polya
**    Oct 10, 2019
**
**    This file defines all the needed functionality
**    for creating and extracting archives.
**
**    I utilize the lib archive library and essentially
**    just wrapped the functionility in a class to make
**    it easier to use.  Although, now it is less flexible.
**
**    https://github.com/libarchive/libarchive/wiki/Examples
**********************************************************/

#include <string>
#include <vector>

using namespace std;

class  Archiver
{
private:
	string name;
	vector<string> filenames;

public:
	///////////////////////////////////////////////////////////
	//  Initializes a new archive.  The single argument
	//  constructor is used for extracting the provided archive.
	//  The second argument is the list of files that will be
	//  archived, this is obviosuly only used for creating an
	//  archive
	//
	//  name:  the name of the archive
	//  files: files which are going to be archived
	Archiver();
	Archiver(const string& name);
	Archiver(const string& name, const vector<string>& filenames);

	///////////////////////////////////////////////////////////
	//  See constructors
	void setup(const string& name);
	void setup(const string& name, const vector<string>& filenames);

	int archive();
	int extract();

private:
	int copyArchive(struct archive *ar, struct archive *aw);
};