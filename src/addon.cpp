#include <assert.h>
#include <sstream>
#define NAPI_VERSION 4
#include <napi.h>
#include "addon.h"
#include "logger.h"
#define THROW_AS_JAVASCRIPT_EXCEPTION(x) \
do {                                                                \
    /*TRACE( x );*/                                                     \
    std::ostringstream sout;                                        \
    sout << x;                                                      \
    std::string ss2 = sout.str();                                   \
    Napi::Error::New(napi_env, ss2).ThrowAsJavaScriptException();   \
} while(0)

#ifdef NOTDEFINED
void
cpp_callback(napi_env env, napi_value callback, void* context, void* data)
{
    Event* event = (Event*)data;
    TRACE("env : [" << env << "] context : [" << std::hex << context << "] data : [" << std::hex << data << "] type " << event->_type);
    napi_value recv;
    napi_value args[2];
    napi_status status = napi_call_function(env, recv, callback, 0, args, &recv);
    if (status == napi_ok) {
        TRACE("status" << status);
    }
}
napi_value
dummy_js_func(napi_env env, napi_callback_info info)
{
    TRACE("");
    return nullptr;
}
#endif

/**
 * TSFunction class can provide a default call_js function but the addon.cpp
 * file is providing it because this function plays an important role in the
 * stop() and cleanup of the worker process by recognizing a "stopped" event,
 * allowing that event to go back to js land and then on return from the JS
 * callback release the TSFunction and delete the CallerContext and if there is
 * a dedicated WorkerTHread for each CallerContext also delete the WorkerThread.
 */
void
call_js(napi_env env, napi_value callback, void* context, void* data)
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
#define OPTION_JOIN_IN_STOPFUNCTION
#ifdef OPTION_JOIN_IN_STOPFUNCTION
#else
            if (event->_type == "stopped") {
                cc->tsfunction.release();
                w->join();
                delete cc;
                delete w;
            }
#endif
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

/**
* configures the addon - mostly for testing purposes.
* Aslo demonstrates how to return an object to the javascript caller
*/
napi_value
configure(napi_env env, napi_callback_info info)
{
    Napi::CallbackInfo napi_info(env, info);
    Napi::Env napi_env = napi_info.Env();
    auto length = napi_info.Length();
    if (length != 3) {
        THROW_AS_JAVASCRIPT_EXCEPTION("configure expects 3 arguments");
    }
    Napi::Object res = Napi::Object::New(napi_env);
    res["ticks_ms"] = Worker::s_ticks_ms;
    res["start_delay_ms"] = Worker::s_start_delay_ms;
    res["stop_delay_ms"] = Worker::s_stop_delay_ms;

    UInt32 ticks_ms = napi_info[0].As<Napi::Number>().Int32Value();
    UInt32 start_delay_ms = napi_info[1].As<Napi::Number>().Int32Value();
    UInt32 stop_delay_ms = napi_info[2].As<Napi::Number>().Int32Value();
    configure<Worker>(ticks_ms, start_delay_ms, stop_delay_ms);
    Worker::s_ticks_ms = napi_info[0].As<Napi::Number>().Int32Value();
    Worker::s_start_delay_ms = napi_info[1].As<Napi::Number>().Int32Value();
    Worker::s_stop_delay_ms = napi_info[2].As<Napi::Number>().Int32Value();
    
    return napi_value(res);
}

