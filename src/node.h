

using namespace std;

class Peer{
	private:
		class Device {
		private:
			int channel = -1;
			int sendSock = -1;
			int recSock = -1;
			int id = -1;
			string addr;
			string name;
			vector<string> torrentNames;

		public:
			Device ();
			Device (string addr, string name); // this will be used for local devices
			Device (int id, string addr, string name); // this will be used for remote devices

			string getAddr() const;
			string getName() const;
			int getChannel() const;
			int getSendSock() const;
			int getRecSock() const;

			void setSendSock(int sock);
			void setRecSock(int sock);

			void addTorrentNames(vector<string> torrentNames);



		};

	private:
		static const string commString;
		static const string commSeparator;

		vector<Device> nodes;
		vector<Device> localDevices;

		typedef enum _requestType {
			badReq,
			torrentFile, 
			chunk,
			torrentList 
		} requestType;

		Peer::requestType getReqTypeFromReq(const string& resp);

	public:
		Peer();

		void findNearbyDevices();
		void findLocalDevices();

		int connect2Node(Peer::Device& dev);
		int sendReqWait4Resp(const Peer::Device& dev, const string req, string& resp);
		int initServer(Peer::Device& dev);
		int listen4Req(Peer::Device& dev, Peer::Device& client);
		int fetchRequestData(const Peer::Device& dev, string& req);
		int sendResponse(Peer::Device& dev, string data);
		void endComm(Peer::Device& dev);

		void parseTorrentList(const string& resp, vector<string>& torrentList);
		int requestTorrentList(Peer::Device& dev);

		// request processing
		bool processRequest(const string& req, string& resp);
		int processTorrentListReq();
		void getTorrentList(vector<string>& torrentNames);

		static const string applicationDir;
		string getApplicationPath();


		// testing functions
		void Client();
		void Server();

};

