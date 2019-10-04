
#include "napi_ext.h"
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

namespace NapiExt {

Event::Event(napi_env env, PlainEvent* event)
  : ::Napi::Value()
{
    _env = env;
    _value = napi_value(event->ToNapi(env));
}
Event
Event::New(napi_env env, PlainEvent* event)
{
    return Event(env, event);
}
} // namespace
