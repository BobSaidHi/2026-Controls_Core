/**
 * @brief Common Configuration File
 * @author Noah (@BobSaidHi)
 */

#pragma once

/* Library Imports */
#include <cstdint>
#include <etl/array.h>

// MARK: Board Config
// Make sure the hardware pins are imported
#include <pins_arduino.h>

// TODO: Board Config
#define WT32_ETH01 0
#define ESP32C5 1
#define ESP32S3 2

#define BOARD WT32_ETH01

#if BOARD == ESP32C5
#    warning "Using Nacelle Board"
#elif BOARD == ESP32S3
#    warning "Using Load Board"
// #elif BOARD == WT32_ETH01
// #    warning "Not using production board"
#else
#   warning "Not using production board"
#endif // BOARD

// TODO: Formatter config

// Debugging Setup -  Compile Time Log Level
/**
 * @see
 * https://stackoverflow.com/questions/53164773/in-vs-code-show-readable-section-headings-titles-in-minimap@\
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/log.html
 */
#pragma region Debugging Setup
// WONTFIX - Maybe create a separate logger constants file if needed.
// todo
// #define CT_LOG_LEVEL_MACRO 20 // CONFIG - Compile time log level
// static_assert(
//     CT_LOG_LEVEL_MACRO >= 0,
//     "CT_LOG_LEVEL_MACRO must be an integer greater than or equal to 0");
// static_assert(
//     CT_LOG_LEVEL_MACRO <= 100,
//     "CT_LOG_LEVEL_MACRO must be an integer less than or equal to 100");

// #if CT_LOG_LEVEL_MACRO > 5
// #define NDEBUG // Disable runtime Assertions
// #endif

#define PROJECT_ID "WT26" // CONFIG - Project ID to use with logger

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

    /* Additional Checks*/
    constexpr uint_fast8_t TIME_TO_MICROS_ROLLOVER_MINS =
        UINT32_MAX / 1000000 / 60;
    constexpr uint_fast8_t TIME_TO_MILLIS_ROLLOVER_DAYS =
        UINT32_MAX / 1000 / 60 / 60 / 24;

/* Standardized Pins - I2C */
#if USE_WT32_ETH01_PINS
    constexpr uint_fast8_t I2C_SDA_PIN = IO5;  // todo
    constexpr uint_fast8_t I2C_SCL_PIN = IO17; // todo
#else
    constexpr uint_fast8_t I2C_SDA_PIN = 23; // todo
    constexpr uint_fast8_t I2C_SCL_PIN = 22; // todo
#endif

/* Standardized Pins - Storage (SPI) */
#if USE_WT32_ETH01_PINS                         // todo
    constexpr uint_fast8_t SPI_MISO_PIN = IO14; // todo
    constexpr uint_fast8_t SPI_CLK_PIN = IO12;  // todo
    constexpr uint_fast8_t SPI_MOSI_PIN = IO4;  // todo
    constexpr uint_fast8_t SPI_CS_PIN = IO2;    // todo
#else                                           // todo
    constexpr uint_fast8_t SPI_MISO_PIN = MISO; // todo
    constexpr uint_fast8_t SPI_CLK_PIN = SCK;   // todo
    constexpr uint_fast8_t SPI_MOSI_PIN = MOSI; // todo
    constexpr uint_fast8_t SPI_CS_PIN = SS;     // todo
#endif

} // end namespace WTbCommonConfig

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
    constexpr etl::array<uint8_t, 6> LOAD_MAC = {0xFF, 0xFF, 0xFF, 0xFF,
                                                 0xFF}; // todo
    constexpr etl::array<uint8_t, 6> NACELLE_MAC = {0xFF, 0xFF, 0xFF, 0xFF,
                                                    0xFF}; // todo

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
