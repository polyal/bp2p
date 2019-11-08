

using namespace std;

class Node{
private:
	enum DevStatus
	{
		READY,
		BUSY
	};
	
	map<DeviceDescriptor, vector<DeviceDescriptor>> local2remote;
	map<DeviceDescriptor, DevStatus> remoteStatus;
	

public:
	vector<DeviceDescriptor> localDevs;
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

