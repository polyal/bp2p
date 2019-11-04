

using namespace std;

class Peer{
	private:
		inline static const string commString = "bp2p";
		inline static const string commSeparator = "||";

		vector<Device> nodes;
		vector<Device> localDevices;

	public:
		Peer();

		void processRequest(const vector<char>& req, vector<char>& resp);
		void createRequest();
		
		static const string applicationDir;
		string getApplicationPath();
};

