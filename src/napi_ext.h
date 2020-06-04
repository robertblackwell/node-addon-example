#ifndef RWELL_NAPIEXT_GUARD
#define RWELL_NAPIEXT_GUARD
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

typedef struct Event PlainEvent;
///
/// This file and this class is really being pedantic - but is experimenting
/// with extending the Napi C++ approach to additional C++ data types.
///
/// The aim here is to be able to write
///
///     Napi::Event n_ev = Napi::Event::New(env, event)
///
///     just like we can write
///
///     Napi::String ns = Napi::String::New(env, somestring).
///
/// To achive this I have introduced an namespace napiExt (so as not to
/// interfere with Napi::) and defined a new class NapiExt::Event the extends
/// napi::Value
///
/// I could also ask to go the other way - from a NapiExt::Event to a plan c++
/// Event, this would look something like
///
///     Event plain_event = anapivalue.As<CppEvent>().CppEvent()
///
/// To make this work I first note that a napivalue.As<CppEvent> already works by
/// virtue of the existing Napi:: code. The extra work needed is to implement
/// the CppEvent() method the class NapiExt::Event. We would probably also want
/// to be able to test a napi::Value to see if it is a NapiExt::Event. This is a
/// bit more complicated; since we actually need to extend Napi::Value by adding
/// an bool IsCppEvent() method. But we cannot add this to NapiValue hence we
/// create a class NapiExt::Value which extends Napi::Value with the new method.
///
namespace NapiExt {

class Event : public ::Napi::Value
{
  public:
    Event(napi_env env, PlainEvent* event);
    static Event New(napi_env env, PlainEvent* event);
};
} // namespace

#endif
