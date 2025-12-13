// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

// Import Header
#include "AdapterWLAN_A.hpp"

// Custom Imports
#include "../../CommonConfig.hpp" // Link the live src file, not the build file//todo

// Library Imports
#include <array>
#include <etl/format_spec.h>
#include <etl/string.h>
#include <etl/to_string.h>

// MARK: Standard

AdapterWLAN_A::AdapterWLAN_A() {
    // Initialize Logger
    ESP_LOGI(TAG, "init");
}

// MARK: Public Concrete

etl::string<AdapterWLAN_A::MAC_ADDR_STR_LEN>
AdapterWLAN_A::formatMACAddress(const etl::array<uint8_t, 6> rawMACAddress) {
    ESP_LOGV(TAG, "func. called");
    // delay(1500);
    //   Format the MAC address and log it
    etl::string<MAC_ADDR_STR_LEN> MACAddressFormatted = "";
    ESP_LOGV(TAG, "Created formatted string");
    // delay(1500);

    constexpr uint_fast8_t segmentDigits = 2;
    etl::format_spec format;
    format.hex();
    format.width(segmentDigits);
    format.fill('0');
    ESP_LOGV(TAG, "Setup format spec");
    // delay(1500);

    ESP_LOGV(TAG, "Formatting");
    // delay(1500);
    etl::string<segmentDigits> formatBufferTmp;
    MACAddressFormatted.append(
        etl::to_string(rawMACAddress.at(0), formatBufferTmp, format));
    MACAddressFormatted.append(":");
    MACAddressFormatted.append(
        etl::to_string(rawMACAddress.at(1), formatBufferTmp, format));
    MACAddressFormatted.append(":");
    MACAddressFormatted.append(
        etl::to_string(rawMACAddress.at(2), formatBufferTmp, format));
    MACAddressFormatted.append(":");
    MACAddressFormatted.append(
        etl::to_string(rawMACAddress.at(3), formatBufferTmp, format));
    MACAddressFormatted.append(":");
    MACAddressFormatted.append(
        etl::to_string(rawMACAddress.at(4), formatBufferTmp, format));
    MACAddressFormatted.append(":");
    MACAddressFormatted.append(
        etl::to_string(rawMACAddress.at(5), formatBufferTmp, format));

    ESP_LOGV(TAG, "ret");
    return MACAddressFormatted;
}

/**
 * @see https://docs.arduino.cc/libraries/wifi/
 * @see
 * https://docs.arduino.cc/retired/library-examples/wifi-library/ScanNetworks/
 * @see https://github.com/arduino-libraries/WiFi/blob/master/src/WiFi.h
 * @see https://www.data-alliance.net/blog/wifi-channels-guide
 * @see https://deepbluembedded.com/esp32-wifi-signal-strength-arduino-rssi/
 */
