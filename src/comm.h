

using namespace std;

class connection {
	public:
		// class that defines a device
	 	class device {
				private:
					int id = -1;
					string addr;
					string name;

				public:
					device ();
					device (string addr, string name); // this will be used for local devices
					device (int id, string addr, string name); // this will be used for remote devices
		};


	private:
		int devID = -1;
		int sock = -1;

 	public:
 		connection ();
 		static vector<device> findLocalDevices();


 	
};

