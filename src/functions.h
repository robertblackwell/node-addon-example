#ifndef RWELL_FUNCTIONS_GUARD
#define RWELL_FUNCTIONS_GUARD
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

void fse_unwatch(CallerContext* watcher);
void* fse_context_of(CallerContext* watcher);
CallerContext* worker_start(StartFunctionArg0 arg0, napi_threadsafe_function tsf_cb);

#endif
