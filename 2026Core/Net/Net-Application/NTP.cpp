// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

// Custom Imports
#include "NTP.hpp"
#include "../Net-Link/Packet.hpp"

// Library Imports
#include "esp_log.h"
#include "soc/systimer_reg.h"
#include "soc/systimer_struct.h"
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
        // if (adjtime(&delta, nullptr) != 0) {
        if (!updateSystemTimer(correctionFactor_micros)) {
            // ESP_LOGE(TAG, "Failed to adjust system time:", strerror(errno));
            ESP_LOGE(TAG, "Failed to adjust system time");

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
    // return micros() + correctionFactor_micros;
    return micos();
}

unsigned long SyncedClock::getCorrectionFactor() const {
    return correctionFactor_micros;
}

// MARK: System Timer

/**
 * @details 11.7.4 Update After Light-sleep (ESP32-C5 TRM pgs. 510 - 511)
 * 1. Configure RTC timer before the chip goes to Light-sleep mode, to record
 * the exact sleep time. For more information, see Chapter 2 Low-power
 * Management [to be added later].
 * 2. Read the sleep time from the RTC timer when the chip wakes up from
 * Light-sleep mode.
 * 3. Read the current count value of system timer, see Section 11.7.1.
 * 4. Convert the time value recorded by RTC timer from the clock cycles based
 * on RTC_SLOW_CLK to that based on 16 MHz CNT_CLK. For example, if the
 * frequency of RTC_SLOW_CLK is 32 kHz, the recorded RTC timer value should be
 * converted by multiplying by 500.
 * 5. Add the converted RTC value to the current count value of system timer:\
 * • Fill the new value into SYSTIMER_TIMER_UNITn_LOAD_LO (low 32 bits) and
 * SYSTIMER_TIMER_UNITn_LOAD_HI (high 20 bits).
 * • Set SYSTIMER_TIMER_UNITn_LOAD to load the new timer value into the system
 * timer. By such way, the system timer is updated.
 */
static bool SyncedClock::updateSystemTimer(int32_t deltaMicros) {
    /**
     * 3. Read the current count value of system timer, see Section 11.7.1.
     */
    // int64_t timerCount = getSystemTimer();
    int64_t timerCount = esp_timer_impl_get_counter_reg();

    if (timerCount < 0) {
        ESP_LOGE(SyncedClock::TAG, "Failed to update system timer");
        return false;
    }
    constexpr uint64_t SYS_TIMER_MAX = 0x000FFFFF'FFFFFFFF;
    constexpr uint32_t SYS_TIMER_HIGH_MASK = 0x000FFFFF;
    constexpr uint32_t SYS_TIMER_LOW_MASK = 0xFFFFFFFF;
    constexpr uint64_t SYS_TIMER_MASK =
        ((uint64_t)SYS_TIMER_HIGH_MASK << 32) | SYS_TIMER_LOW_MASK;
    static_assert(SYS_TIMER_MAX == SYS_TIMER_MASK,
                  "System timer max value mismatch");
    if (timerCount > SYS_TIMER_MAX) {
        ESP_LOGE(SyncedClock::TAG, "System timer value out of range");
        return false;
    }

    /**
     * 4. Convert the time value recorded by RTC timer from the clock cycles
     * based on RTC_SLOW_CLK to that based on 16 MHz CNT_CLK. For example, if
     * the frequency of RTC_SLOW_CLK is 32 kHz, the recorded RTC timer value
     * should be converted by multiplying by 500.
     *
     * | -    | Slow   | Micros | Fast   |
     * | ---  | ---    | ---    | ---    |
     * | Frq. | 32 kHz | 1 MHz  | 16 MHz |
     * | Eg.  | 1      | 31.25  | 500    |
     * | Eg2. | -      | 1      | 16     |
     */
    constexpr CNT_CLCK_PER_MICROS = 16;
    const int32_t deltaCntClck = deltaMicros * CNT_CLCK_PER_MICROS;

    /**
     * 5. Add the converted RTC value to the current count value of system
     * timer:
     */
    timerCount += deltaCntClck;
    timerCount &= SYS_TIMER_MASK;
    constexpr uint_fast32_t UNIT0 = 0;
    constexpr uint_fast32_t UNIT1 = 1;

    /**
     * • Fill the new value into SYSTIMER_TIMER_UNITn_LOAD_LO (low 32 bits) and
     * SYSTIMER_TIMER_UNITn_LOAD_HI (high 20 bits).
     */
    // Low 32 bits to be loaded to UNIT0
    SYSTIMER.unit_load_val->lo.val =
        (uint32_t)(timerCount & SYS_TIMER_LOW_MASK);
    // High 20 bits to be loaded to UNIT0
    SYSTIMER.unit_load_val->hi.val =
        (uint32_t)((timerCount >> 32) & SYS_TIMER_HIGH_MASK);

    /**
     * • Set SYSTIMER_TIMER_UNITn_LOAD to load the new timer value into the
     * system timer. By such way, the system timer is updated.
     */
    constexpr uint32_t RELOAD_VALUE_OF_UNITn_WT = 1;
    SYSTIMER.unit_load[UNIT0].timer_unit_load =
        RELOAD_VALUE_OF_UNITn_WT; // For UNIT0

    return true;
}

