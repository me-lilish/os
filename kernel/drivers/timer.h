#pragma once
#include "../core/ktypes.h"

namespace Timer {
    void init(uint32_t frequency_hz);
    uint32_t ticks();
    bool quantum_elapsed();
    void clear_quantum_flag();
    void sleep_ticks(uint32_t count);
}
