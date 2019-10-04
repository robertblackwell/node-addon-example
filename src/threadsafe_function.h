#ifndef RWELL_THREADSAFE_FUNCTIONS_GUARD
#define RWELL_THREADSAFE_FUNCTIONS_GUARD
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "check.h"
#include "event.h"
#include <assert.h>
#include <iostream>
#include <memory>
#include <napi.h>
#include <node_api.h>
#include <stdio.h>
#include <string>
#include <vector>

class CallerContext;
//#include "caller_context.h"

///
/// utility functions to wrap a jscallback
///

napi_threadsafe_function WrapCallback(napi_env env, napi_value arg1, CallerContext* caller, napi_threadsafe_function_call_js calljs);
napi_threadsafe_function WrapCallback(Napi::Function f, CallerContext* caller, napi_threadsafe_function_call_js calljs);
///
/// Class representing a thread safe function - I did not really understand how
/// to use Napi::ThreadSafeFunction so I rolled my own with out all the template
/// and overload stuff.
class TSFunction
{
  public:
    static TSFunction New(napi_env env, napi_value f, CallerContext* caller, napi_threadsafe_function_call_js calljs);
    static TSFunction New(Napi::Function f, CallerContext* caller, napi_threadsafe_function_call_js calljs);
    static TSFunction New(Napi::Function f, CallerContext* caller);
    static void CallJs(napi_env env, napi_value callback, void* context, void* data);

    TSFunction(napi_env env, napi_value f, CallerContext* caller, napi_threadsafe_function_call_js calljs);
    TSFunction(Napi::Function f, CallerContext* caller, napi_threadsafe_function_call_js calljs);
    TSFunction(Napi::Function f, CallerContext* caller);
    TSFunction();
    ~TSFunction();
    void callBlocking(Event* event);
    void ref();
    void unref();
    void abort();
    bool isEmpty();
    void acquire();
    void release();

  protected:
    napi_env _env;
    napi_threadsafe_function _tsf;
};

#endif
