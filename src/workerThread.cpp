#include "node.h"

using namespace std;

template <class Fn, class... Args>
Node::WorkerThread::WorkerThread(Fn&& fn, Args&&... args) 
{
	this->status = make_shared<atomic<WorkerThread::Status>>(WorkerThread::Status::ACTIVE);
	this->event = make_shared<SyncEvent>();
	this->t = make_unique<thread>(fn, args...);
}

Node::WorkerThread::WorkerThread(unique_ptr<thread> t, shared_ptr<atomic<Status>> status) 
{
	this->t = move(t);
	this->status = status;
}

Node::WorkerThread::WorkerThread(unique_ptr<thread> t, shared_ptr<atomic<Status>> status, shared_ptr<SyncEvent> event) 
{
	this->t = move(t);
	this->status = status;
	this->event = event;
}

void Node::WorkerThread::activate()
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

void Node::WorkerThread::pause()
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

void Node::WorkerThread::kill()
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

void Node::WorkerThread::close()
{
	kill();
	if (this->t) this->t->join();
}

void Node::WorkerThread::setStatus(Status status)
{
	*this->status = status;
}
