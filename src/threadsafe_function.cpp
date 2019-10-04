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
#include "logger.h"
#include "threadsafe_function.h"
#include "caller_context.h"
#include "worker.h"

/**
 * @TODO - some blocks of code a duplicated below (or almost) - need to fix this
 * by deleting some functions
 *
 */
static void
finalizer(napi_env env, void* finalize_data, void* hint)
{
    TRACE("finalize_data : " << std::hex << finalize_data << " hint: " << std::hex << hint);
}

napi_threadsafe_function
WrapCallback(napi_env env, napi_value arg1, CallerContext* caller, napi_threadsafe_function_call_js calljs)
{
    napi_value asyncResource, asyncName;
    napi_async_context asyncContext;
    napi_threadsafe_function callback = NULL;
    CHECK(napi_create_object(env, &asyncResource) == napi_ok);
    CHECK(napi_create_string_utf8(env, "fsevents", NAPI_AUTO_LENGTH, &asyncName) == napi_ok);
    /** wrap the js callback function for calling */
    CHECK(napi_async_init(env, asyncResource, asyncName, &(asyncContext)) == napi_ok);
    //    CHECK(napi_create_threadsafe_function(env, arg1, asyncResource,
    //    asyncName, 0, 2, NULL, NULL, NULL, worker_dispatch_events, &callback)
    //    == napi_ok); CHECK(napi_create_threadsafe_function(env, arg1,
    //    asyncResource, asyncName, 0, 2, NULL, NULL, NULL, Test::calljs,
    //    &callback) == napi_ok);
    CHECK(napi_create_threadsafe_function(env,
                                          arg1,
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
                                          &callback) == napi_ok);

    /** next call ensures the runloop on main thread should not exit until this
     * function ceases to exist */
    CHECK(napi_ref_threadsafe_function(env, callback) == napi_ok);
    return callback;
}
napi_threadsafe_function
WrapCallback(Napi::Function f, CallerContext* caller, napi_threadsafe_function_call_js calljs)
{
    napi_env env = f.Env();
    napi_value arg1 = napi_value(f);
    return WrapCallback(env, arg1, caller, calljs);
}
TSFunction::TSFunction()
  : _env(nullptr)
  , _tsf(nullptr)
{}
TSFunction::~TSFunction()
{
    TRACE("be gone " << std::hex << this);
}
TSFunction::TSFunction(Napi::Function f, CallerContext* caller)
  : TSFunction(f, caller, TSFunction::CallJs)
{}

TSFunction::TSFunction(Napi::Function f, CallerContext* caller, napi_threadsafe_function_call_js calljs)
  : TSFunction(f.Env(), napi_value(f), caller, calljs)
{}

TSFunction::TSFunction(napi_env env, napi_value arg1, CallerContext* caller, napi_threadsafe_function_call_js calljs)
{
    _env = env;
    napi_value asyncResource, asyncName;
    napi_async_context asyncContext;
    napi_threadsafe_function callback = NULL;
    CHECK(napi_create_object(env, &asyncResource) == napi_ok);
    CHECK(napi_create_string_utf8(env, "fsevents", NAPI_AUTO_LENGTH, &asyncName) == napi_ok);
    /** wrap the js callback function for calling */
    CHECK(napi_async_init(env, asyncResource, asyncName, &(asyncContext)) == napi_ok);
    //    CHECK(napi_create_threadsafe_function(env, arg1, asyncResource,
    //    asyncName, 0, 2, NULL, NULL, NULL, worker_dispatch_events, &callback)
    //    == napi_ok); CHECK(napi_create_threadsafe_function(env, arg1,
    //    asyncResource, asyncName, 0, 2, NULL, NULL, NULL, Test::calljs,
    //    &callback) == napi_ok);
    napi_status status = napi_create_threadsafe_function(env,
                                                         arg1,
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
TSFunction
TSFunction::New(Napi::Function f, CallerContext* caller, napi_threadsafe_function_call_js calljs)
{
    TSFunction tsf(f, caller, calljs);
    return tsf;
}
TSFunction
TSFunction::New(napi_env env, napi_value f, CallerContext* caller, napi_threadsafe_function_call_js calljs)
{
    TSFunction tsf(env, f, caller, calljs);
    return tsf;
}
TSFunction
TSFunction::New(Napi::Function f, CallerContext* caller)
{
    TSFunction tsf(f, caller, TSFunction::CallJs);
    return tsf;
}
void
TSFunction::CallJs(napi_env env, napi_value callback, void* context, void* data)
{
    Event* event = (Event*)data;
    TRACE("env : [" << env << "] context : [" << std::hex << context << "] data : [" << std::hex << data << "] type " << event->_type);
    CallerContext* cc = (CallerContext*)context;
    Worker* w = cc->worker_ptr;

    if ((env == nullptr) || (callback == nullptr)) {
        TRACE("env is 0 - the call back is closed - get out of here");
        return;
    }
    napi_value recv;
    napi_value args[2];
    try {
        NAPI_THROW_IF_FAILED_VOID(Napi::Env(env), napi_get_null(env, &recv));
        args[0] = event->ToNapi(env);
        napi_status status = napi_call_function(env, recv, callback, 1, args, &recv);
        if (status == napi_ok) {
            if (event->_type == "stopped") {
                cc->tsfunction.release();
                w->join();
                delete cc;
                delete w;
            }
            return;
        } else if (status == napi_pending_exception) {
            TRACE("not ok - exception is pending - can not do anything about "
                  "it - out of here");
            return;
        } else {
            NAPI_FATAL_IF_FAILED(status, "worker_thread.cpp", "invalid status");
        }
    } catch (std::exception& e) {
        TRACE("exception in CallJs");
    }
}

void
TSFunction::abort()
{
    napi_status status = napi_release_threadsafe_function(_tsf, napi_tsfn_release);
    if (status != napi_ok) {
    }
}
void
TSFunction::release()
{
    TRACE("XXXX this: " << this);
    napi_status status = napi_release_threadsafe_function(_tsf, napi_tsfn_abort);
    if (status != napi_ok) {
        TRACE("not ok");
    }
}
void
TSFunction::ref()
{
    napi_status status = napi_ref_threadsafe_function(_env, _tsf);
    if (status != napi_ok) {
        TRACE("not ok");
    }
}
void
TSFunction::unref()
{
    napi_status status = napi_unref_threadsafe_function(_env, _tsf);
    if (status != napi_ok) {
        TRACE("not ok");
    }
}
bool
TSFunction::isEmpty()
{
    return (_tsf == nullptr);
}
void
TSFunction::callBlocking(Event* event)
{
    napi_status status = napi_call_threadsafe_function(_tsf, (void*)event, napi_tsfn_blocking);
    if (status != napi_ok) {
        TRACE("napi_call_threadsafe_function BAD status");
    }
}