uint8_t AdapterWLAN_A::identifyOptimalChannel() {
    // Detect WLAN Environment
    const uint8_t networksCount = WiFi.scanNetworks();
    ESP_LOGI(TAG, "WiFI Net Cnt: %d", networksCount);
    if (networksCount == 0) {
        ESP_LOGV(TAG, "ret");
        return WTbNetConfig::WIFI_DEFAULT_CH;
    }

    // Calculate how many networks are on each channel, and their weighted,
    // combined strength Weights

    // Only bother with the standard/ friendly channels (1, 6, 11)
    uint16_t ch1RSSITotal = 0;
    uint16_t ch6RSSITotal = 0;
    uint16_t ch11RSSITotal = 0;

    // Check each network
    for (uint8_t i = 0; i < networksCount; i++) {
        // Check the channel of the network
        ESP_LOGD(TAG, "RSSI: %d", WiFi.RSSI(i));
        // Could probably ave done something clever with subtraction, and maybe
        // even a lookup table/ array for the offset weights, but this is
        // probably fine too.

        if (WiFi.RSSI(i) > ESP32_MAX_RSSI) {
            ESP_LOGE(TAG, "RSSI too high");
        } else if (WiFi.RSSI(i) < ESP32_MIN_RSSI) {
            ESP_LOGE(TAG, "RSSI too low");
        }

        // 120 + 0 (max strength) = 120
        // 120 + -120 (min strength) = 0
        // Lowest number is still the least interference but its a uint
        uint16_t inverseRSSI = (120 + WiFi.RSSI(i));
        ESP_LOGD(TAG, "invRSSI: %ds", inverseRSSI);
        if (WiFi.channel(i) == 1) { // Ch. 1
            ch1RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_0_WEIGHT);
        } else if (WiFi.channel(i) == 2) { // Ch. 2 overlaps with ch. 1 and 6
            ch1RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_1_WEIGHT);
            ch6RSSITotal +=
                (uint16_t)(inverseRSSI * WTbNetConfig::OFFSET_4_WEIGHT);
        } else if (WiFi.channel(i) == 3) { // Ch. 3 overlaps with ch. 1 and 6
            ch1RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_2_WEIGHT);
            ch6RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_3_WEIGHT);
        } else if (WiFi.channel(i) == 4) { // Ch. 4 overlaps with ch. 1 and 6
            ch1RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_3_WEIGHT);
            ch6RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_2_WEIGHT);
        } else if (WiFi.channel(i) == 5) { // Ch. 5 overlaps with ch. 1 and 6
            ch1RSSITotal +=
                (uint16_t)(inverseRSSI * WTbNetConfig::OFFSET_4_WEIGHT);
            ch6RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_1_WEIGHT);
        } else if (WiFi.channel(i) == 6) { // Ch. 6
            ch6RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_0_WEIGHT);
        } else if (WiFi.channel(i) == 7) { // Ch. 7 overlaps with ch. 6 and 11
            ch6RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_1_WEIGHT);
            ch11RSSITotal +=
                (uint16_t)(inverseRSSI * WTbNetConfig::OFFSET_4_WEIGHT);
        } else if (WiFi.channel(i) == 8) { // Ch. 8 overlaps with ch. 6 and 11
            ch6RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_2_WEIGHT);
            ch11RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_3_WEIGHT);
        } else if (WiFi.channel(i) == 9) { // Ch. 9 overlaps with ch. 6 and 11
            ch6RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_3_WEIGHT);
            ch11RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_2_WEIGHT);
        } else if (WiFi.channel(i) == 10) { // Ch. 10 overlaps with ch. 6 and 11
            ch6RSSITotal +=
                (uint16_t)(inverseRSSI * WTbNetConfig::OFFSET_4_WEIGHT);
            ch11RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_1_WEIGHT);
        } else if (WiFi.channel(i) == 11) { // Ch. 11
            ch11RSSITotal += (inverseRSSI * WTbNetConfig::OFFSET_0_WEIGHT);
        } else { // Should never run in the North America
            ESP_LOGE(TAG, "Detected Illegal WiFi Ch.");
        }
        ESP_LOGD(TAG, "ch1RSSSSubTotal: %d", ch1RSSITotal);
        ESP_LOGD(TAG, "ch6RSSSSubTotal: %d", ch6RSSITotal);
        ESP_LOGD(TAG, "ch11RSSSSubTotal: %d", ch11RSSITotal);
    }

    // Return the channel with the least interference
    if (ch1RSSITotal < ch6RSSITotal && ch1RSSITotal < ch11RSSITotal) {
        ESP_LOGI(TAG, "Optimal Ch.: 1");
        return 1;
    } else if (ch6RSSITotal < ch1RSSITotal && ch6RSSITotal < ch11RSSITotal) {
        ESP_LOGI(TAG, "Optimal Ch.: 6");
        return 6;
    } else if (ch11RSSITotal < ch1RSSITotal && ch11RSSITotal < ch6RSSITotal) {
        ESP_LOGI(TAG, "Optimal Ch.: 11");
        return 11;
    } else {
        ESP_LOGE(TAG, "Failed to compute, default Ch. 6");
        return WTbNetConfig::WIFI_DEFAULT_CH;
    }
}

