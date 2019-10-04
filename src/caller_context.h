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
 * by the start() function. Contains per caller info and points
 * at the active instance of WorkerThread that is servicing the caller
 *
 */
class CallerContext
{
  public:
    static void finalize(Napi::Env env, CallerContext* finalizeData);
    Worker* worker_ptr;
    napi_async_context async_context;
    napi_value unwrappedJsCallback;
    napi_env env;
    napi_threadsafe_function tsf_cb;

    Napi::Env napi_env;
    TSFunction tsfunction;
    CppTSFunction cpptsfunction_dummy;

    StartFunctionArg0 startArg0;
    CallerContext(Napi::Env env, StartFunctionArg0 arg0);
    ~CallerContext();
};
typedef void(*CallerContextFinalise(Napi::Env env, CallerContext* caller));

#endif
