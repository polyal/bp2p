

using namespace std;

class Node{
private:
	vector<DeviceDescriptor> localDevs;
	vector<DeviceDescriptor> remoteDevs;

public:
	Node();

	void findLocalDevs();
	void scanForDevs();

	void processRequest(const vector<char>& req, vector<char>& resp);
	void createRequest();

	thread createServerThread(DeviceDescriptor servDev);

	static void server(DeviceDescriptor dev);
	
	static const string applicationDir;
	string getApplicationPath();
};

