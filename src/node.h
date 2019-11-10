

using namespace std;

class Node{
private:
	enum DevStatus
	{
		READY,
		BUSY,
		INACTIVE
	};
	
	map<DeviceDescriptor, vector<DeviceDescriptor>> local2remote;
	map<DeviceDescriptor, DevStatus> remoteStatus;

public:
	vector<DeviceDescriptor> localDevs;
	Node();

	void findLocalDevs();
	void scanForDevs();

	void requestTorrentList(const DeviceDescriptor& client, const DeviceDescriptor& server, Message& rsp);
	void requestTorrentFile(const DeviceDescriptor& client, const DeviceDescriptor& server, 
		const string& torrentName, Message& rsp);
	void requestChunk(const DeviceDescriptor& client, const DeviceDescriptor& server, 
		const string& torrentName, const int chunkNum, Message& rsp);

	static void processRequest(const Message& req, Message& rsp);
	static void processRequest(const vector<char>& req, vector<char>& rsp);
	void createRequest();

	thread createServerThread(DeviceDescriptor servDev);

	static void server(DeviceDescriptor dev);
	
	static const string applicationDir;
	string getApplicationPath();

private:
	void sendRequestWait4Response(RRPacket& req, Message& rsp, 
		const DeviceDescriptor& clientDes, const DeviceDescriptor& serverDes);

};

