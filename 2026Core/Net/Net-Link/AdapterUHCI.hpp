#pragma once

// MARK: Includes
#include <cstdint>
#include <driver/uart.h>
#include <driver/uhci.h>
#include <driver/uhci_types.h>
#include <freertos/queue.h>
#include <stdatomic.h>

// #include "uhci.h"
// #include "uhci_types.h"

/**
 * @brief Network Adapter (Wrapper) that leverages ESP32 hardware features to
 * provide features to similar to ESP-NOW over UART.
 * @details Network Adapter for the link layer Universal Asynchronous
 * Receiver/Transmitter (UART) via the ESP32's General Direct Memory Access
 * (GDMA) & Host Controller Interface (HSI) data packet encoding & error
 * correction via the Universal HCI (UHCI) peripheral.
 * @author Noah (@BobSaidHi)
 * MARK: AdapterUHCI
 */
class AdapterUHCI {
  public: // MARK: Public
    // Baked Config
    constexpr size_t MAX_DATA_LEN = 250;
    static constexpr char *TAG = "UHCI";

    /**
     * @brief Construct a new UHCI network adapter object
     * @param pins A struct/pair of integers representing the TX and RX pins
     */
    AdapterUHCI(const Pins pins);
    ~AdapterUHCI() = default;

    /**
     * @brief Structure to hold TX and RX pin numbers
     * @details This is better than an std::pair because the members have names
     * @see
     * https://stackoverflow.com/questions/612328/difference-between-struct-and-typedef-struct-in-c
     */
    typedef struct {
        int tx;
        int rx;
    } Pins;

    /**
     * @brief Complete hardware initialization of the UHCI adapter
     * @return true if initialization was successful, false otherwise
     */
    bool begin();

    bool wakeOnUART();

  private: // MARK: Private
    // MARK: Compile Time Config
    constexpr uint_fast8_t UART_PORT = UART_NUM_1; // UART port number

    /**
     * @details From example: For uart port configuration, please refer to UART
     * programming guide. Please double-check as the baud rate might be limited
     * by serial port chips.
     * @details @contactalexliu says parity bits may not be very useful and is
     * not an alternative for CRC
     * @details Software flow control appears to use special chars instead of
     * dedicated pins (?)
     * @see
     * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html#single-step
     * @see
     * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html#_CPPv413uart_config_t
     * @see
     * https://github.com/espressif/esp-idf/blob/v5.5.1/components/esp_driver_uart/include/driver/uart.h
     */
    constexpr uart_config_t UART_CFG = {
        .baud_rate = 2'000'000, // 2 Mbps
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        // .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
        // .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };

    /**
     * @see
     * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#_CPPv424uhci_controller_config_t
     * @see
     * https://github.com/espressif/esp-idf/blob/v5.5.1/components/esp_driver_uart/include/driver/uhci.h
     */
    constexpr uhci_controller_config_t UHCI_CFG = {
        .uart_port = UART_PORT,     // Connect uart port to UHCI hardware.
        .tx_trans_queue_depth = 30, // Queue depth of transaction queue.
        .max_transmit_size =
            MAX_DATA_LEN, // Maximum transfer size in one transaction, in bytes.
        .max_receive_internal_mem =
            2 * MAX_DATA_LEN, // internal memory usage, for more information,
                              // please refer to API reference.
        .dma_burst_size = 32, // Burst size.
        .max_packet_receive =
            MAX_DATA_LEN, // Max receive size, auto stop receiving after reach
                          // this value, only valid when length_eof set true
        // When to trigger a end of frame event, you can choose
        // `idle_eof`, `rx_brk_eof`, `length_eof`, for more information,
        // please refer to API reference.
        .rx_eof_flags =
            {
                .rx_brk_eof = 1,
                .idle_eof = 1,
                .length_eof = 1,
            },
    };

    // MARK: Run Time Config
    const Pins pins;

    // MARK: Working & Types

    // Working
    QueueHandle_t uHCIRxQueue;

    // Types

    /**
     * @see
     * https://github.com/espressif/esp-idf/blob/v5.5.1/examples/peripherals/uart/uart_dma_ota/main/uart_dma_ota_example_main.c
     */
    typedef enum {
        UHCI_EVT_PARTIAL_DATA,
        UHCI_EVT_EOF,
    } uhci_event_t;

    /**
     * @see https://www.reddit.com/r/embedded/comments/zn23of/comment/j0fav6o/
     * @see
     * https://stackoverflow.com/questions/63471387/should-volatile-still-be-used-for-sharing-data-with-isrs-in-modern-c
     * @see https://en.cppreference.com/w/c/language/atomic.html
     * @see https://en.cppreference.com/w/cpp/atomic/atomic.html
     * @see https://stackoverflow.com/a/16783513
     */
    static_assert(atomic<uint_fast32_t>::is_always_lock_free,
                  "Atomic operations on uint_fast32_t are not lock-free on "
                  "this platform.");
    typedef struct {
        _Atomic uint_fast32_t bits = 0;
        _Atomic uint_fast32_t packets = 0;
        _Atomic uint_fast32_t dataRate_bpuS = 0;
        uint_fast32_t lastStatUpdate_uS = 0;
    } NetStats_T;

    typedef struct {
        QueueHandle_t uhci_queue = uHCIRxQueue;
        size_t receive_size = 0;
        // NetStats_T stats;
        uint8_t *p_receive_data = nullptr;
    } RxContext_t;

    typedef struct {
        RxContext_t rx;
        NetStats_T rxStats;
        NetStats_T txStats;
    } UHCI_Context_t;

    // Working
    uhci_controller_handle_t uhci_ctrl;
    etl::array<uint8_t, MAX_DATA_LEN> rxData = {0};
    UHCI_Context_t uHCIContext;

    // MARK: Callbacks

    /**
     * @brief Callback for when a tra transmit (Tx) transaction is complete
     * @param uhci_ctrl Handle to the UHCI controller that initiated the
     * transmission.
     * @param edata Pointer to a structure containing event data related to the
     * completed transmission. This structure provides details such as the
     * number of bytes transmitted and any status information relevant to the
     * operation.
     * @param user_ctx User-defined context passed during the callback
     * registration. It can be used to maintain application-specific state or
     * data.
     * @returns Whether a high priority task has been waken up by this callback
     * function
     * @see
     * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#_CPPv423uhci_tx_done_callback_t
     **/
    bool txDoneCallback(uhci_controller_handle_t uhci_ctrl,
                        const uhci_rx_event_data_t *edata, void *user_ctx);

    /**
     * @brief Callback for when a receive (Rx) transaction event occurs (may be
     * incomplete)
     * @param uhci_ctrl Handle to the UHCI controller that initiated the
     * transmission.
     * @param edata Pointer to a structure containing event data related to the
     * completed transmission. This structure provides details such as the
     * number of bytes transmitted and any status information relevant to the
     * operation.
     * @param user_ctx User-defined context passed during the callback
     * registration. It can be used to maintain application-specific state or
     * data.
     * @returns Whether a high priority task has been waken up by this callback
     * function
     * @see
     * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#_CPPv423uhci_tx_done_callback_t
     */
    IRAM_ATTR static bool rxEventCallback(uhci_controller_handle_t uhci_ctrl,
                                          const uhci_rx_event_data_t *edata,
                                          void *user_ctx);

    /**
     * @brief Transmit data over UHCI
     * @param data the data to transmit
     * @param block whether to block until transmission is complete
     */
    void transmit(etl::array<uint8_t, 0> data, bool block = false);

    // void receive();

    // void transmitTask();

    void receiveTask();
};
