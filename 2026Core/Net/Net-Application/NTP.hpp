#pragma once

// Custom Imports
#include "../NetAdapter_A.hpp"

// Library Imports

/* Constants */

/**
 * @brief A clock that can be synced
 */
class SyncedClock {
  public:
    /**
     * @brief Construct a new Synced Clock object
     * @param netAdapter the network adapter to use for time sync
     */
    SyncedClock(NetAdapter_A *netAdapter);
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
    bool requestHandler();

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
    int_fast8_t getCorrectionFactor() const;

  private:
    NetAdapter_A *netAdapter;
    unsigned long lastSyncTime_micros = 0;
    unsigned long timeSyncInit_micros = 0;
    int_fast8_t correctionFactor_micros = 0;
};
