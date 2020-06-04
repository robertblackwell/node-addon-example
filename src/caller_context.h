#ifndef RWELL_CALLERCONTEXT_GUARD
#define RWELL_CALLERCONTEXT_GUARD
#include <assert.h>
#include <iostream>
#include <memory>
#include <napi.h>
#include <node_api.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "arguments.h"
#include "check.h"
#include "threadsafe_function.h"
#include "cpp_threadsafe_function.h"

class Worker;

/**
 * The C++ implementation of the object that gets returns as "instance"
 * or "opaque" by the start() function. Contains per caller info and points
 * at the active instance of Worker instance
 */
class CallerContext
{
  public:
    static void finalize(Napi::Env env, CallerContext* finalizeData);
    Worker* worker_ptr;
    napi_async_context async_context;
    napi_env env;
    Napi::Env napi_env;

    // the threadsafe callback that delivers all events except stop and cancel to js
    TSFunction tsfunction;
    // tsf callback that receives the open or start thread event
    TSFunction open_tsf;
    // tsf callbacck that receives the cancel event
    TSFunction cancel_tsf;
    // tsf callbacck that receives the stop event
    TSFunction stop_tsf;
    //
    CppTSFunction cpptsfunction_dummy;

    StartFunctionArg0 startArg0;
    CallerContext(Napi::Env env, StartFunctionArg0 arg0);
    ~CallerContext();
};
typedef void(*CallerContextFinalise(Napi::Env env, CallerContext* caller));

#endif
