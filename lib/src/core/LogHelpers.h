#pragma once
#include <iostream>
#define SEQUOIA_LOG_DEBUG(x) if (sequoia::getDebug()) {std::cout << x << std::endl;}
#define SEQUOIA_DEBUG (sequoia::getDebug()) 
namespace sequoia {
    void setDebug(bool flag);
    bool getDebug();

}