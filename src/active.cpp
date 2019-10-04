
#include "active.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include "check.h"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <memory>
#include <napi.h>
#include <node_api.h>
#include <stdio.h>
#include <string>
#include <vector>

typedef std::vector<CallerContext*> CallerList;

static std::vector<CallerContext*> activeCallers;

void
ActiveCallers::init()
{
    activeCallers = std::vector<CallerContext*>();
}
void
ActiveCallers::addCaller(CallerContext* caller)
{
    if (ActiveCallers::findCaller(caller)) {
        throw std::string("ActiveCaller::addCaller the caller already exists");
    }
    activeCallers.push_back(caller);
}
int
ActiveCallers::findCaller(CallerContext* caller)
{
    std::vector<CallerContext*>::iterator it = std::find(activeCallers.begin(), activeCallers.end(), caller);
    if (it != activeCallers.end()) {
        int index = std::distance(activeCallers.begin(), it);
        return index;
    }
    return -1;
}
void
ActiveCallers::removeCaller(CallerContext* caller)
{
    int index = ActiveCallers::findCaller(caller);
    if (index == -1) {
        throw std::string("ActiveCaller::removeCaller - caller is not in active list");
    }
    activeCallers.erase(activeCallers.begin() + index);
}
