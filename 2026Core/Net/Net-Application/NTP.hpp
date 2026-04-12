#pragma once

// TODO: Use std::byte instead of uint8_t for byte-oriented data

// Custom Imports
#include "../NetAdapter_A.hpp"

// Library Imports
#include <etl/format_spec.h>
#include <etl/string.h>
#include <etl/to_string.h>

/* Constants */
#define IMPL_OFFSET 0
#define IMPL_HWTIMER 1
#define IMPL_SYSRTC 2
#define IMPL_POSIX 3
#define IMPL_USED IMPL_OFFSET

#if IMPL_USED == IMPL_HWTIMER
// @see
// https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html
#    warning "Hardware Timer Implementation Incomplete"
#elif IMPL_USED == IMPL_POSIX
#    warning "POSIX Implementation Incomplete"
#    warning "POSIX Implementation not recommended, it may be slow"
#endif

/**
 * @brief A clock that can be synced
 */
class SyncedClock {
  public: // MARK: Public
    static constexpr const char *TAG = "NTP";

    /**
     * @brief Construct a new Synced Clock object
     * @param netAdapter the network adapter to use for time sync
     * @see ../docs/cpp/explicit_SonarLint.md
     * @includedoc ../docs/cpp/explicit_SonarLint.md
     */
    explicit SyncedClock(NetAdapter_A &netAdapter);
    ~SyncedClock() = default;

    /**
     * @brief Initialize time synchronization
     * @param upstreamMAC the MAC address of the upstream clock
     * @returns true if successful, false otherwise
     */
    bool initTimeSync(const etl::array<uint8_t, 6> upstreamMAC);

    /**
     * @brief Handle incoming requests
     * @returns true if successful, false otherwise
     */
    bool requestHandler(const etl::array<uint8_t, 6> upstreamMAC);

    // MARK: Getters

    /**
     * @brief Get the number of microseconds since the last sync
     * @returns the number of microseconds since the last sync
     */
    unsigned long getMicrosSinceSync() const;

    /**
     * @brief Get the current corrected time in microseconds
     * @returns the current corrected time in microseconds
     */
    unsigned long getMicros() const;

    /**
     * @brief Get the current correction factor in microseconds
     * @returns the current correction factor in microseconds
     */
    unsigned long getCorrectionFactor() const;

    constexpr static uint_fast8_t LOG_STRING_SIZE = 3 + ((11 + 6) * 3) + 1;
    /**
     * @brief Get at string that describes the current state of the clock
     * @returns the current state of the clock as a string
     */
    etl::string<LOG_STRING_SIZE> getLogString() const;

  private: // MARK: Private
    NetAdapter_A &netAdapter;
    unsigned long lastSyncTime_micros = 0;
    unsigned long timeSyncInit_micros = 0;
    unsigned long correctionFactor_micros = 0;

// MARK: System Timer
#if IMPL_USED == IMPL_SYSRTC
#    warning "SysClock RTC Implementation Incomplete"
#    warning                                                                   \
        "SysClock RTC Implementation not recommended due to unknown consequences of messing with the primary system clock"

    uint64_t esp_timer_impl_get_counter_reg(void) const;

    /**
     * @brief Adjusts high power/precision UNIT0 system timer by an amount of
     * microseconds
     * @param deltaMicros the amount of microseconds to adjust the timer by
     * @returns true if successful, false otherwise
     * @deprecated Unfinished, not recommended
     */
    static bool updateSystemTimer(int32_t deltaMicros);

    /**
     * @brief Gets the current value of the high power/precision UNIT0 system
     * timer
     * @returns the current value of the system timer
     * @deprecated Unfinished, not recommended
     */
    static int64_t getSystemTimer();
#endif // IMPL_SYSRTC
};
