// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

// Custom Imports
#include "NTP.hpp"
#include "../Net-Link/Packet.hpp"

// Library Imports
#include "esp_log.h"
#include <Arduino.h>
#include <sys/time.h>

SyncedClock::SyncedClock(NetAdapter_A *netAdapter) : netAdapter(netAdapter) {}

SyncedClock::~SyncedClock() {}

bool SyncedClock::initTimeSync(const etl::array<uint8_t, 6> upstreamMAC) {
    // Time data
    timeSyncInit_micros = micros();

    // Convert to byte array
    const uint8_t *timeSyncInit_data = (uint8_t *)timeSyncInit_micros;
    etl::array<uint8_t, WTbNetConfig::MAX_PACKET_DATA_LENGTH> data = {
        timeSyncInit_data[0], timeSyncInit_data[1], timeSyncInit_data[2],
        timeSyncInit_data[3]};

    // Convert to packet
    Packet packet = Packet(data, (uint8_t)sizeof(timeSyncInit_micros),
                           Packet::PacketType::NTPRequest);

    // Send
    return netAdapter->send(upstreamMAC, packet, true);
}

bool SyncedClock::requestHandler(const etl::array<uint8_t, 6> upstreamMAC) {
    if (timeSyncInit_micros == 0) {
        // Return current time to client
        // Time data
        unsigned long currentTime_micros = micros();

        // Convert to byte array
        const uint8_t *timeSyncInit_data = (uint8_t *)timeSyncInit_micros;
        etl::array<uint8_t, WTbNetConfig::MAX_PACKET_DATA_LENGTH> data = {
            timeSyncInit_data[0], timeSyncInit_data[1], timeSyncInit_data[2],
            timeSyncInit_data[3]};

        // Convert to packet
        Packet packet = Packet(data, (uint8_t)sizeof(timeSyncInit_micros),
                               Packet::PacketType::NTPRequest);

        return netAdapter->send(upstreamMAC, packet, true);
    } else {
        // Complete time sync
        unsigned long currentTime_micros = micros();
        correctionFactor_micros = currentTime_micros - timeSyncInit_micros;
        lastSyncTime_micros = currentTime_micros;
        timeSyncInit_micros = 0;

        /**
         * @brief Update system time
         * @see https://www.man7.org/linux/man-pages/man3/adjtime.3.html
         * @see
         * https://stackoverflow.com/questions/1546882/how-to-convert-errno-in-unix-to-corresponding-string
         */
        timeval delta = {0, correctionFactor_micros};
        // TODO: Does this affect micros()?
        if (adjtime(&delta, nullptr) != 0) {
            ESP_LOGE(TAG, "Failed to adjust system time:", strerror(errno));

            return false;
        }

        // Success
        return true;
    }
}

// MARK: Getters

unsigned long SyncedClock::getMicrosSinceSync() const {
    return micros() - lastSyncTime_micros;
};

unsigned long SyncedClock::getMicros() const {
    return micros() + correctionFactor_micros;
}

unsigned long SyncedClock::getCorrectionFactor() const {
    return correctionFactor_micros;
}
