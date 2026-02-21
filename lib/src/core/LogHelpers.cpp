#pragma once
#include <iostream>
#include "LogHelpers.h"
namespace {
    bool _debug = false;
}

void sequoia::setDebug(bool flag) { _debug = flag; };
bool sequoia::getDebug() { return _debug; };
