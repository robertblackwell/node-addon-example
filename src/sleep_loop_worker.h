#ifndef RWELL_WORKER_THREAD_GUARD
#define RWELL_WORKER_THREAD_GUARD
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "arguments.h"
#include "caller_context.h"
#include "check.h"
#include "event.h"
#include <assert.h>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <napi.h>
#include <node_api.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <vector>
#include "worker.h"

/**
* This is a simple worker thread that processes a list of timer intervals (with repeats) for a single client.
* The intervals are specified in seconds.
* When an interval expires the worker thread calls a callback function and passes CallerContext* and Event*
* where the event type is set to "interval".
* If the interval is the last repeat of the interval then the event type is set to "expired".
*
* Starting the worker:
* ===================
* An instance of the worker is created by a call to getInstance() which also starts the worker thread
* but at this point there are no intervals to be processed.
*
* There is NO provision for having a single worker thread service intervals for multiple clients
* or multiple instances of the addon
*
* Giving the worker work to do:
* =============================
* Intervals are given to the worker via the addCaller() method. This method also provides the worker
* with a callback to be invoked when an interval expires.
*
* The method addCaller should only be called once. calling a second time will cause an exception.
*
* When all intervals have expired the worker has nothing to do and cycles in a loop/sleep doing nothing.
*
* Stopping the worker:
* ====================
* Termination of the worker thread is initiated by a call to the stop() method. This sets a flag that
* the worker will trigger the worker to break out of its idle/work loop. This is the start of termination
* of the worker thread.
*
* During the termination of the thread a "stopped" event will be send to the event handler/callback.
* This is the only notification that the worker is stopping and hence on receipt of a "stopped"
* event the event handler should do any cleanup required after termination of the worker thread
* and should probably 'join()' the worker thread to ensure that it has terminated.
*
* This is an un-tidiness - as the use of the interval callback and the event mechanism will not easily
* generalise as a termination cleanup strategy for a multi-client worker thread.
*
* @TODO - re-implement stop() process to have a separate 'stopped' callback provided
* to the worker thread either at thread start up (via the getInstance() call??) or the stop() call.
* I like the 'stop()' call as it has better semantics - when stop() returns to JS land we know the worker
* has stopped. This also generalises well. The stop() call the removes the last client also
* kills the thread and does join() to wait for the termination to complete.
*
*/
class SleepLoopWorker : public Worker
{
    
  public:

    static UInt32 s_ticks_ms;
    static UInt32 s_start_delay_ms;
    static UInt32 s_stop_delay_ms;

    static SleepLoopWorker* getInstance();
    static void releaseInstance(SleepLoopWorker* instance);
    SleepLoopWorker();
    ~SleepLoopWorker();
    void addCaller(CallerContext* caller, StartFunctionArg0 Arg0, EventHandler eventHandler);
    void join();
    void stopAndWait();
    // this is also stopAndWait
    void stop();
    //	void cancel();
    //	bool stopRequested();

  private:
    int findCaller(CallerContext* caller);
    void removeCaller(CallerContext* caller);
    void internalRunloop();
    bool oneloop();
    void waitForActive();
    void waitForNotActive();
    
    std::mutex _lock;
    std::condition_variable _running_cv;
    std::thread _thread;
    std::vector<Job> _jobs;
    bool _stopFlag;
    /// the properties below are only set after the runloop has completed initialization
    /// the default constructor waits for these to be set
    bool _isActive;
    std::string _dummy_var;
};

#endif
