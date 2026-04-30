// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

// Custom Imports
#include "NTP.hpp"
#include "../Net-Link/Packet.hpp"

// Library Imports
#include "esp_log.h"
#include <Arduino.h>
#if IMPL_USED == IMPL_SYSRTC
#    include "soc/systimer_reg.h"
#    include "soc/systimer_struct.h"
#elif IMPL_USED == IMPL_POSIX
#    include <sys/time.h>
#endif

// MARK: Public

SyncedClock::SyncedClock(NetAdapter_A &netAdapter) : netAdapter(netAdapter) {}

bool SyncedClock::initTimeSync(const etl::array<uint8_t, 6> &upstreamMAC) {
    // Get time data
    timeSyncInit_micros = micros();
    ESP_LOGV(TAG, "Time sync init at %lu micros", timeSyncInit_micros);

    // Convert to byte array
    const auto *timeSyncInit_data =
        reinterpret_cast<const uint8_t *>(&timeSyncInit_micros);
    etl::array<uint8_t, WTbNetConfig::MAX_PACKET_DATA_LENGTH> data = {
        timeSyncInit_data[0], timeSyncInit_data[1], timeSyncInit_data[2],
        timeSyncInit_data[3]};
    ESP_LOGV(TAG, "Converted time sync init to byte array");

    // Convert to packet
    auto packet =
        Packet(data, static_cast<uint8_t>(sizeof(timeSyncInit_micros)),
               Packet::PacketType::NTPRequest);
    ESP_LOGV(TAG, "Created packet");

    // Send current time
    return netAdapter.send(upstreamMAC, packet, true);
}

bool SyncedClock::requestHandler(const etl::array<uint8_t, 6> &upstreamMAC) {
    if (timeSyncInit_micros == 0) {
        // Return current time to client
        // Get time data
        const unsigned long currentTime_micros = micros();

        // Convert to byte array
        const uint8_t *currentTime_data =
            reinterpret_cast<const uint8_t *>(&currentTime_micros);
        etl::array<uint8_t, WTbNetConfig::MAX_PACKET_DATA_LENGTH> data = {
            currentTime_data[0], currentTime_data[1], currentTime_data[2],
            currentTime_data[3]};

        // Convert to packet
        auto packet =
            Packet(data, static_cast<uint8_t>(sizeof(currentTime_micros)),
                   Packet::PacketType::NTPRequest);

        return netAdapter.send(upstreamMAC, packet, true);
    }

    // Else: Complete time sync - compute correction factor and update system
    unsigned long currentTime_micros = micros();
    correctionFactor_micros = currentTime_micros - timeSyncInit_micros;
    lastSyncTime_micros = currentTime_micros;
    timeSyncInit_micros = 0;

#if IMPL_USED == IMPL_OFFSET
    // Time correction already stored
    return true;
#elif IMPL_USED == IMPL_HWTIMER
    // Update hw timer
    return true;
#elif IMPL_USED == IMPL_SYSRTC
    if (!updateSystemTimer(correctionFactor_micros)) {
        ESP_LOGE(TAG, "Failed to adjust system time");

        return false;
    }
    // Else: Success
    return true;
#elif IMPL_USED == IMPL_POSIX
    /**
     * @brief Update system time
     * @see https://www.man7.org/linux/man-pages/man3/adjtime.3.html
     * @see
     * https://stackoverflow.com/questions/1546882/how-to-convert-errno-in-unix-to-corresponding-string
     * Note: This likely does not affect micros(), which is based on the
     * system RTC
     */
    timeval delta = {0, correctionFactor_micros};
    if (adjtime(&delta, nullptr) != 0) {
        ESP_LOGE(TAG, "Failed to adjust system time:", strerror(errno));

        return false;
    }
    // Else: Success
    return true;
}
#endif // IMPL_USED
}

// MARK: Getters

unsigned long SyncedClock::getMicrosSinceSync() const {
    return micros() - lastSyncTime_micros;
};

unsigned long SyncedClock::getMicros() const {
#if IMPL_USED == IMPL_OFFSET
    return micros() + correctionFactor_micros;
#elif IMPL_USED == IMPL_HWTIMER
// todo
#elif IMPL_USED == IMPL_SYSRTC
    return micros();
#elif IMPL_USED == IMPL_POSIX
// todo
#endif // IMPL_USED
}

unsigned long SyncedClock::getCorrectionFactor() const {
    return correctionFactor_micros;
}

/**
 * @see https://stackoverflow.com/questions/3350385/how-to-return-an-object-in-c
 */
etl::string<SyncedClock::LOG_STRING_SIZE> SyncedClock::getLogString() const {
    etl::string<LOG_STRING_SIZE> logString(TAG); // 3 chars
    (void)logString.append(": lsTus: 0x");       // 11 chars

    etl::format_spec format;
    (void)format.hex().width(6).fill('0'); // [6 chars]

    etl::to_string(lastSyncTime_micros, logString, format, true); // 6 chars
    (void)logString.append(": tSIus: 0x");                        // 11 chars
    etl::to_string(timeSyncInit_micros, logString, format, true); // 6 chars
    (void)logString.append(": cFus: 0x");                         // 11 chars
    etl::to_string(correctionFactor_micros, logString, format, true); // 6 chars

    return logString;
};

