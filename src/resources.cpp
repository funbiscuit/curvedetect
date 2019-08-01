
#include "resources.h"

Resources& Resources::get()
{
    static Resources instance;
    return instance;
}

