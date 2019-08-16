

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

		void connect2Node(Peer::Device& dev);
		// TODO: create a class for request and response
		void sendReqWait4Resp(const Peer::Device& dev, const string req, string resp);

		void initServer(Peer::Device& dev);



};

