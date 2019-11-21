#include <atomic>
#include <thread>
#include <memory>
#include "eventSync.h"

using namespace std;

class WorkerThread
{
public:
	enum Status
	{
		ACTIVE,
		PAUSE,
		KILL
	};

	template <class Fn, class... Args> WorkerThread(Fn&& fn, Args&&... args);
	WorkerThread(unique_ptr<thread> t, shared_ptr<atomic<Status>> status);
	WorkerThread(unique_ptr<thread> t, shared_ptr<atomic<Status>> status, shared_ptr<SyncEvent> event);

	void activate();
	void pause();
	void kill();
	void close();

protected:
	unique_ptr<thread> t = nullptr;
	shared_ptr<atomic<Status>> status = nullptr;
	shared_ptr<SyncEvent> event = nullptr;

	void setStatus(Status status);
}
;