/**
 * @details 11.7.1 Read Current Count Valuep (ESP32-C5 TRM pgs. 510 - 511)
 * 1. Set SYSTIMER_TIMER_UNITn_UPDATE to fill the current count value of COMPx
 * into SYSTIMER_TIMER_UNITn_VALUE_HI and SYSTIMER_TIMER_UNITn_VALUE_LO.
 * 2. Poll the reading of SYSTIMER_TIMER_UNITn_VALUE_VALID till it is 1. Then,
 * user can read the count value from SYSTIMER_TIMER_UNITn_VALUE_HI and
 * SYSTIMER_TIMER_UNITn_VALUE_LO.
 * 3. Read the lower 32 bits and higher 20 bits from
 * SYSTIMER_TIMER_UNITn_VALUE_LO and SYSTIMER_TIMER_UNITn_VALUE_HI respectively.
 * @see
 * https://github.com/espressif/esp-idf/blob/master/components/esp_timer/src/esp_timer_impl_lac.c#L95
 * TODO: Switch to official implementation
 */
static int64_t SyncedClock::getSystemTimer() {
    /**
     * 1. Set SYSTIMER_TIMER_UNITn_UPDATE to fill the current count value of
     * COMPx into SYSTIMER_TIMER_UNITn_VALUE_HI and
     * SYSTIMER_TIMER_UNITn_VALUE_LO.
     */
    constexpr uint32_t UPDATE_TIMER_UNITn_WT = 1;
    constexpr uint_fast32_t UNIT0 = 0;
    constexpr uint_fast32_t UNIT1 = 1;
    SYSTIMER.unit_op[UNIT0].timer_unit_update =
        UPDATE_TIMER_UNITn_WT; // For UNIT0 (?) (Low + High?)
    // SYSTIMER.unit_op[UNIT1].timer_unit_update =
    //     UPDATE_TIMER_UNITn_WT; // For UNIT1 (?)

    /**
     * 2. Poll the reading of SYSTIMER_TIMER_UNITn_VALUE_VALID till it is 1.
     * Then, user can read the count value from SYSTIMER_TIMER_UNITn_VALUE_HI
     * and SYSTIMER_TIMER_UNITn_VALUE_LO.
     */
    constexpr uint_fast32_t I_MAX = 1000;
    bool success = false;
    for (uint_fast32_t i = 0; i < I_MAX, i++) {
        if (SYSTIMER.unit_op[UNIT0].timer_unit_value_valid == 1) {
            success = true;
            break;
        }
    }
    if (!success) {
        ESP_LOGE(TAG, "Failed to read system timer value: timeout");
        return -1;
    }

    /**
     * 3. Read the lower 32 bits and higher 20 bits from
     * SYSTIMER_TIMER_UNITn_VALUE_LO and SYSTIMER_TIMER_UNITn_VALUE_HI
     * respectively.
     */
    uint32_t timerLow =
        SYSTIMER.unit_val[UNIT0].lo.timer_unit_value_lo; // Low 32 bits
    uint32_t timerHigh =
        SYSTIMER.unit_val[UNIT0].hi.timer_unit_value_hi; // High 20 bits

    return ((int64_t)timerHigh << 32) | timerLow;
}
