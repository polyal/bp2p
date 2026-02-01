#include <condition_variable>
#include <mutex>

using namespace std;

struct SyncEvent
{
	recursive_mutex m;
	condition_variable_any cv;	
};
