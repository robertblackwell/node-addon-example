#ifndef RWELL_ARGUMENTS_GUARD
#define RWELL_ARGUMENTS_GUARD
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

#ifdef OLD_ARG0
///
/// StartFunctionArg0 = The arguments passed from js/node to the addon start
/// function
///
class StartFunctionArg0
{
  public:
    StartFunctionArg0(Napi::Value arg0Value);

    std::string getIdent();
    std::vector<std::string> getStrings();

  protected:
    Napi::Value _value;
    std::string _ident;
    std::vector<std::string> _stringsVector;
};
#endif
struct StartFunctionScheduleItem
{
    long ident;
    long interval;
    long repeats;
    Napi::Value replyobj;
};
class StartFunctionArg0
{
  public:
    StartFunctionArg0(Napi::Value arg0Value);
    std::string                            _label;
    std::vector<StartFunctionScheduleItem> _schedule;
};
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

#endif
