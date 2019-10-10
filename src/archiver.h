// https://github.com/libarchive/libarchive/wiki/Examples

#include <string>
#include <vector>

using namespace std;

class  Archiver
{
private:
	string archiveName;
	vector<string> filenames;

public:
	Archiver();
	Archiver(const string& archiveName);
	Archiver(const string& archiveName, const vector<string>& filenames);

	void setup(const string& archiveName);
	void setup(const string& archiveName, const vector<string>& filenames);

	int archive();
	int extract();

private:
	int copyData(struct archive *ar, struct archive *aw);
};