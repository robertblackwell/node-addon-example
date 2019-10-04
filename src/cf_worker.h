
#ifndef RWELL_CFWORKER_GUARD_H
#define RWELL_CFWORKER_GUARD_H

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <thread>
#include <mutex>

#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "worker.h"

/**
* This class is a std::thread with a CFRunLoop that runs a timer loop.
* The class is intended to be a base class - to get the timer handler to do some
* useful work derive a class that overrides the timer_handler method.
*/

class CFWorker : public Worker
{
public:
    CFWorker();
    ~CFWorker();

    static CFWorker* getInstance();
    static void open(CallerContext* caller, EventHandler eventHandler);
    void addCaller(CallerContext* caller, StartFunctionArg0 Arg0, EventHandler eventHandler);
    void cancel(CallerContext* caller, EventHandler eventHandler);
    void stop();
    
    /**
    * Post a C++ lambda for execution on the threads CFRunLoop
    */
    void post(std::function<void()> lambda);
    /**
    * Post an 'objc' or 'apples 'C' extension' BLOCK for execution on the threads CFRunLoop
    */
    void post(void(^block)());
    void set_up_timer();
    void handle_timer(CFRunLoopTimerRef timer);
    void join();
    bool oneloop();
    void send_stop_to_all();

    CFRunLoopRef        m_cf_runloop;
protected:
    UInt32 find_caller(CallerContext* caller);
    UInt32 remove_caller(CallerContext* caller);
    bool                m_is_active;
    bool                m_stop_flag;
    std::thread         m_thread;
    std::mutex          m_lock;
    std::condition_variable  m_running_cv;
    std::string         m_dummy_var;
    std::vector<Job>    m_jobs;

};

#endif /* start_interval_timer_hpp */
