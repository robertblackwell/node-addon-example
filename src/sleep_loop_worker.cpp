#include <assert.h>
#include <iostream>
#include <memory>
#include <napi.h>
#include <node_api.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "napi_ext.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "caller_context.h"
#include "check.h"
#include "logger.h"
#include "worker.h"
#include "sleep_loop_worker.h"

UInt32 Worker::s_ticks_ms = 1000;
UInt32 Worker::s_start_delay_ms = 10000;
UInt32 Worker::s_stop_delay_ms = 12000;


class WorkerException : public std::exception
{
  public:
    WorkerException(std::string msg)
      : std::exception()
      , _msg(msg)
    {}
    const char* what() const throw() { return std::string("WorkerException:" + _msg).c_str(); }
    std::string _msg;
};

SleepLoopWorker*
SleepLoopWorker::getInstance()
{
    SleepLoopWorker* w = new SleepLoopWorker();
    return w;
}
void
SleepLoopWorker::releaseInstance(SleepLoopWorker* instance)
{
    delete instance;
}
SleepLoopWorker::SleepLoopWorker() : _dummy_var(std::string("not_initialized"))
{
    _stopFlag = false;
    _isActive = false;
    TRACE("before starting thread dummy_var " << _dummy_var);
    _thread = std::thread(&SleepLoopWorker::internalRunloop, this);
    TRACE("after starting thread but before waiting dummy_var " << _dummy_var);
    std::unique_lock<std::mutex> unique_lck(_lock);
    _running_cv.wait(unique_lck, [&]() -> bool{
        return _isActive;
    });
    TRACE("after starting thread but after waiting dummy_var " << _dummy_var);
}
SleepLoopWorker::~SleepLoopWorker()
{
    TRACE("dissapear" << std::hex << this);
}
void
SleepLoopWorker::addCaller(CallerContext* caller, StartFunctionArg0 Arg0, EventHandler eventHandler)
{
    std::lock_guard<std::mutex> guard(_lock);
    if (_jobs.size() > 0) {
        throw Napi::Error::New(caller->env, "SleepLoopWorker can only have 1 Job at a time");
    }
    if (!_isActive) {
        throw Napi::Error::New(caller->env, "SleepLoopWorker is not active");
    }
    Job job(caller, Arg0, eventHandler);
    _jobs.push_back(job);
}
int
SleepLoopWorker::findCaller(CallerContext* caller)
{
    return -1;
}
void
SleepLoopWorker::internalRunloop()
{
    TRACE("entered - but before slow init dummy var : " << _dummy_var);
//    sleep(5); // simulate a delayed or long initialization
    /// signal that the runloop is active
    {
        std::lock_guard<std::mutex> lck(_lock);
        _isActive = true;
        _dummy_var = "now active";
    }
    _running_cv.notify_all();
    TRACE("after notify dummy_var " << _dummy_var);

    while (1) {
        {
            /// NOTE:
            /// this puts the std::lock_guard<> in a block so that it
            /// automatically unlocks on block exit. Test to see if we need to
            /// stop before we sleep
            std::lock_guard<std::mutex> guard(_lock);
            if (_stopFlag) {
                break;
            }
        }
        /// sleep outside the locked section
        TRACE("SleepLoopWorker::runloop sleeping");
        sleep(1);
        {
            /// NOTE:
            /// this puts the std::lock_guard<> in a block so that it
            /// automatically unlocks on block exit. Before start doing some
            /// potentially time consuming work
            std::lock_guard<std::mutex> guard(_lock);
            if (_stopFlag) {
                break;
            }
            oneloop();
        }
    }
    TRACE("SleepLoopWorker::runloop ending");
    ///
    /// at this point we know the thread is going to terminate.
    /// Maybe release the tsfunction ?
    ///
    CallerContext* an_active_caller = _jobs[0].caller;
    EventHandler evh = _jobs[0].eventHandler;
    Event* ev = Event::New("stopped", _jobs[0].label, 0);
    evh(an_active_caller, ev);
    
    TRACE("worker thread stopping");
    {
        std::lock_guard<std::mutex> lck(_lock);
        _isActive = false;
        _dummy_var = "stopping";
    }
    _running_cv.notify_all();
    TRACE("worker thread stopped");

}
void
SleepLoopWorker::join()
{
    //_thread.join();
}
///
/// Perform one iteration of the workers process loop - it processes each
/// callers entire schedule. It locks the mutex for the duration of the
/// processing to ensure that a stop does not take effect part way through
/// processing
/// @return true unless the _stopFlag is set
///
bool
SleepLoopWorker::oneloop()
{
    std::vector<unsigned long> expiredIntervals;
    if (_stopFlag) {
        return false;
    } else {

        // decrement counter for each active caller and remeber those that
        // expire decrement the repeat count for each expired caller and if the
        // repat count is NOT exhausted refresh the interval counter
        for (unsigned long i = 0; i < _jobs[0].items.size(); i++) {
            if (_jobs[0].items[i].interval_counter > 0) {
                _jobs[0].items[i].interval_counter--;
                if (_jobs[0].items[i].interval_counter <= 0) {
                    expiredIntervals.push_back(i);
                    if (_jobs[0].items[i].repeats_counter > 0) {
                        _jobs[0].items[i].interval_counter = _jobs[0].items[i].interval;
                        _jobs[0].items[i].repeats_counter--;
                    } else {
                        _jobs[0].items[i].interval_counter = 0;
                    }
                }
            }
        } 
        // fire an event for each expired caller
        for (unsigned long i = 0; i < expiredIntervals.size(); i++) {
            CallerContext* an_active_caller = _jobs[0].caller;
            EventHandler evh = _jobs[0].eventHandler;
            auto ident = _jobs[0].items[i].ident;
            Event* ev = Event::New((_jobs[0].items[i].repeats_counter > 0) ? "interval" : "expired", _jobs[0].label, ident);
            evh(an_active_caller, ev);
        }
    } // else
    return !_stopFlag;
}