/**
* demonstrate accessing arguments, returnin a value and how to handle errors
* NativeAddon.demo(astringArgument);    will return a string with the name of the demo function + the argument
* NativeAddon.demo(astringargument, 1)  will trigger a std::string exception
* NativeAddon.demo(astringargument, 2)  will trigger a Napi::Error (sub class of std::exception)
*/
napi_value
demo(napi_env env, napi_callback_info info)
{
    Napi::CallbackInfo napi_info(env, info);
    Napi::Env napi_env = napi_info.Env();
    auto length = napi_info.Length();
    try {
        if (length > 1) {
            Napi::Number arg1 = napi_info[1].As<Napi::Number>();
            UInt32 num = arg1.Int32Value();
            if (num == 1) {
                throw std::string("Wrong number of args in demo : throw std::string ");
            } else if (num == 2) {
                throw Napi::Error::New(env, "Wrong number of args in demo : throw Napi::Error");
            } else if (num == 3) {
                throw std::runtime_error("Wrong number of args for demo : throw std::runtime_error"); // cannot add string msg to std::exception
            } else if (num == 4) {
                Napi::Error::New(napi_env, "Wrong number of args for demo : throw Napi::Error direct to javascript").ThrowAsJavaScriptException();
            }
        }
        Napi::String ns = napi_info[0].As<Napi::String>();
        std::string ss = ns.Utf8Value();
        std::ostringstream sout;
        sout << "return from : [" << __PRETTY_FUNCTION__ << "] arg : " << ss;
        std::string ss2 = sout.str();
        Napi::String returnValue = Napi::String::New(napi_env, ss2);
        return napi_value(returnValue);
    } catch(std::string& except_s) {
        THROW_AS_JAVASCRIPT_EXCEPTION( "std::string exception in  : [" << __PRETTY_FUNCTION__ << "] msg : " << except_s);
//        TRACE("got a string exception : " << except_s );
//        std::ostringstream sout;
//        sout << "std::string exception in  : [" << __PRETTY_FUNCTION__ << "] msg : " << except_s;
//        std::string ss2 = sout.str();
//        Napi::Error::New(napi_env, ss2).ThrowAsJavaScriptException();
    } catch(Napi::Error& e) {
        TRACE("got a real exception : " << e.what());
        std::ostringstream sout;
        sout << "std::exception in : [" << __PRETTY_FUNCTION__ << "] what : " << e.what();
        std::string ss2 = sout.str();
        Napi::Error::New(napi_env, ss2).ThrowAsJavaScriptException();
    } catch(std::exception& e) {
        TRACE("got a real exception : " << e.what());
        std::ostringstream sout;
        sout << "std::exception in : [" << __PRETTY_FUNCTION__ << "] what : " << e.what();
        std::string ss2 = sout.str();
        Napi::Error::New(napi_env, ss2).ThrowAsJavaScriptException();
    }
    return nullptr;
}
/**
* Allows js code to peek into the state of the adon - again mainly for testing
*/
napi_value
peek(napi_env env, napi_callback_info info)
{
    try {

    } catch(...) {

    }
    return nullptr;
}
///
/// Start the addon based on the parameters in arg0 and sends events back to the
/// NODE/JS caller via the JS callback function provided in arg1. return a
/// napi_external which is used to provide context to the addon on callls to
/// other addon functions
///
napi_value
start(napi_env env, napi_callback_info info)
{
    CallerContext* caller_context = nullptr;
    Worker* worker_ptr = nullptr;
    TRACE("start entered" << std::hex << worker_ptr);
    try {
        ///
        /// Wrap the env, info in the c++ magic to make using this stuff easier
        ///
        Napi::CallbackInfo napi_info(env, info);
        Napi::Env napi_env = napi_info.Env();
        if (napi_info.Length() != 2) {
            Napi::Error::New(napi_env, std::string(__PRETTY_FUNCTION__) + std::string(" function 'start' expects 2 arguments" ) );
        }
        ///
        /// Decode arg0 into whatever c++ data structure best matches the input
        /// argument and the ultimate use of arg0 In this example
        /// StartFunction::Arg0 is a class that mirrors the js structure or
        /// arg0. This process is often termed 'marshalling'
        ///
        Napi::Value arg0Value = napi_info[0];
        StartFunctionArg0 arg0(arg0Value);

        ///
        /// arg1 should be a js callback function; so test it is and then do
        /// some magic so that it can be called (or scheduled) from a non-main
        /// thread
        ///
        Napi::Value arg1Value = napi_info[1];
        if (!arg1Value.IsFunction()) {
            throw Napi::Error::New(napi_env, "expected argv[1] to be a callback function ");
        }
        Napi::Function callbackFunction = arg1Value.As<Napi::Function>();

        ///
        /// Need to create a data structure to hold context data for each call
        /// to function start() This will be, wrapped into an opaque object ad
        /// passed back as the return value of a start() call.
        ///
        caller_context = new CallerContext(napi_env, arg0);
        ///
        /// Now this is a neat little exercise. Here we combine the js/node
        /// callback function with
        /// -   a function called CallJs
        /// -   and some other napi magic
        /// So that from the worker thread we can schedule the node/js callback
        /// to be run on the main threads run loop. However the node runtime
        /// does not call the node/js callback but rather calls the CallJs
        /// function whose job is to build an array of arguments to be passed to
        /// the node/js callback. If CallJs is NOT provided the node/js callback
        /// is called with no arguments and this==undefined. Since we want to
        /// pass event objects to the node/js callback we need to provide a
        /// CallJs function that does that job. the CallJs function is internal
        /// to TSFunction class
        ///
        caller_context->env = env;
        caller_context->tsfunction = TSFunction::New(arg1Value.As<Napi::Function>(), caller_context, call_js);

        ///
        /// This is an example of how to setup a c/c++ function that can be called across thread boundaries
        ///
        #ifdef GHGHGHG
        caller_context->cpptsfunction_dummy = CppTSFunction::New(env, caller_context, [](::napi_env env, napi_value callback, void* context, void* data){
            Event* event = (Event*)data;
            TRACE("env : [" << env << "] context : [" << std::hex << context << "] data : [" << std::hex << data << "] type " << event->_type);
            /// in here do what ever is required
            
            ///
            
            /// the next is not required - not manditory to napi_call_function the js function
            return;
            napi_value recv;
            napi_value args[2];
            napi_status status = napi_call_function(env, recv, callback, 0, args, &recv);
            if (status != napi_ok) {
                TRACE("status" << status);
            }
        } /*cpp_callback*/);
        #endif
        ///
        /// Now w can start the real work - the details are hidden in the
        /// following function call. The significant thing here is that this
        /// call returns a pointer to a 'context object' that provides the info
        /// that other addon functions require to perform their job.
        ///
        /// so for example in js land the statements illustrate this
        ///
        /// let opaqueContext = Addon.start(someObject, (...args) => {
        ///     .......
        ///     .......
        ///     .......
        ///     /* when ready to stop */
        ///     Addon.stop(opaqueContext)
        /// })
        ///
        ///

        /// start a worker thread - here define a macro with the name of the specific worker class
        /// this is cheap and nasty - coorect way would be to use templates
#define WORKER_CLASS CFWorker
        worker_ptr = WORKER_CLASS::getInstance();
        /// attach it to the caller_context
        caller_context->worker_ptr = worker_ptr;
        /// add a 'job' for the thread to do - this thread only allows 1 job
        worker_ptr->addCaller(caller_context, arg0, [](CallerContext* caller, Event* event) {
            TRACE("caller: " << std::hex << caller << " event: " << std::hex << event << " type:" << event->_type << " ident: " << event->_ident);
            caller->tsfunction.callBlocking(event);
//            caller->cpptsfunction_dummy.callBlocking(event);
        });
        TRACE("starting worker data : " << std::hex << (void*)caller_context);

        ///
        /// However we cannot return a CallerContext* back to js land in its C++
        /// form; we need to wrap it as a napi_external/Napi::External. Which
        /// will be seen by JS as an opaque object.
        ///
        /// In addition we attach a finalizer function that will be called when
        /// the external is garbage collected that seems like the right time to
        /// free up the CallerContext
        ///
        auto res = Napi::External<CallerContext>::New(env, caller_context, CallerContext::finalize); // dummy);
        return napi_value(res);

    } catch (std::string errMsg) {

        if (caller_context->tsfunction.isEmpty()) {
            caller_context->tsfunction.release();
        }
        if (caller_context != nullptr) {
            delete caller_context;
        }
        if (worker_ptr != nullptr) {
            delete worker_ptr;
        }
        Napi::Error::New(env, errMsg).ThrowAsJavaScriptException();
        ///
        /// A note on error handling
        /// ========================
        /// I have chosen to build this addon with Cpp Exceptions enabled and
        /// have all functions/classes etc below the main addon entry points use
        /// throw/catch to indicate errors.
        ///
        /// Back at the level of the addon functions I want to catch any c++
        /// exception and turn it into js Error().
        ///
        /// the napi_throw_error call queues a JS Error and the return NULL
        /// triggers it into action.
        ///
    }
    return nullptr;
}

