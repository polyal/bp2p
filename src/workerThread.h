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

	template <typename Work>
	WorkerThread(Work w)
	{
		this->status = make_shared<atomic<WorkerThread::Status>>(WorkerThread::Status::ACTIVE);
		this->event = make_shared<SyncEvent>();
		this->t = make_unique<thread>(
			[this, w]
			{
				workerLoop(w);		
			});
	}

	WorkerThread(unique_ptr<thread> t, shared_ptr<atomic<Status>> status);
	WorkerThread(unique_ptr<thread> t, shared_ptr<atomic<Status>> status, shared_ptr<SyncEvent> event);

	template <typename Work>
	void workerLoop(Work w)
	{
		do{
			unique_lock<std::mutex> lock(this->event->m);
			event->cv.wait(lock, 
				[this]
				{
					return (*this->status != PAUSE) || *this->status == KILL;
				});

			w();

			lock.unlock();
			this_thread::sleep_for (std::chrono::milliseconds(20));
		} while(*this->status != KILL);
	}

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