void
SleepLoopWorker::removeCaller(CallerContext* caller)
{}
void
SleepLoopWorker::stopAndWait()
{
    TRACE("before wait for active == false");
    std::unique_lock<std::mutex> unique_lck(_lock);
    /// tell runloop to stop
    _stopFlag = true;
    /// now wait for the runloop to act on the stop
    _running_cv.wait(unique_lck, [&]() -> bool{
        return !_isActive;
    });
    TRACE("after wait for active == false - so join the thread");
    // at this point we know the runloop is stopping
    // so wait for the thread to exit completely
    _thread.join();
    TRACE("after join");
}
void
SleepLoopWorker::stop()
{
    TRACE("before wait for active == false");
    std::unique_lock<std::mutex> unique_lck(_lock);
    /// tell runloop to stop
    _stopFlag = true;
    /// now wait for the runloop to act on the stop
    _running_cv.wait(unique_lck, [&]() -> bool{
        return !_isActive;
    });
    TRACE("after wait for active == false - so join the thread");
    // at this point we know the runloop is stopping
    // so wait for the thread to exit completely
    _thread.join();
    TRACE("after join");
}

void
SleepLoopWorker::waitForActive()
{
    TRACE("before wait_for_active dummy_var " << _dummy_var);
    std::unique_lock<std::mutex> unique_lck(_lock);
    _running_cv.wait(unique_lck, [&]() -> bool{
        return _isActive;
    });
    TRACE("after wait_for_active dummy_var " << _dummy_var);
}
void
SleepLoopWorker::waitForNotActive()
{
    TRACE("before wait for not active dummy_var " << _dummy_var);
    std::unique_lock<std::mutex> unique_lck(_lock);
    ///  wait for the runloop to act on the stop
    _running_cv.wait(unique_lck, [&]() -> bool{
        return !_isActive;
    });
    TRACE("after wait for not active dummy_var " << _dummy_var);

}
