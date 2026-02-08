#pragma once

// TODO: Use std::byte instead of uint8_t for byte-oriented data

// Custom Imports
#include "../NetAdapter_A.hpp"

// Library Imports

/* Constants */

/**
 * @brief A clock that can be synced
 */
class SyncedClock {
  public: // MARK: Public
    static constexpr const char *TAG = "NTP";

    /**
     * @brief Construct a new Synced Clock object
     * @param netAdapter the network adapter to use for time sync
     */
    SyncedClock(NetAdapter_A &netAdapter);
    ~SyncedClock();

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

  private: // MARK: Private
    NetAdapter_A &netAdapter;
    unsigned long lastSyncTime_micros = 0;
    unsigned long timeSyncInit_micros = 0;
    unsigned long correctionFactor_micros = 0;

    // MARK: System Timer

    // uint64_t esp_timer_impl_get_counter_reg(void) const;

    /**
     * @brief Adjusts high power/precision UNIT0 system timer by an amount of
     * microseconds
     * @param deltaMicros the amount of microseconds to adjust the timer by
     */
    // static bool updateSystemTimer(int32_t deltaMicros);

    /**
     * @brief Gets the current value of the high power/precision UNIT0 system
     * timer
     * @returns the current value of the system timer
     */
    // static int64_t getSystemTimer();
};
