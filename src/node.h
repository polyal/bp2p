

using namespace std;

class Peer{
	private:
		class Device {
		private:
			int devID = -1;
			int sock = -1;
			int id = -1;
			string addr;
			string name;

		public:
			Device ();
			Device (string addr, string name); // this will be used for local devices
			Device (int id, string addr, string name); // this will be used for remote devices

			string getAddr();
			string getName();
			int getSock();

		};

	private:
		vector<Device> nodes;
		vector<Device> localDevices;

	public:
		Peer();

		void findNearbyDevices();
		void findLocalDevices();

		void connectToClient(Peer::Device dev);

};

