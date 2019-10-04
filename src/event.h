#ifndef RWELL_EVENT_GUARD
#define RWELL_EVENT_GUARD
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "check.h"
#include <assert.h>
#include <iostream>
#include <memory>
#include <napi.h>
#include <node_api.h>
#include <stdio.h>
#include <string>
#include <vector>

/**
 * This is the class representing the event that a workerthread passes back to a
 * caller.
 */
struct Event
{
    std::string _type;
    std::string _label;
    int64_t _ident;
    int64_t _thread;
    std::string _time_stamp;
    Napi::Value ToNapi(napi_env env);
    Napi::Value ToNapi(Napi::Env env);
    static Event* New(std::string type, std::string label, int64_t ident);
};

Napi::Value pack(napi_env env, Event* event);
Napi::Value pack(Napi::Env napi_env, Event* event);

// typedef void (*worker_event_handler_t)(void *context, Event *event);
// typedef void (*fse_thread_hook_t)(void *context);

#endif
