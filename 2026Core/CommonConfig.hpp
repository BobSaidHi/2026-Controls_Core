/**
 * @brief Common Configuration File
 * @author Noah (@BobSaidHi)
 */

#pragma once

/* Library Imports */
#include <cstdint>
#include <etl/array.h>

/**
 * @see @see
 * https://stackoverflow.com/questions/53164773/in-vs-code-show-readable-section-headings-titles-in-minimap@\
 */
// MARK: Board Config

// TODO: Board Config
#define WT32_ETH01 0

// TODO: Formatter config

/**
 * @brief Debugging Setup
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/log.html
 */
#pragma region Debugging Setup

#ifndef PROJECT_ID
#    define PROJECT_ID "WT26" // CONFIG - Project ID to use with logger
#endif

#pragma endregion // Debugging Setup

/**
 * @brief Common Configuration Constants
 * MARK: Common Config.
 * @see
 * https://stackoverflow.com/questions/53164773/in-vs-code-show-readable-section-headings-titles-in-minimap
 */
namespace WTbCommonConfig {
    /* Logger */
    // constexpr uint_fast8_t DEBUG_LEVEL = CT_LOG_LEVEL_MACRO; // todo
    // CONFIG - Size of string to store debug message
    constexpr uint_fast8_t MAX_MSG_SIZE = 40; // todo
    constexpr unsigned long SERIAL_BAUD = 115200; // CONFIG - Baud rate for serial communication

    /* Additional Checks*/
    constexpr uint_fast8_t TIME_TO_MICROS_ROLLOVER_MINS = UINT32_MAX / 1000000 / 60;
    constexpr uint_fast8_t TIME_TO_MILLIS_ROLLOVER_DAYS =
        UINT32_MAX / 1000 / 60 / 60 / 24;

} // end namespace WTbCommonConfig

// MARK: Constants
namespace CONSTS {
    /* Additional Checks*/
    constexpr uint_fast8_t TIME_TO_MICROS_ROLLOVER_MINS =
        UINT32_MAX / 1000000 / 60;
    constexpr uint_fast8_t TIME_TO_MILLIS_ROLLOVER_DAYS =
        UINT32_MAX / 1000 / 60 / 60 / 24;

    /* Time Constants */
    // constexpr uint32_t MILLIS_PER_SEC = 1000;
    // constexpr uint32_t SECS_PER_MIN = 60;

} // end namespace CONSTS

/**
 * TODO: Detect I2C discount, safely exit, and reconnect
 * @see
 * https://arduino.stackexchange.com/questions/80729/esp32-stack-canary-watchpoint-triggered
 */

/**
 * @brief Network Configuration Constants
 * MARK: Network Config.
 */
namespace WTbNetConfig {
    // Device Configuration
    constexpr etl::array<uint8_t, 6> LOAD_MAC = {0xFF, 0xFF, 0xFF,
                                                 0xFF, 0xFF, 0xFF}; // todo
    constexpr etl::array<uint8_t, 6> NACELLE_MAC = {0x30, 0xED, 0xA0,
                                                    0xE0, 0x6B, 0x78}; // todo

    // Packet Configuration
    // TODO - MAX_PACKET_ABS_LEN is ESP-NOW specific and should be specified
    // elsewhere
    constexpr uint_fast8_t MAX_PACKET_ABS_LEN = 250; // TODO
    constexpr uint_fast8_t PACKET_HEADER_LEN = 1;    // todo
    constexpr uint_fast8_t MAX_PACKET_DATA_LENGTH =
        MAX_PACKET_ABS_LEN - PACKET_HEADER_LEN; // todo
    constexpr uint_fast8_t RECV_QUEUE_LEN = 10; // CONFIG
    // TODO - packet format

    // Wireless Configuration
    constexpr uint_fast8_t WIFI_DEFAULT_CH = 6; // CFG - Should be 1, 6, or 11
    /**
     * In the 2.5Ghz WiFi band, only channels 1, 6, and 11 don't overlap
     * @see
     * https://en.wikipedia.org/wiki/List_of_WLAN_channels#/media/File:NonOverlappingChannels2.4GHz802.11-en.svg
     * @see https://superuser.com/a/443243
     * @see (Dissent):
     * https://superuser.com/questions/382042/why-use-wifi-channels-other-than-1-6-or-11
     */
    static_assert(WIFI_DEFAULT_CH == 1 || WIFI_DEFAULT_CH == 6 ||
                      WIFI_DEFAULT_CH == 11,
                  "WIFI_DEFAULT_CH SHOULD be 1, 6, or 11");

    /**
     * Calculate how many networks are on each channel, and their weighted,
     * combined strength Weights Weights are used to place more emphasis on the
     * RSSI value depending on the distance between the channels.
     */
    // todo - improve this
    constexpr uint_fast8_t OFFSET_0_WEIGHT = 1;
    constexpr uint_fast8_t OFFSET_1_WEIGHT = 2;
    constexpr uint_fast8_t OFFSET_2_WEIGHT = 2;
    constexpr uint_fast8_t OFFSET_3_WEIGHT = 2;
    constexpr float OFFSET_4_WEIGHT = 1.5;
} // end namespace WTbNetConfig
