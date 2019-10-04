#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "logger.h"
#include "cf_worker.h"

CFWorker* CFWorker::getInstance()
{
    static CFWorker* theInstance = new CFWorker();
    return theInstance;
}

void CFWorker::set_up_timer()
{
    /// at this point we should be on the new thread
    m_cf_runloop = CFRunLoopGetCurrent();
    CFTimeInterval interval = 1.0;
    CFTimeInterval timer_start_delay = 0.0;
    CFAbsoluteTime FireTime = CFAbsoluteTimeGetCurrent() + timer_start_delay;
    CFRunLoopTimerRef timer = CFRunLoopTimerCreateWithHandler(kCFAllocatorDefault, FireTime, interval, 0, 0, ^(CFRunLoopTimerRef timer) {
        // handle_timer() is virtual gets overridden by derived classes
        handle_timer(timer);
    }); // end of timer handler
    CFRunLoopAddTimer(m_cf_runloop, timer, kCFRunLoopDefaultMode);
}

CFWorker::CFWorker() : m_jobs(std::vector<Job>())
{
    m_is_active = false;
    m_stop_flag = false;
    m_dummy_var = "not active";
    TRACE("before starting thread dummy_var " << m_dummy_var);
//    m_thread = std::thread(&WorkerThread::internalRunloop, this);
//    int status = pthread_create(&m_thread, NULL, thread_func2, (void*)this);
    m_thread = std::thread([&](){
        m_cf_runloop = CFRunLoopGetCurrent();
        set_up_timer();
        post([&](){
            TRACE("post after create timer before m_running_cv.notify thread: " << pthread_self());
            {
                std::lock_guard<std::mutex> lck(m_lock);
                m_is_active = true;
                m_dummy_var = "now active";
            }
            m_running_cv.notify_all();
            TRACE("post after create timer after m_running_cn.notify thread: " << pthread_self());
        });
        TRACE("before CFRunLoopRun" << pthread_self());
        CFRunLoopRun();
        send_stop_to_all();
        TRACE("after CFRunLoopRun" << pthread_self());
        {
            std::lock_guard<std::mutex> lck(m_lock);
            m_is_active = false;
            m_dummy_var = "stopping";
            m_cf_runloop = nullptr;
        }
        m_running_cv.notify_all();

    });
    /// back in the 'main' thread
    TRACE("after starting thread but BEFORE waiting on dummy_var " );
    /// wait for the timer to be definitely started
    {
        std::unique_lock<std::mutex> unique_lck(m_lock);
        m_running_cv.wait(unique_lck, [&]() -> bool{
            return m_is_active;
        });
    }
    TRACE("after starting thread but AFTER waiting on dummy_var " );
}
CFWorker::~CFWorker()
{

}
void CFWorker::handle_timer(CFRunLoopTimerRef timer)
{
    TRACE(timer);
    oneloop();
}
void CFWorker::post(std::function<void()> lambda)
{
    CFRunLoopPerformBlock(m_cf_runloop, kCFRunLoopDefaultMode,^() {
        lambda();
    });
}
void CFWorker::post(void(^block)())
{
    CFRunLoopPerformBlock(m_cf_runloop, kCFRunLoopDefaultMode,^() {
        block();
    });
}
void CFWorker::addCaller(CallerContext* caller, StartFunctionArg0 Arg0, EventHandler eventHandler)
{
    post([this, caller, Arg0, eventHandler](){
        TRACE(" caller:" << caller );
        Job job(caller, Arg0, eventHandler);
        m_jobs.push_back(job);
    });
}
void CFWorker::join()
{
    m_thread.join();
}
UInt32 CFWorker::find_caller(CallerContext* caller)
{
    for(UInt32 j = 0; j < m_jobs.size(); j++) {
        if (m_jobs[j].caller == caller) {
            return j;
        }
    }
    return -1;
}
UInt32 CFWorker::remove_caller(CallerContext* caller)
{
    UInt32 index = find_caller(caller);
    if (index < 0)
        return index;
    m_jobs.erase(m_jobs.begin() + index);
    return index;;
}
void CFWorker::cancel(CallerContext* caller, EventHandler eventHandler)
{
    TRACE("cancel entry m_dummy_var: " << m_dummy_var << " thread: " << pthread_self());
    // check the run loop has not stopped or is not stopping, needs lock protection
    // as this is called from the node thread.
    {
        std::lock_guard<std::mutex> lck(m_lock);
        if ((m_cf_runloop == nullptr) || (!m_is_active))
            return;
    }
    // post a block to the runloop that will remove a caller from the m_jobs list
    post([this, caller, eventHandler](){
        UInt32 index = remove_caller(caller);
        TRACE("cancel post - remove_caller index : " << index);
        if (index >= 0) {
            // the caller was found - remove it
            Job found_job = m_jobs[index];
            std::string label = found_job.label;
            // if the removed job was the last send "cancelled_last" otherwise "cancelled"
            std::string event_name = (m_jobs.size() == 0) ? "cancelled_last" : "cancelled";
            Event* ev = Event::New(event_name, label, 0);
            eventHandler(caller, ev);
        } else {
            // caller was not found - report error via event
            std::string event_name = (m_jobs.size() == 0) ? "empty" : "not_found";
            Event* ev = Event::New(event_name, "NONE", 0);
            eventHandler(caller, ev);
        }
    });
}