// MARK: System Timer
#if IMPL_USED == IMPL_SYSRTC

/**
 * @see
 * https://github.com/espressif/esp-idf/blob/master/components/esp_timer/src/esp_timer_impl_lac.c#L95
 */
uint64_t SyncedClock::esp_timer_impl_get_counter_reg(void) const {
    uint32_t lo, hi;
    uint32_t lo_start = REG_READ(COUNT_LO_REG);
    uint32_t div = REG_GET_FIELD(CONFIG_REG, TIMG_LACT_DIVIDER);
    /* The peripheral doesn't have a bit to indicate that the update is
     * done, so we poll the lower 32 bit part of the counter until it
     * changes, or a timeout expires.
     */
    REG_WRITE(UPDATE_REG, 1);
    do {
        lo = REG_READ(COUNT_LO_REG);
    } while (lo == lo_start && div-- > 0);

    /* Since this function is called without a critical section, verify that
     * LO and HI registers are consistent. That is, if an interrupt happens
     * between reading LO and HI registers, and esp_timer_impl_get_time is
     * called from an ISR, then try to detect this by the change in LO
     * register value, and re-read both registers.
     */
    do {
        lo_start = lo;
        hi = REG_READ(COUNT_HI_REG);
        lo = REG_READ(COUNT_LO_REG);
    } while (lo != lo_start);

    timer_64b_reg_t result = {.lo = lo, .hi = hi};
    return result.val;
}

/**
 * @details 11.7.4 Update After Light-sleep (ESP32-C5 TRM pgs. 510 - 511)
 * 1. Configure RTC timer before the chip goes to Light-sleep mode, to
 * record the exact sleep time. For more information, see Chapter 2
 * Low-power Management [to be added later].
 * 2. Read the sleep time from the RTC timer when the chip wakes up from
 * Light-sleep mode.
 * 3. Read the current count value of system timer, see Section 11.7.1.
 * 4. Convert the time value recorded by RTC timer from the clock cycles
 * based on RTC_SLOW_CLK to that based on 16 MHz CNT_CLK. For example, if
 * the frequency of RTC_SLOW_CLK is 32 kHz, the recorded RTC timer value
 * should be converted by multiplying by 500.
 * 5. Add the converted RTC value to the current count value of system
 * timer:\ • Fill the new value into SYSTIMER_TIMER_UNITn_LOAD_LO (low 32
 * bits) and SYSTIMER_TIMER_UNITn_LOAD_HI (high 20 bits). • Set
 * SYSTIMER_TIMER_UNITn_LOAD to load the new timer value into the system
 * timer. By such way, the system timer is updated.
 */
bool SyncedClock::updateSystemTimer(int32_t deltaMicros) {
    /**
     * 3. Read the current count value of system timer, see Section 11.7.1.
     */
    // int64_t timerCount = getSystemTimer();
    int64_t timerCount = SyncedClock::esp_timer_impl_get_counter_reg();

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
     * based on RTC_SLOW_CLK to that based on 16 MHz CNT_CLK. For example,
     * if the frequency of RTC_SLOW_CLK is 32 kHz, the recorded RTC timer
     * value should be converted by multiplying by 500.
     *
     * | -    | Slow   | Micros | Fast   |
     * | ---  | ---    | ---    | ---    |
     * | Frq. | 32 kHz | 1 MHz  | 16 MHz |
     * | Eg.  | 1      | 31.25  | 500    |
     * | Eg2. | -      | 1      | 16     |
     */
    constexpr int32_t CNT_CLCK_PER_MICROS = 16;
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
     * • Fill the new value into SYSTIMER_TIMER_UNITn_LOAD_LO (low 32 bits)
     * and SYSTIMER_TIMER_UNITn_LOAD_HI (high 20 bits).
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
 * 1. Set SYSTIMER_TIMER_UNITn_UPDATE to fill the current count value of
 * COMPx into SYSTIMER_TIMER_UNITn_VALUE_HI and
 * SYSTIMER_TIMER_UNITn_VALUE_LO.
 * 2. Poll the reading of SYSTIMER_TIMER_UNITn_VALUE_VALID till it is 1.
 * Then, user can read the count value from SYSTIMER_TIMER_UNITn_VALUE_HI
 * and SYSTIMER_TIMER_UNITn_VALUE_LO.
 * 3. Read the lower 32 bits and higher 20 bits from
 * SYSTIMER_TIMER_UNITn_VALUE_LO and SYSTIMER_TIMER_UNITn_VALUE_HI
 * respectively.
 * @see
 * https://github.com/espressif/esp-idf/blob/master/components/esp_timer/src/esp_timer_impl_lac.c#L95
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/hardware-abstraction.html
 * TODO: Switch to official implementation
 */
int64_t SyncedClock::getSystemTimer() {
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
     * Then, user can read the count value from
     * SYSTIMER_TIMER_UNITn_VALUE_HI and SYSTIMER_TIMER_UNITn_VALUE_LO.
     */
    constexpr uint_fast32_t I_MAX = 1000;
    bool success = false;
    for (uint_fast32_t i = 0; i < I_MAX; i++) {
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
#endif // IMPL_SYSRTC