// Inherited from NetAdapter_A

/**
 * @see https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/
 * @see https://cplusplus.com/forum/beginner/200574/
 * @see https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/#4
 * @see https://www.geeksforgeeks.org/cpp-20-std-format/
 * @see https://stackoverflow.com/a/69326849
 */
etl::array<uint8_t, 6> AdapterWLAN_A::getMACAddress() const {
    ESP_LOGV(TAG, "func called");
    // delay(1000);

    // Save WiFi configuration
    bool leaveWifiOn = false;
    wl_status_t WiFIStatus = WiFi.status();

    if (WiFIStatus == WL_CONNECTED) {
        ESP_LOGD(TAG, "WiFi already connected");
        leaveWifiOn = true;
    } else if (WiFIStatus == WL_NO_SSID_AVAIL) {
        ESP_LOGD(TAG, "No SSID Avail");
        leaveWifiOn = true;
    } else if (WiFIStatus == WL_SCAN_COMPLETED) {
        ESP_LOGD(TAG, "WiFi Scan Completed");
        leaveWifiOn = true;
    } else if (WiFIStatus == WL_IDLE_STATUS) {
        ESP_LOGW(TAG, "WiFi Idle");
        leaveWifiOn = true;
    } else if (WiFIStatus == WL_STOPPED) {
        ESP_LOGW(TAG, "Restarting WiFi1");
        leaveWifiOn = false;

        // Configure WiFi
        WiFi.mode(WIFI_STA);
        WiFi.STA.begin();
    } else if (WiFIStatus == WL_CONNECT_FAILED) {
        ESP_LOGW(TAG, "Restarting WiFi2");
        leaveWifiOn = false;

        // Configure WiFi
        WiFi.mode(WIFI_STA);
        WiFi.STA.begin();
    } else if (WiFIStatus == WL_CONNECTION_LOST) {
        ESP_LOGW(TAG, "Restarting WiFi3");
        leaveWifiOn = false;

        // Configure WiFi
        WiFi.mode(WIFI_STA);
        WiFi.STA.begin();
    } else if (WiFIStatus == WL_DISCONNECTED) {
        leaveWifiOn = false;

        ESP_LOGW(TAG, "Starting WiFi4");
        // Configure WiFi
        WiFi.mode(WIFI_STA);
        ESP_LOGV(TAG, "WiFi mode set");
        WiFi.STA.begin();
        ESP_LOGV(TAG, "WiFi STA begin");
    } else {
        ESP_LOGE(TAG, "Invalid WiFi Status");
        return {0};
    }
    ESP_LOGV(TAG, "WiFi Status Check Done");
    // delay(1000);

    // Create a temporary c style array to hold the MAC address
    // NOSONAR
    uint8_t MACAddressTemp[6] = {0};
    esp_err_t opStatus = esp_wifi_get_mac(WIFI_IF_STA, MACAddressTemp);
    ESP_LOGD(TAG, "Queried MACAddr");
    // delay(1000);

    // Cleanup WiFi Config Changes
    if (!leaveWifiOn) {
        ESP_LOGV(TAG, "WiFi dis");
        WiFi.disconnect();
    }

    // Check if the operation was successful
    if (opStatus == ESP_OK) {
        ESP_LOGV(TAG, "Got MACAddr");
    } else if (opStatus == ESP_ERR_WIFI_NOT_INIT) {
        ESP_LOGE(TAG, "Failed to get MACAddr: WiFi not init");
        return {0};
    } else if (opStatus == ESP_ERR_INVALID_ARG) {
        ESP_LOGE(TAG, "Failed to get MACAddr: Invalid args");
        return {0};
    } else if (opStatus == ESP_ERR_WIFI_IF) {
        ESP_LOGE(TAG, "Failed to get MACAddr: Invalid interface");
        return {0};
    } else if (opStatus == ESP_ERR_WIFI_MAC) {
        ESP_LOGE(TAG, "Failed to get MACAddr: Invalid MAC Addr");
        return {0};
    } else if (opStatus == ESP_ERR_WIFI_MODE) {
        ESP_LOGE(TAG, "Failed to get MACAddr: Invalid WiFi mode");
        return {0};
    } else {
        ESP_LOGE(TAG, "Failed to get MAC address: ERR %d", opStatus);
        return {0};
    }
    ESP_LOGV(TAG, "Checked opStatus");
    // delay(1000);

    // Success
    // todo: verify performance

    // Create an array to return the MAC address
    etl::array<uint8_t, 6> MACAddress = {0};
    std::copy(std::begin(MACAddressTemp), std::end(MACAddressTemp),
              MACAddress.begin());
    ESP_LOGD(TAG, "MACAddr[0]: ", MACAddress.at(0));
    ESP_LOGD(TAG, "MACAddr[1]: ", MACAddress.at(1));
    ESP_LOGD(TAG, "MACAddr[2]: ", MACAddress.at(2));
    ESP_LOGD(TAG, "MACAddr[3]: ", MACAddress.at(3));
    ESP_LOGD(TAG, "MACAddr[4]: ", MACAddress.at(4));
    ESP_LOGD(TAG, "MACAddr[5]: ", MACAddress.at(5));

    // Format and log the MAC address
    ESP_LOGV(TAG, "Formatting MACAddr");
    // delay(1500);
    etl::string<sizeof("MACADDR: ") + MAC_ADDR_STR_LEN> MACAddressFormatted =
        "MACADDR: ";
    ESP_LOGV(TAG, "Created MACAddr string");
    // delay(1500);
    MACAddressFormatted.append(NetAdapter_A::formatMACAddress(MACAddress));
    ESP_LOGV(TAG, "Passing formatted MACAddr w/ c_str():");
    // delay(1500);
    ESP_LOGI(TAG, MACAddressFormatted);
    // ESP_LOGV(TAG, "Passed formatted MACAddr w/
    // data():"); ESP_LOGI(TAG,
    // MACAddressFormatted.data()); // still does not work as expected
    ESP_LOGD(TAG, "MACAddrFormatted[0]: ", MACAddressFormatted.at(0));
    ESP_LOGD(TAG, "MACAddrFormatted[1]: ", MACAddressFormatted.at(1));
    ESP_LOGD(TAG, "MACAddrFormatted[2]: ", MACAddressFormatted.at(2));
    ESP_LOGD(TAG, "MACAddrFormatted[3]: ", MACAddressFormatted.at(3));
    ESP_LOGD(TAG, "MACAddrFormatted[4]: ", MACAddressFormatted.at(4));
    ESP_LOGD(TAG, "MACAddrFormatted[5]: ", MACAddressFormatted.at(5));
    ESP_LOGD(TAG, "MACAddrFormatted[6]: ", MACAddressFormatted.at(6));
    ESP_LOGD(TAG, "MACAddrFormatted[7]: ", MACAddressFormatted.at(7));
    ESP_LOGD(TAG, "MACAddrFormatted[8]: ", MACAddressFormatted.at(8));
    ESP_LOGD(TAG, "MACAddrFormatted[9]: ", MACAddressFormatted.at(9));
    ESP_LOGD(TAG, "MACAddrFormatted[10]: ", MACAddressFormatted.at(10));
    ESP_LOGD(TAG, "MACAddrFormatted[11]: ", MACAddressFormatted.at(11));
    ESP_LOGD(TAG, "MACAddrFormatted[12]: ", MACAddressFormatted.at(12));
    ESP_LOGD(TAG, "MACAddrFormatted[13]: ", MACAddressFormatted.at(13));
    ESP_LOGD(TAG, "MACAddrFormatted[14]: ", MACAddressFormatted.at(14));
    ESP_LOGD(TAG, "MACAddrFormatted[15]: ", MACAddressFormatted.at(15));
    ESP_LOGD(TAG, "MACAddrFormatted[16]: ", MACAddressFormatted.at(16));
    ESP_LOGD(TAG, "MACAddrFormatted[17]: ", MACAddressFormatted.at(17));
    ESP_LOGD(TAG, "MACAddrFormatted[18]: ", MACAddressFormatted.at(18));
    ESP_LOGD(TAG, "MACAddrFormatted[19]: ", MACAddressFormatted.at(19));
    ESP_LOGD(TAG, "MACAddrFormatted[20]: ", MACAddressFormatted.at(20));
    ESP_LOGD(TAG, "MACAddrFormatted[21]: ", MACAddressFormatted.at(21));
    ESP_LOGD(TAG, "MACAddrFormatted[22]: ", MACAddressFormatted.at(22));
    ESP_LOGD(TAG, "MACAddrFormatted[23]: ", MACAddressFormatted.at(23));
    ESP_LOGD(TAG, "MACAddrFormatted[24]: ", MACAddressFormatted.at(24));
    ESP_LOGD(TAG, sessFormatted.at(25));
    ESP_LOGD(TAG, "MACAddrFormatted[26]: ", MACAddressFormatted.at(26));

    // Return the numerical MAC address
    ESP_LOGV(TAG, "ret");
    return MACAddress;
}

