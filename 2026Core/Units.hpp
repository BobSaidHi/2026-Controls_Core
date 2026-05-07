/**
 * @file Units.hpp
 * @brief Common Unit Constants File
 * @author Noah (@BobSaidHi)
 */

#pragma once

// Standard Includes
#include <cstdint>

// MARK: Constants

namespace UNITS {
    /* Additional Checks*/
    constexpr uint_fast8_t TIME_TO_MICROS_ROLLOVER_MINS =
        UINT32_MAX / 1000000 / 60;
    constexpr uint_fast8_t TIME_TO_MILLIS_ROLLOVER_DAYS =
        UINT32_MAX / 1000 / 60 / 60 / 24;

    /* General Conversions */
    constexpr unsigned long m_TO_BASE = 1000;
    constexpr unsigned long u_TO_m = 1000;
    constexpr unsigned long u_TO_BASE = m_TO_BASE * u_TO_m;

    /* Time Constants */
    constexpr uint32_t SECS_PER_MIN = 60;
    constexpr uint32_t MILLIS_PER_SEC = m_TO_BASE;
    constexpr uint32_t MICROS_PER_MILLI = u_TO_m;
    constexpr uint32_t MICROS_PER_SEC = u_TO_BASE;

    /* Voltage Constants */
    constexpr uint_fast16_t mV_PER_V = m_TO_BASE;
    constexpr uint_fast16_t uV_PER_mV = u_TO_m;
    constexpr uint_fast16_t uV_PER_V = mV_PER_V * uV_PER_mV;
    constexpr uint_fast16_t uVOLTS_PER_VOLT = uV_PER_V;

} // end namespace UNITS
