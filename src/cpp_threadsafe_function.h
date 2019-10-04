#ifndef RWELL_CPP_THREADSAFE_FUNCTIONS_GUARD
#define RWELL_CPP_THREADSAFE_FUNCTIONS_GUARD
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "check.h"
#include "event.h"
#include "threadsafe_function.h"
#include <assert.h>
#include <iostream>
#include <memory>
#include <napi.h>
#include <node_api.h>
#include <stdio.h>
#include <string>
#include <vector>

class CallerContext;


///
/// This class provides a means of calling a c/cpp function on the main threads runloop from someother thread.
///
/// There should be a better way to do this using env->CallIntoModule() - but that requires
/// struct napi_env__ (defined in js_native_api_v8.h) be visible to addon code.
/// which is currently not the case. So rather than mess with header files I have tried a more indirect route.
///
/// The steps required to call a c/ccp function on the main thread from another thread is as follows:
/// -   create a JS function which does nothing inside a the native addon using napi_create_function
/// -   wrap this js function as a napi_thread_safe_function where the call_js is the c/cpp function we want
///     called on the main thread.
///
class CppTSFunction : public TSFunction
{
  public:
    typedef void(*CppCallback)(napi_env env, void* context);
    static CppTSFunction New(napi_env env, CallerContext* caller, napi_threadsafe_function_call_js calljs);
    static CppTSFunction New(Napi::Env env, CallerContext* caller, napi_threadsafe_function_call_js calljs);

    CppTSFunction(napi_env env, CallerContext* caller, napi_threadsafe_function_call_js calljs);
    CppTSFunction(Napi::Env env, CallerContext* caller, napi_threadsafe_function_call_js calljs);
    CppTSFunction();
    ~CppTSFunction();
};

#endif