bool AdapterWLAN_A::begin() {
    ESP_LOGV(TAG, "init");
    return AdapterWLAN_A::begin(WTbNetConfig::WIFI_DEFAULT_CH);
}

bool AdapterWLAN_A::begin(const uint8_t wiFiChannel) {
    // Set device as a Wi-Fi Station
    bool result0 = WiFi.mode(WIFI_STA);
    int result1 = WiFi.setChannel(wiFiChannel);

    bool resultFinal = result0 && (result1 == 0);
    if (resultFinal) {
        // ESP_LOGI(TAG, "Started WiFi on ch. ",
        // //todo
        //              wiFiChannel);
    } else {
        // logger->error(FullyQualifiedComponentID, "Failed to start WiFI on ch.
        // ", //todo
        //               wiFiChannel);
    }
    return resultFinal;
}

bool AdapterWLAN_A::setMaxTxPower(TxDbmToESP txPower) {
    // Set the maximum transmit power
    if (esp_wifi_set_max_tx_power((uint8_t)txPower) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set max tx power");
        return false;
    }

    // Verify the transmit power was set correctly
    ESP_LOGI(TAG, "Set max tx power to ", getMaxTxPower());
    if (getMaxTxPower_Raw() != txPower) {
        ESP_LOGE(TAG, "Tx max power verification failed");
        return false;
    }

    // Success
    return true;
}

