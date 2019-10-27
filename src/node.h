

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
		inline static const string commString = "bp2p";
		inline static const string commSeparator = "||";

		vector<Device> nodes;
		vector<Device> localDevices;

	public:
		Peer();

		void findNearbyDevices();
		void findLocalDevices();

		void processRequest(const vector<char>& req, vector<char>& resp);
		void createRequest();
		
		static const string applicationDir;
		string getApplicationPath();
};

