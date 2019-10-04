#ifndef RWELL_ACTIVE_GUARD
#define RWELL_ACTIVE_GUARD
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "caller_context.h"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <memory>
#include <napi.h>
#include <node_api.h>
#include <stdio.h>
#include <string>
#include <vector>

class ActiveCallers
{
  public:
    static void init();
    static void addCaller(CallerContext* caller);
    static int findCaller(CallerContext* caller);
    static void removeCaller(CallerContext* caller);
};

#endif