void CFWorker::stop()
{
    TRACE("stop before CFRunLoopStop  m_dummy_var: " << m_dummy_var << " thread: " << pthread_self());
    // check the run loop has not stopped or is not stopping, needs lock protection
    // as this is called from the node thread.
    {
        std::lock_guard<std::mutex> lck(m_lock);
        if ((m_cf_runloop == nullptr) || (!m_is_active))
            return;
    }
    // post a block to the work threads runloop to stop the run loop
    post([this](){
        m_stop_flag = true;
        CFRunLoopStop(m_cf_runloop);
    });
    // wait for the run loop to stop
    TRACE("stop after CFRunLoopStop  before wait m_dummy_var: " << m_dummy_var << " thread: " << pthread_self());
    {
        std::unique_lock<std::mutex> unique_lck(m_lock);
        m_running_cv.wait(unique_lck, [&]() -> bool{
            return ! m_is_active;
        });
    }
    // now wait for the worker thread to terminate
    TRACE("stop after wait before join m_dummy_var: " << m_dummy_var << " thread: " << pthread_self());
    m_thread.join();
    TRACE("stop after join m_dummy_var: " << m_dummy_var << " thread: " << pthread_self());
}
// send a 'stopped' event to all callers being serviced when a 'stop' is executed
void CFWorker::send_stop_to_all()
{
    TRACE("");
    for (UInt32 j = 0; j < m_jobs.size(); j++) {
        TRACE("stopping " << j);
        
        Event* ev = Event::New("stopped", m_jobs[j].label, 0);
        m_jobs[j].eventHandler(m_jobs[j].caller, ev);
    }
}
bool CFWorker::oneloop()
{
    // holds details of interval counters that have expired
    struct TriggeredItem {
        UInt32 job_index;
        UInt32 interval_index;
        TriggeredItem(UInt32 aJobIndex, UInt32 anIntervalIndex) {
            job_index = aJobIndex;
            interval_index = anIntervalIndex;
        }
    };
    // a list of all interval counters that expired this invocation of oneloop
    std::vector<TriggeredItem> triggered_items;
    if (m_stop_flag) {
        return false;
    } else {
        if (m_jobs.size() == 0)
            return m_stop_flag;
        // Decrement counter for each active caller and remember those that expired
        // Decrement the repeat count for each expired caller and if the
        // repeat count is NOT exhausted refresh the interval counter
        for (unsigned long j = 0; j < m_jobs.size(); j++) {
            for (unsigned long i = 0; i < m_jobs[0].items.size(); i++) {
                
                if (m_jobs[j].items[i].interval_counter > 0) {
                    m_jobs[j].items[i].interval_counter--;
                    if (m_jobs[j].items[i].interval_counter <= 0) {
                        // record the expired item
                        TriggeredItem triggered_item(j, i);
                        // now see if we should reset the counter
                        triggered_items.push_back(triggered_item);
                        if (m_jobs[j].items[i].repeats_counter > 0) {
                            m_jobs[j].items[i].interval_counter = m_jobs[j].items[i].interval;
                            m_jobs[j].items[i].repeats_counter--;
                        } else {
                            // this item is done
                            m_jobs[j].items[i].interval_counter = -1;
                            m_jobs[j].items[i].repeats_counter = -1;
                        }
                    }
                }
            }
        }
        // Fire an event for each expired caller and interval
        // This is poor as it ties up the run loop for an extended period
        // Should package the generation of each event into a separate run of the run loop
        // so other things get a chance to interleave with this code
        for (unsigned long i = 0; i < triggered_items.size(); i++) {
            TriggeredItem triggered_item = triggered_items[i];
            UInt32 triggered_job_index = triggered_item.job_index;
            UInt32 triggered_interval_index = triggered_item.interval_index;
            CallerContext* an_active_caller = m_jobs[triggered_job_index].caller;
            EventHandler evh = m_jobs[triggered_job_index].eventHandler;
            
            auto triggered_interval = m_jobs[triggered_job_index].items[triggered_job_index];
            auto ident = m_jobs[triggered_job_index].items[triggered_interval_index].ident;
            auto repeats_counter = m_jobs[triggered_job_index].items[triggered_interval_index].repeats_counter;
            auto label = m_jobs[triggered_job_index].label;
            Event* ev = Event::New((repeats_counter >= 0) ? "interval" : "expired", label, ident);
            evh(an_active_caller, ev);
        }
    } // else
    return !m_stop_flag;
}
