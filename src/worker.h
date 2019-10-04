#ifndef RWELL_WORKER_GUARD
#define RWELL_WORKER_GUARD
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

/**
* Common interface types for all implementations of the worker thread
*/

typedef void (*EventHandlerFunction)(CallerContext* caller, Event* event);
typedef std::function<void(CallerContext* caller, Event* event)> EventHandler;

/// a scheudle item
struct Item
{
    int ident;
    int interval;
    int repeats;
    int interval_counter;
    int repeats_counter;
};
/// a worktherThread Job - the schedule for a single CallerContext
struct Job
{
    CallerContext* caller;
    EventHandler eventHandler;
    std::string label;
    std::vector<Item> items;
    int interval_counter;
    int current_repeat;
    Job()
      : caller(nullptr)
      , items(std::vector<Item>())
      , interval_counter(0)
      , current_repeat(0)
    {}
    Job(CallerContext* aCaller, StartFunctionArg0 anArg0, EventHandler anEventHandler)
      : caller(aCaller)
      , eventHandler(anEventHandler)
    {
        label = anArg0._label;
        for (unsigned i = 0; i < anArg0._schedule.size(); i++) {
            Item item;
            item.ident = anArg0._schedule[i].ident;
            item.interval = anArg0._schedule[i].interval;
            item.repeats = anArg0._schedule[i].repeats;
            item.interval_counter = anArg0._schedule[i].interval;
            item.repeats_counter = anArg0._schedule[i].repeats;
            items.push_back(item);
        }
    }
};

template <class T>
void configure(UInt32 ticks_ms, UInt32 start_delay_ms, UInt32 stop_delay_ms)
{
    T::s_ticks_ms = ticks_ms;
    T::s_start_delay_ms = start_delay_ms;
    T::s_stop_delay_ms = stop_delay_ms;
}

template<class T>
T* getInstance()
{
    return new T();
}


/// abstract class
class Worker
{
  public:
    virtual ~Worker() = 0;
    static UInt32 s_ticks_ms;
    static UInt32 s_start_delay_ms;
    static UInt32 s_stop_delay_ms;
    
    virtual void addCaller(CallerContext* caller, StartFunctionArg0 Arg0, EventHandler eventHandler) = 0;
    virtual void join() = 0;
    
    virtual void stop() = 0;

  private:

};


#endif
