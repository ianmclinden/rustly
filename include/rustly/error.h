#pragma once

#include <rustly/display.h>

namespace rustly
{
    template <typename E>
    concept Error = ToString<E>;
}