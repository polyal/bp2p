

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

		};

	private:
		vector<Device> clientNodes;
		vector<Device> serverNodes;
		vector<Device> localDevices;

	public:
		Peer();

		void initializeLocalDevices();
		void initializeServerNodes();
};

