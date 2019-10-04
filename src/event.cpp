#include "event.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "check.h"
#include <assert.h>
#include <ctime>
#include <iostream>
#include <memory>
#include <napi.h>
#include <node_api.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>

/**
 * @TODO - get rid of the duplicate code
 */
inline std::string
currentTime()
{
    time_t now = time(0);
    char* tstr = ctime(&now);
    std::string s(tstr);
    return s;
}
Event*
Event::New(std::string type, std::string label, int64_t ident)
{
    Event* ev = new Event();
    ev->_ident = ident;
    ev->_type = type;
    ev->_label = label;
    ev->_thread = (int64_t)(pthread_self());
    ev->_time_stamp = currentTime();
    return ev;
}

Napi::Value
Event::ToNapi(napi_env env)
{
    //    try {
    std::stringstream stream;
    stream << std::hex << this->_thread;
    std::string result(stream.str());
    Napi::String label = Napi::String::New(env, this->_label);
    Napi::String type = Napi::String::New(env, this->_type);
    Napi::Number ident = Napi::Number::New(env, this->_ident);
    Napi::String thread = Napi::String::New(env, result);
    Napi::String timestamp = Napi::String::New(env, this->_time_stamp);
    Napi::Object obj = Napi::Object::New(env);
    obj["type"] = type;
    obj["thread"] = thread;
    obj["timestamp"] = timestamp;
    obj["ident"] = ident;
    obj["label"] = label;
    return obj;
}
Napi::Value
Event::ToNapi(Napi::Env napienv)
{
    napi_env env = napi_env(napienv);
    return this->ToNapi(env);
}
Napi::Value
pack(napi_env env, Event* event)
{
    std::stringstream stream;
    stream << std::hex << event->_thread;
    std::string result(stream.str());
    Napi::String label = Napi::String::New(env, event->_label);
    Napi::String type = Napi::String::New(env, "interval");
    Napi::String thread = Napi::String::New(env, result);
    Napi::String timestamp = Napi::String::New(env, event->_time_stamp);
    Napi::Object obj = Napi::Object::New(env);
    obj["type"] = type;
    obj["thread"] = thread;
    obj["timestamp"] = timestamp;
    obj["label"] = label;
    return obj;
}
Napi::Value
pack(Napi::Env napienv, Event* event)
{
    return pack(napi_env(napienv), event);
}
