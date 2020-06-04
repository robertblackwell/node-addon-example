#include "arguments.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "check.h"
#include "logger.h"
#include <assert.h>
#include <iostream>
#include <memory>
#include <napi.h>
#include <node_api.h>
#include <stdio.h>
#include <string>
#include <vector>

#ifdef OLD_ARG0
StartFunctionArg0::StartFunctionArg0(Napi::Value arg0)
{
    std::vector<std::string> stringsVector({});
    if (!arg0.IsObject()) {
        throw Napi::Error::New(arg0.Env() ,"arg 0 is not an object");
    }
    Napi::Object arg0Obj = arg0.As<Napi::Object>();
    std::string identKey("ident");
    std::string ident = arg0Obj.Get(identKey).ToString().Utf8Value();
    _ident = ident;
    std::string stringsKey("strings");
    Napi::Value stringsValue = arg0Obj.Get(std::string("strings")); //(stringsKey);
    if (!stringsValue.IsObject()) {
        throw Napi::Error::New(arg0.Env(), "arg[0]['stringsValue'] is not an object");
    }

    Napi::Object stringsAsObj = stringsValue.As<Napi::Object>();
    Napi::Array stringsAsArray = stringsValue.As<Napi::Array>();
    UInt32 length = stringsAsArray.Length();

    for (UInt32 i = 0; i < length; i++) {
        Napi::Value el = stringsAsObj.Get(i);
        if (!el.IsString()) {
            throw Napi::Error::New(arg0.Env() "expected array element to be string index : " + std::to_string(i);
        }
        auto str = stringsAsObj.Get(i).ToString().Utf8Value();
        stringsVector.push_back(str);
        _stringsVector.push_back(str);
    }
}
std::string
StartFunctionArg0::getIdent()
{
    return _ident;
}
std::vector<std::string>
StartFunctionArg0::getStrings()
{
    return _stringsVector;
}
#endif
///
/// StartFunctionArg0 - the new one
///
StartFunctionArg0::StartFunctionArg0(Napi::Value arg0)
{
    TRACE(" entered");
    if (!arg0.IsObject()) {
        throw Napi::Error::New(arg0.Env(), "arg 0 is not an object");
    }
    Napi::Object arg0Obj = arg0.As<Napi::Object>();
    if (!arg0Obj.Has(std::string("schedule"))) {
        throw Napi::Error::New(arg0.Env(), "arg0 does not have property names schedule");
    }
    this->_label = arg0Obj.Get("label").As<Napi::String>().Utf8Value();
    // Napi::Value schedValue = arg0Obj.Get("schedule");
    Napi::Object schedObject = arg0Obj.Get(std::string("schedule")).As<Napi::Object>();
    Napi::Array schedArray = arg0Obj.Get(std::string("schedule")).As<Napi::Array>();
    if (!schedObject.IsObject()) {
        throw Napi::Error::New(arg0.Env(), "arg0 value of schedule property is NOT object");
    }
    if (!schedArray.IsArray()) {
        throw Napi::Error::New(arg0.Env(), "arg0 value of schedule property is NOT array");
    }
    UInt32 length = schedArray.Length();

    for (UInt32 i = 0; i < length; i++) {
        Napi::Object el = schedArray.Get(i).As<Napi::Object>();
        StartFunctionScheduleItem temp;
        temp.ident = el.Get("ident").ToNumber().Int64Value();
        temp.interval = el.Get("interval").ToNumber().Int64Value();
        temp.repeats = el.Get("repeats").ToNumber().Int64Value();
        temp.replyobj = el;
        this->_schedule.push_back(temp);
    }
}
///
/// end of StartFunctionArg0
///
///
/// StartReturn - a class representing the return value/object from the start
/// function
///
/*
    class StartReturn {
    public:
        StartReturn(napi_env env, void* externalContext);
        operator napi_value() const;
    protected:
        napi_env    _env;
        napi_value  _value;
        void*       _externalContext;
    };
    StartReturn::StartReturn(napi_env env, void* externalContext): _env(env),
   _externalContext(externalContext)
    {
    }
    inline StartReturn::operator napi_value() const {
        return _value;
    }
*/
