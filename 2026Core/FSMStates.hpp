#pragma once

#include <cstdint>

namespace FSMCommon {
    enum States : uint_fast8_t {
        sINIT = 0,
        sESTOP,
        sRST,
        sStartLoad,
        sRunLoad,
        sCurtail
    };
}
