// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

// Import Header
#include "NetAdapter_A.hpp"

// Custom Imports
#include "Net-Link/Packet.hpp"

// Library Imports
#include "esp_log.h"
#include <Arduino.h>

// Logging Config

// Definition of static member variables

/* MARK: Standard Functions */

NetAdapter_A::NetAdapter_A() { // TODO: consider making this static
    // Initialize Logger
    ESP_LOGI(TAG, "init");
}

/* Public Static Concrete Utility Functions */
// MARK: Utilities

uint32_t NetAdapter_A::testThroughput(uint_fast8_t maxTime,
                                      uint_fast32_t maxBytes, bool checkLatency,
                                      bool verifyReceipt) {
    ESP_LOGV(TAG, "Checking WiFi");
    // delay(1000);
    //  Verify WiFi is connected
    //  assert(WiFi.status() == WL_CONNECTED); // todo superclass or include

    // Create a Packet
    Packet packet;
    etl::array<uint8_t, WTbNetConfig::MAX_PACKET_DATA_LENGTH> data;
    // Prepare data
    for (int i = 1; i < WTbNetConfig::MAX_PACKET_DATA_LENGTH; i++) {
        data[i] = (uint8_t)i;
    }
    ESP_LOGV(TAG, "Default Packet Ready");
    // delay(1000);

    // Setup Packet if not checking latency
    if (!checkLatency) {
        ESP_LOGV(TAG, "Encoding Latency Info");

        static_assert(data.size() == WTbNetConfig::MAX_PACKET_DATA_LENGTH);

        packet.setContents(data, data.size(), Packet::PacketType::SpeedTest);
    }
    ESP_LOGV(TAG, "Packet Fully Ready");
    // delay(1000);

    // Calculate maximum test duration
    const uint64_t startTime = millis();
    ESP_LOGD(TAG, "sT: ", startTime);
    const uint64_t endTime = millis() + ((uint32_t)maxTime * 1000);
    ESP_LOGD(TAG, "eT: ", endTime);
    ESP_LOGV(TAG, "Test Duration Calculated");
    // delay(1000);

    // Track number of bytes actually sent
    uint32_t bytesSent = 0;

    // Send packets until the test is over
    while (millis() < endTime && bytesSent < maxBytes) {
        ESP_LOGV(TAG, "SP");
        // Update packet if checking latency
        // constexpr uint8_t millisLen = sizeof(millis());
        data[0] = (uint8_t)false;
        if (checkLatency) {
            ESP_LOGD(TAG, "Checking Latency");
            const uint8_t *currentTime = (uint8_t *)millis();
            static_assert(data.size() == WTbNetConfig::MAX_PACKET_DATA_LENGTH);
            ESP_LOGV(TAG, "Copying Time");
            memcpy(data.begin() + 1, &currentTime, sizeof(currentTime));
            ESP_LOGV(TAG, "Time Copied");
            packet.setContents(data, data.size(), Packet::PacketType::NetTest);
        }
        ESP_LOGV(TAG, "Packet Ready");

        // Actually send the packet
        this->sendAll(packet.getRawPacket(), packet.getLengthValid());
        // Statistics
        bytesSent += WTbNetConfig::MAX_PACKET_DATA_LENGTH;
    }
    ESP_LOGV(TAG, "Test Almost Complete");

    // Send once more w/ end indicator
    // Update packet if checking latency
    // constexpr uint8_t millisLen = sizeof(millis());
    data[0] = (uint8_t)true;
    if (checkLatency) {
        ESP_LOGD(TAG, "Checking Latency");
        const uint8_t *currentTime = (uint8_t *)millis();
        static_assert(data.size() == WTbNetConfig::MAX_PACKET_DATA_LENGTH);
        ESP_LOGV(TAG, "Copying Time");
        memcpy(data.begin() + 1, &currentTime, sizeof(currentTime));
        ESP_LOGV(TAG, "Time Copied");
        packet.setContents(data, data.size(), Packet::PacketType::NetTest);
    }

    // Actually send the packet
    this->sendAll(packet.getRawPacket(), packet.getLengthValid());
    // Statistics
    bytesSent += WTbNetConfig::MAX_PACKET_ABS_LEN;
    ESP_LOGV(TAG, "Test Complete");

    // Compute actual speeds
    ESP_LOGD(TAG, "bytesSent: ", bytesSent);
    uint64_t elapsedTime = millis() - startTime;
    ESP_LOGD(TAG, "timeElapsed: ", elapsedTime);
    auto netSpeed = (uint32_t)((uint64_t)bytesSent * 1000 / elapsedTime);
    ESP_LOGI(TAG, "SPD-bps: ", netSpeed);
    ESP_LOGV(TAG, "ret");
    return netSpeed;
}

bool NetAdapter_A::send(const etl::array<uint8_t, 6> destMAC_addr,
                        const Packet packet, const bool verifyReceipt) {
    return this->send(destMAC_addr, packet.getRawPacket(),
                      packet.getLengthValid(), verifyReceipt);
}

bool NetAdapter_A::sendAll(const Packet packet) {
    return this->sendAll(packet.getRawPacket(), packet.getLengthValid());
}