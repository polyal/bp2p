#include "workerThread.h"


WorkerThread::WorkerThread(unique_ptr<thread> t, shared_ptr<atomic<Status>> status) 
{
	this->t = move(t);
	this->status = status;
}

WorkerThread::WorkerThread(unique_ptr<thread> t, shared_ptr<atomic<Status>> status, shared_ptr<SyncEvent> event) 
{
	this->t = move(t);
	this->status = status;
	this->event = event;
}

void WorkerThread::activate()
{
	if (this->event){
		std::unique_lock<std::mutex> lock(this->event->m);
		setStatus(ACTIVE);
		lock.unlock();
		this->event->cv.notify_one();
	}
	else
	setStatus(ACTIVE);
}

void WorkerThread::pause()
{
	if (this->event){
		std::unique_lock<std::mutex> lock(this->event->m);
		setStatus(PAUSE);
		lock.unlock();
		this->event->cv.notify_one();
	}
	else
		setStatus(PAUSE);
}

void WorkerThread::kill()
{
	if (this->event){
		std::unique_lock<std::mutex> lock(this->event->m);
		setStatus(KILL);
		lock.unlock();
		this->event->cv.notify_one();
	}
	else
		setStatus(KILL);
}

void WorkerThread::close()
{
	kill();
	if (this->t) this->t->join();
}

void WorkerThread::setStatus(Status status)
{
	*this->status = status;
}
