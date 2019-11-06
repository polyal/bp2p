

using namespace std;

class Node{
private:
	inline static const string commString = "bp2p";
	inline static const string commSeparator = "||";

	vector<Device> nodes;
	vector<Device> localDevices;

public:
	Node();

	void processRequest(const vector<char>& req, vector<char>& resp);
	void createRequest();

	static void server(BTDevice dev);
	
	static const string applicationDir;
	string getApplicationPath();
};

