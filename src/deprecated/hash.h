#include <openssl/sha.h>
#include <vector>
#include <string>
#include <memory>

using namespace std;


///////////////////////////////////////////////////////////
//  Abstract Hash class
//  Defines basic functionality for hashing
class Hash
{
protected:
	vector<char> hash;
	string strHash;

public:
	Hash();

	virtual void init() = 0;
	virtual void update(const vector<char>& buff, const int size) = 0;
	virtual vector<char> final() = 0;
	virtual vector<char> computeHash(const vector<char>& buff, const int size) = 0;
	virtual vector<char> getBytes();
	virtual string toString();
};

///////////////////////////////////////////////////////////
//  Sha256 Hashing class
class Sha256 : public Hash
{
protected:
	SHA256_CTX ctx;

public:
	Sha256();

	void init();
	void update(const vector<char>& buff, const int size);
	vector<char> final();
	vector<char> computeHash(const vector<char>& buff, const int size);
};

///////////////////////////////////////////////////////////
//  Hashes files in chunks or as a whole
class Sha256FileHasher
{
private:
	static const int chunkSize = 32768;
	string filename;
	vector<Sha256> chunkHashs;
	Sha256 fileHash;

public:
	Sha256FileHasher();
	Sha256FileHasher(const string& filename);

	void open(const string& filename);     // used in conjuction with no argument constructor
	vector<Sha256> computeFileChunkHash(); // splits file into chunks, then returns a vector of hashed chunks
	Sha256 computeFileHash();              // hashes the file as a whole
	vector<string> chunkHashsToString();   // converts vector of hashes to vector of lists
	string fileHashToString();             // converts file hash to string
};