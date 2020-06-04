#include "cpp_threadsafe_function.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "caller_context.h"
#include "logger.h"
#include "threadsafe_function.h"
#include "worker.h"
#include <assert.h>
#include <iostream>
#include <memory>
#include <napi.h>
#include <node_api.h>
#include <stdio.h>
#include <string>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static napi_value
dummy_js_func(napi_env env, napi_callback_info info)
{
    TRACE("");
    return nullptr;
}
#pragma GCC diagnostic pop

static void
finalizer(napi_env env, void* finalize_data, void* hint)
{
    TRACE("finalize_data : " << std::hex << finalize_data << " hint: " << std::hex << hint);
}

CppTSFunction::CppTSFunction()
  : TSFunction()
{
}
CppTSFunction::~CppTSFunction()
{
    TRACE("");
}
CppTSFunction::CppTSFunction(Napi::Env env, CallerContext* caller, napi_threadsafe_function_call_js calljs)
  : CppTSFunction(napi_env(env), caller, calljs)
{
}

CppTSFunction::CppTSFunction(napi_env env, CallerContext* caller, napi_threadsafe_function_call_js calljs)
{
    _env = env;
    napi_value asyncResource, asyncName;
    napi_async_context asyncContext;
    napi_threadsafe_function callback = NULL;
    CHECK(napi_create_object(env, &asyncResource) == napi_ok);
    CHECK(napi_create_string_utf8(env, "cpp_threadsafe_function_dummy", NAPI_AUTO_LENGTH, &asyncName) == napi_ok);
    /** wrap the js callback function for calling */
    CHECK(napi_async_init(env, asyncResource, asyncName, &(asyncContext)) == napi_ok);
    //    CHECK(napi_create_threadsafe_function(env, arg1, asyncResource,
    //    asyncName, 0, 2, NULL, NULL, NULL, worker_dispatch_events, &callback)
    //    == napi_ok); CHECK(napi_create_threadsafe_function(env, arg1,
    //    asyncResource, asyncName, 0, 2, NULL, NULL, NULL, Test::calljs,
    //    &callback) == napi_ok);
    std::string dummy_name("dummy");
    napi_value func;
    /// This only works because there is no capture by the following closure
    auto dummy_js_func_as_closure = [](napi_env env, napi_callback_info info) -> napi_value {
        TRACE("");
        return nullptr;
    };
    napi_status stt = napi_create_function(env, dummy_name.c_str(), dummy_name.size(), dummy_js_func_as_closure/*dummy_js_func*/, nullptr, &func);
    CHECK(stt == napi_ok);
    napi_status status = napi_create_threadsafe_function(env,
                                                         func,
                                                         asyncResource,
                                                         asyncName,
                                                         0,
                                                         2,
                                                         (void*)caller, // NULL, //thread_fiinalize data void*
                                                         finalizer,     // NULL, // thread finalizer function napi_finalize =
                                                                        // void(napi_env, env, void* finlaize_data, void*
                                                                        // finalize_hint)
                                                         (void*)caller, // NULL, // void* context
                                                         calljs,
                                                         &callback);
    if (status != napi_ok) {
        TRACE("");
    }
    CHECK(status == napi_ok);
    _tsf = callback;
    /** next call ensures the runloop on main thread should not exit until this
     * function ceases to exist */
    CHECK(napi_ref_threadsafe_function(env, callback) == napi_ok);
}

CppTSFunction
CppTSFunction::New(Napi::Env env, CallerContext* caller, napi_threadsafe_function_call_js calljs)
{
    CppTSFunction tsf(env, caller, calljs);
    return tsf;
}
CppTSFunction
CppTSFunction::New(napi_env env, CallerContext* caller, napi_threadsafe_function_call_js calljs)
{
    CppTSFunction tsf(env, caller, calljs);
    return tsf;
}
