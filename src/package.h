#include <string>
#include <vector>

using namespace std;


class Package
{
public:
	Package();
	Package(const string& name);
	Package(const string& name, const vector<string> filenames);

	void setup(const string& name);
	void setup(const string& name, const vector<string> filenames);

	int package();
	int unpackage();

private:
	static const string tempName;
	string name;
	vector<string> files;

	int archive();
	int extract();
	int compress();
	int decompress();
};