///
/// stop the addon all together,
/// -   terminate the background thread
/// -   a final 'closing' event will be sent
/// Only argument is the napi_external that holds the 'context'
///
napi_value
cancel(napi_env env, napi_callback_info info)
{
    try {
        TRACE("cancel called");
        Napi::CallbackInfo napi_info(env, info);
        Napi::Env napi_env = napi_info.Env();
        if (napi_info.Length() != 1) {
            Napi::Error::New(napi_env, std::string(__PRETTY_FUNCTION__) + std::string(" function 'cancel' expects 1 argument" )).ThrowAsJavaScriptException();
        }
        Napi::Value arg0Value = napi_info[0];
        CallerContext* caller_context = arg0Value.As<Napi::External<CallerContext>>().Data();
        
        /// we probably dont need to cast this to a specific worker type
        WORKER_CLASS* wt = dynamic_cast<WORKER_CLASS*>(caller_context->worker_ptr);
        wt->cancel(caller_context, [](CallerContext* caller, Event* event) {
            TRACE("caller: " << std::hex << caller << " event: " << std::hex << event << " type:" << event->_type << " ident: " << event->_ident);
            caller->tsfunction.callBlocking(event);
//            caller->cpptsfunction_dummy.callBlocking(event);
        });

        ///
        /// at this point cannot be sure the worker thread has stopped.
        /// Must let the thread and callback stuff cleanup
        ///
        Napi::Value result  = napi_env.Undefined();
        return napi_value(result);
    } catch(Napi::Error error) {
        error.ThrowAsJavaScriptException();
        return nullptr;
    }
}


