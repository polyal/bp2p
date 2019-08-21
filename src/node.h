

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



		};

	private:
		vector<Device> nodes;
		vector<Device> localDevices;

	public:
		Peer();

		void findNearbyDevices();
		void findLocalDevices();

		int connect2Node(Peer::Device& dev);
		// TODO: create a class for request and response
		int sendReqWait4Resp(const Peer::Device& dev, const string req, string& resp);
		int initServer(Peer::Device& dev);
		int listen4Req(Peer::Device& dev, Peer::Device& client, string& req);
		int sendResponse(Peer::Device& dev, string data);
		void endComm(Peer::Device& dev);


		// testing functions
		void Client();
		void Server();


};

