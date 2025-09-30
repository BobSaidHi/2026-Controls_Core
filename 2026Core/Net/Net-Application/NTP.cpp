// Custom Imports
#include "NTP.hpp"

// Library Imports
#include <Arduino.h>

SyncedClock::SyncedClock(NetAdapter_A *netAdapter) : netAdapter(netAdapter) {}

SyncedClock::~SyncedClock() {}

bool SyncedClock::initTimeSync(const etl::array<uint8_t, 6> upstreamMAC) {
    timeSyncInit_micros = micros();
    return netAdapter->send(upstreamMAC, *timeSyncInit_micros,
                            sizeof(timeSyncInit_micros),
                            true); // todo add header
}

bool SyncedClock::requestHandler() {
    if (timeSyncInit_micros == 0) {
        // Return current time to client
        unsigned long currentTime = micros();
        return netAdapter->send(upstreamMAC, *currentTime, sizeof(currentTime),
                                true); // todo add header
    } else {
        // Complete time sync
        unsigned long currentTime = micros();
        correctionFactor_micros = currentTime - timeSyncInit_micros;
        lastSyncTime_micros = currentTime;
        timeSyncInit_micros = 0;
        return true;
    }
}

unsigned long getMicrosSinceSync() const {
    return micros() - lastSyncTime_micros;
};

unsigned long SyncedClock::getMicros() const {
    return micros() + correctionFactor_micros;
}

unsigned long SyncedClock::getCorrectionFactor() const {
    return correctionFactor_micros;
}