///
/// stop the addon all together,
/// -   terminate the background thread
/// -   a final 'closing' event will be sent
/// Only argument is the napi_external that holds the 'context'
///
napi_value
stop(napi_env env, napi_callback_info info)
{
    try {
        TRACE("stop called");
        Napi::CallbackInfo napi_info(env, info);
        Napi::Env napi_env = napi_info.Env();
        if (napi_info.Length() != 1) {
            Napi::Error::New(napi_env, std::string(__PRETTY_FUNCTION__) + std::string(" function 'stop' expects 1 argument" )).ThrowAsJavaScriptException();
        }
        Napi::Value arg0Value = napi_info[0];
        CallerContext* caller_context = arg0Value.As<Napi::External<CallerContext>>().Data();
        
        /// we probably dont need to cast this to a specific worker type
        WORKER_CLASS* wt = dynamic_cast<WORKER_CLASS*>(caller_context->worker_ptr);
    #ifdef OPTION_JOIN_IN_STOPFUNCTION
        TRACE("about to issue stop");
        wt->stop();
        TRACE("returned from stop");
//        caller_context->tsfunction.release();
        delete caller_context;
//        delete wt;
    #else
        wt->stop();
    #endif

        ///
        /// at this point cannot be sure the worker thread has stopped.
        /// Must let the thread and callback stuff cleanup
        ///
        Napi::Value result  = napi_env.Undefined();
        return napi_value(result);
    } catch(Napi::Error error) {
        error.ThrowAsJavaScriptException();
        return nullptr;
    }
}
////
/// Define the function entry points to this addon.
///
napi_value
Init(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        { "start",     NULL, start,     NULL, NULL, NULL, napi_default, NULL },
        { "cancel",    NULL, cancel,    NULL, NULL, NULL, napi_default, NULL },
        { "stop",      NULL, stop,      NULL, NULL, NULL, napi_default, NULL },
        { "configure", NULL, configure, NULL, NULL, NULL, napi_default, NULL },
        { "demo",      NULL, demo,      NULL, NULL, NULL, napi_default, NULL },
        { "peek",      NULL, peek,      NULL, NULL, NULL, napi_default, NULL }
    };
    CHECK(napi_define_properties(env, exports, 4, descriptors) == napi_ok);
    return exports;
}
NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
