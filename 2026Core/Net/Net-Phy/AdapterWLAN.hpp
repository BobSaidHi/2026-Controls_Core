#pragma once

// Custom Imports
#include "../NetAdapter_A.hpp"

// ESP32 Imports
#include <WiFi.h>
#include <esp_wifi.h>

/**
 * @brief Network Adapter to wrap WiFi functionality
 * @author Noah (@BobSaidHi)
 */
class AdapterWLAN : public NetAdapter_A {
  public:
    // MARK: Constants
    static constexpr const char *TAG = "AW";

    static constexpr uint8_t MAC_ADDR_STR_LEN =
        sizeof("00:00:00:00:00:00"); // I can't count --Noah

    /**
     * @see
     * https://docs.espressif.com/projects/esp-idf/en/v4.4.5/esp32/api-reference/network/esp_wifi.html#_CPPv425esp_wifi_set_max_tx_power6int8_t
     */
    enum TxDbmToESP : int_fast8_t {
        DBM02 = 8,
        DBM05 = 20,
        DBM07 = 28,
        DBM08 = 34,
        DBM11 = 44,
        DBM13 = 52,
        DBM14 = 56,
        DBM15 = 60,
        DBM16 = 66,
        DBM18 = 72,
        DBM20 = 80
    };

    /**
     * @brief Minimum RSSI that WiFI.RSSI(i) should report
     */
    static constexpr int_fast16_t ESP32_MIN_RSSI = -120;

    /**
     * @brief Maximum RSSI that WiFI.RSSI(i) should report
     */
    static constexpr int_fast16_t ESP32_MAX_RSSI = 0;

    /**
     * @brief 2.4Ghz WiFi Adjacent Channel Rejection
     * @see datasheet pgs 74 - 75
     * TODO: Get link
     */
    enum class WT32S1_ADJ_CH_REJECTION_dB : uint_fast32_t {
        OFDM_6Mbps = 37,
        OFDM_54Mbps = 21,
        HT20_MCS0 = 37,
        HT20_MCS7 = 20
    };

    /**
     * @brief 2.4Ghz WiFi Adjacent Channel Rejection (ESP32C5)
     * @see Pages 74 - 75 of the ESP32-C5 datasheet
     * @see https://documentation.espressif.com/esp32-c5_datasheet_en.html
     */
    enum class ESP32C5_2Hz4_ADJ_CH_REJ_dB : uint_fast32_t {
        WIFIB_DSSS_1Mbps = 41,
        WIFIB_CCK_11Mbps = 40,
        WIFIG_OFDM_6Mbps = 37,
        WIFIG_OFDM_54Mbps = 17,
        WIFIN_HT20_MCS0 = 34,
        WIFIN_HT20_MCS7 = 16,
        WIFIN_HT40_MCS0 = 24,
        WIFIN_HT40_MCS7 = 13,
        WIFIAX_HE20_MCS0 = 38,
        WIFIAX_HE20_MCS9 = 12
    };

    /**
     * @brief 2.4Ghz WiFi Adjacent Channel  (ESP32S3)
     * @see Page 72 of the datasheet
     * @see https://documentation.espressif.com/esp32-s3_datasheet_en.html
     */
    enum class ESP32S3_2Hz4_ADJ_CH_REJ_dB : uint_fast32_t {
        WIFIB_1Mbps = 35,
        WIFIB_11Mbps = 35,
        WIFIG_6Mbps = 31,
        WIFIG_54Mbps = 20,
        WIFIN_HT20_MCS0 = 31,
        WIFIN_HT20_MCS7 = 16,
        WIFIN_HT40_MCS0 = 25,
        WIFIN_HT40_MCS7 = 11
    };

    // MARK: Standard

    /**
     * @brief Constructor
     */
    AdapterWLAN();
    /**
     * @brief Destructor to comply with SonarLint Rule cpp:S1234: Polymorphic
     * base class destructor should be either public virtual or protected
     * non-virtual
     *
     * When a class with no virtual destructor is used as a base
     * class, surprises can occur if pointers to instances of this class are
     * used. Specifically, if an instance of a derived class is deleted through
     * a pointer to the base type, the behavior is undefined and can lead to
     * resource leaks, crashes or corrupted memory.
     *
     * If it is not expected for base class pointers to be deleted, then the
     * destructor should be made protected to avoid such a misuse.
     *
     * -- SonarLint Rule cpp:S1234: Polymorphic base class destructor should be
     * either public virtual or protected non-virtual
     */
    ~AdapterWLAN() = default; // todo implement any remaining virtual functions

    // MARK: Public Concrete

    // etl::string<len> toLogString() {
    //     return 
    // }

    /**
     * @brief Identify the optimal channel to use for communication
     * @returns the optimal standard/ friendly channel WLAN (1, 6, or 11) to use
     * or WTbNetConfig::WIFI_DEFAULT_CH on failure.
     */
    uint8_t identifyOptimalChannel();

    /**
     * @brief Get the MAC address of the network interface
     * @returns the MAC address of the network interface
     */
    etl::array<uint8_t, 6> getMACAddress() const;

    // Inherited from NetAdapter_A

    /**
     * @brief Initialize the network adapter
     */
    // bool begin() /*override*/; // todo

    /**
     * @brief Initialize the network adapter
     * @param wiFiChannel the channel to use
     */
    bool
    begin(const uint8_t wiFiChannel = (uint8_t)WTbNetConfig::WIFI_DEFAULT_CH);

    /**
     * @brief Format a MAC address as a string
     * @param rawMACAddress the MAC address to format
     * @returns the formatted MAC address
     */
    static etl::string<MAC_ADDR_STR_LEN>
    formatMACAddress(const etl::array<uint8_t, 6> rawMACAddress);

    /**
     * @brief Set the maximum transmit power
     * @param txPower the transmit power to set
     */
    bool setMaxTxPower(TxDbmToESP txPower);

    /**
     * @brief Get the maximum configured transmit power
     * @returns the maximum configured transmit power in dBm
     */
    int_fast8_t getMaxTxPower_dBm();

    // MARK: Network Overrides

    /**
     * @brief Send data
     *
     * @param dest_MAC_addr the MAC address of the destination
     * @param data the data to send
     * @param bytesValid the number of valid bytes in the data array
     * @param verifyReceipt whether to verify the data was received
     */
    bool send(const etl::array<uint8_t, 6> destMAC_addr,
              const etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
              const uint_fast8_t bytesValid, const bool verifyReceipt) override;

    /**
     * @brief Send data to all devices
     *
     * @param dest_MAC_addr the MAC address of the destination
     * @param data the data to send
     * @param bytesValid the number of valid bytes in the data array
     * @param verifyReceipt whether to verify the data was received
     */
    bool
    sendAll(const etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
            const uint_fast8_t bytesValid) override;

//   protected:
  private: // MARK: Private
    // todo track stats?

    /**
     * @brief Get the maximum configured transmit power
     * @returns the maximum configured transmit power in dBm in units of 0.25
     * dBm
     */
    int_fast8_t getMaxTxPower_Raw();
};

// ADAPTER_WLAN_HPP
