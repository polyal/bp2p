using namespace std;

class Utils{
	public:
		static const string applicationDir;
		static void tokenize(const string& text, const string& sep, vector<string>& tokens);
		static string getHomeDir();
};