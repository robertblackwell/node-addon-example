#include "caller_context.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "arguments.h"
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

CallerContext::CallerContext(Napi::Env napi_env, StartFunctionArg0 arg0)
  : tsf_cb(nullptr)
  , napi_env(napi_env)
  , startArg0(arg0)
{}
CallerContext::~CallerContext()
{
    TRACE("this" << std::hex << this);
}
void
CallerContext::finalize(Napi::Env env, CallerContext* finalizeData)
{
    TRACE("");
}
