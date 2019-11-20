#include <condition_variable>
#include <mutex>

using namespace std;

struct SyncEvent
{
	mutex m;
	condition_variable cv;	
};