int_fast8_t AdapterWLAN_A::getMaxTxPower_Raw() {
    int8_t txPower = 0;

    // Check and test for failure
    if (esp_wifi_get_max_tx_power(&txPower) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get max tx power");
        return -1;
    } else if (txPower < 0 || txPower > 80) {
        ESP_LOGE(TAG, "Tx power out of range: ", txPower);
    }

    // Return result
    ESP_LOGD(TAG, "Tx power ", txPower);
    return txPower;
}

// MARK: Network Overrides

bool AdapterWLAN_A::send(
    const etl::array<uint8_t, 6> destMAC_addr,
    const etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
    const uint_fast8_t bytesValid, const bool verifyReceipt) {
    ESP_LOGE(TAG, "AdapterWLAN_A::send not implemented");
    return false;
}

bool AdapterWLAN_A::sendAll(
    const etl::array<uint8_t, WTbNetConfig::MAX_PACKET_ABS_LEN> &data,
    const uint_fast8_t bytesValid) {
    ESP_LOGE(TAG, "AdapterWLAN_A::sendAll not implemented");
    return false;
}

// MARK: Private
// todo: Sometimes this gets printed in terms of ESP units instead of dbm
int_fast8_t AdapterWLAN_A::getMaxTxPower_dBm() {
    const int_fast8_t txPower = getMaxTxPower_Raw();
    ESP_LOGD(TAG, "Tx power (dBm): ", txPower);
    return (int_fast8_t)(txPower * 0.25);
}

// End of file AdapterWLAN_A.cpp
