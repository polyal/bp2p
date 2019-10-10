#include <string>
#include <vector>

using namespace std;

/**********************************************************
*
*  returns 0 on success
*
*  archive:  path of the archived and compressed data
*  filename: a pointer to an array of filrenames to
*            be archived and compressed
*
**********************************************************/
int package(const string& package, const vector<string> filenames);


/**********************************************************
*
*  returns 0 on success
*
*  packageName:  the name of the package to be decompressed
*                and unarchived
*
**********************************************************/
int unpackage(const string& packageName);