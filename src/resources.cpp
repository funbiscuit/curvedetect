
#include "resources.h"

Resources& Resources::getInstance()
{
    static Resources instance;
    return instance;
}

