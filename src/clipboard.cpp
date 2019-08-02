
#include "clipboard.h"

#include <iostream>


Clipboard& Clipboard::get()
{
    static Clipboard instance;

    instance.init_platform();

    return instance;
}

Clipboard::~Clipboard()
{
    cleanup_platform();
}
