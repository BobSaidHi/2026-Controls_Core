// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

// MARK: Includes
// Header
#include "AdapterUHCI.hpp"

// Library Imports
#include "esp_log.h"
#include <Arduino.h>

// MARK: Constructors
/**
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32s3/api-reference/peripherals/uhci.html
 * or @see
 * https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32c5/api-reference/peripherals/uhci.html
 * @see
 * https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf
 * @see
 * https://www.espressif.com/sites/default/files/documentation/esp32-c5_technical_reference_manual_en.pdf
 * @see
 * https://github.com/espressif/esp-idf/blob/v5.5.1/examples/peripherals/uart/uart_dma_ota/main/uart_dma_ota_example_main.c
 */
AdapterUHCI::AdapterUHCI(const Pins pins) : pins(pins) {
    if (!uHCIContext.rxStats.bits.is_lock_free() ||
        !uHCIContext.rxStats.packets.is_lock_free() ||
        !uHCIContext.rxStats.dataRate_bpuS.is_lock_free() ||
        !uHCIContext.txStats.bits.is_lock_free() ||
        !uHCIContext.txStats.packets.is_lock_free() ||
        !uHCIContext.txStats.dataRate_bpuS.is_lock_free()) {
        ESP_LOGE(TAG, "Atomic operations on uint_fast32_t are not lock-free on "
                      "this platform.");
    } else {
        ESP_LOGI(TAG, "Atomic operations on uint_fast32_t are lock-free on "
                      "this platform.");
    }
    ESP_LOGI(TAG, "init");
}

// MARK: Begin
bool AdapterUHCI::begin() {
    /**
     * @brief Configure UART
     * @see
     * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html#single-step
     * @see
     * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uart.html#set-communication-pins
     */
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &UART_CFG));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, this->pins.tx, this->pins.rx,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Configure UHCI
    uhci_controller_handle_t uhciCtrl;
    ESP_ERROR_CHECK(uhci_new_controller(&UHCI_CFG, &uhciCtrl));

    ESP_LOGI(AdapterUHCI::TAG, "UHCI init @%d, tx:%d, rx:%d",
             UART_CFG.baud_rate, pins.tx, pins.rx);

    /** @brief Register Callbacks
     * @see
     * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#register-event-callbacks
     * @see
     * https://stackoverflow.com/questions/68461810/c-get-address-for-non-static-function
     */
    constexpr uhci_event_callbacks_t uHCI_Callbacks = {
        .on_rx_trans_event = &(AdapterUHCI::rxEventCallback),
        .on_tx_trans_done = &(AdapterUHCI::txDoneCallback)
        // .on_rx_trans_done = rxCallback, // Does not exist
    };

    ESP_ERROR_CHECK(
        uhci_register_event_callbacks(uhciCtrl, &uHCI_Callbacks, &uHCIContext));

    return true; // TODO - check
}

// MARK: Callbacks

/**
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#_CPPv423uhci_tx_done_callback_t
 * @see
 * https://arduino.stackexchange.com/questions/22212/using-millis-and-micros-inside-an-interrupt-routine
 */
IRAM_ATTR bool
AdapterUHCI::txDoneCallback(uhci_controller_handle_t uhci_ctrl,
                            const uhci_tx_done_event_data_t *edata,
                            void *user_ctx) {
    /**
     * @brief Get/cast user context
     * @details parameter `user_ctx` is parsed by the third parameter of
     * function`uhci_register_event_callbacks`
     */
    auto *ctx = static_cast<UHCI_Context_t *>(user_ctx);
    NetStats_T &stats = ctx->txStats;

    // Record performance metrics
    auto currentTime_uS = static_cast<uint_fast32_t>(micros());

    stats.bits += edata->sent_size;
    stats.packets++;

    uint_fast32_t deltaTime_uS = currentTime_uS - stats.lastStatUpdate_uS;
    stats.dataRate_bpuS = edata->sent_size / deltaTime_uS;
    stats.lastStatUpdate_uS = currentTime_uS;

    return false; // todo - check
}

/**
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#_CPPv423uhci_tx_done_callback_t
 * @see
 * https://arduino.stackexchange.com/questions/22212/using-millis-and-micros-inside-an-interrupt-routine
 * @see https://www.man7.org/linux/man-pages/man3/memcpy.3.html
 * @SuppressWarnings("cpp:S5008") // Does not work
 */
IRAM_ATTR bool AdapterUHCI::rxEventCallback(uhci_controller_handle_t uhci_ctrl,
                                            const uhci_rx_event_data_t *edata,
                                            void *user_ctx) { // NOSONAR
    /**
     * @brief Get/cast user context
     * @details parameter `user_ctx` is parsed by the third parameter of
     * function`uhci_register_event_callbacks`
     */
    auto *ctx = static_cast<UHCI_Context_t *>(user_ctx);
    RxContext_t *rxCtx = &ctx->rx;
    NetStats_T &stats = ctx->rxStats;

    BaseType_t xTaskWoken = 0;
    uhci_event_t evt;

    rxCtx->receive_size += edata->recv_size;
    (void)memcpy(rxCtx->p_receive_data, edata->data, edata->recv_size);
    if (edata->flags.totally_received) {
        evt = UHCI_EVT_EOF;
        // ctx->receive_size += edata->recv_size;
        // memcpy(ctx->p_receive_data, edata->data, edata->recv_size);

        // Record performance metrics
        auto currentTime_uS = static_cast<uint_fast32_t>(micros());

        stats.bits += edata->recv_size;
        stats.packets++;

        uint_fast32_t deltaTime_uS = currentTime_uS - stats.lastStatUpdate_uS;
        stats.dataRate_bpuS = edata->recv_size / deltaTime_uS;
        stats.lastStatUpdate_uS = currentTime_uS;
    } else {
        evt = UHCI_EVT_PARTIAL_DATA;
        // rxCtx->receive_size += edata->recv_size;
        // memcpy(rxCtx->p_receive_data, edata->data, edata->recv_size);
        rxCtx->p_receive_data += edata->recv_size;
    }

    xQueueSendFromISR(ctx->rx.uhci_queue, &evt,
                      &xTaskWoken); // TODO: Check return value?
    return xTaskWoken;
}

// MARK: Transmit
/**
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#initiating-uhci-transmission
 */
void AdapterUHCI::transmit(etl::array<uint8_t, 0> data, bool block) {
    // Transmit data (non-blocking)
    ESP_ERROR_CHECK(
        uhci_transmit(AdapterUHCI::uhci_ctrl, data.data(), data.size()));

    // Wait for transmission to complete if blocking is requested
    if (block) { // Wait all transaction finishes
        ESP_ERROR_CHECK(uhci_wait_all_tx_transaction_done(uhci_ctrl, -1));
    }
}

// MARK: Receive
/**
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#initiating-uhci-transmission
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#_CPPv412uhci_receive24uhci_controller_handle_tP7uint8_t6size_t
 * @see
 * https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/09-xQueueReceive
 * @see http://www.openrtos.net/Embedded-RTOS-Queues.html
 * @see
 * https://www.freertos.org/Documentation/02-Kernel/04-API-references/02-Task-control/01-vTaskDelay
 * @see https://docs.arduino.cc/language-reference/en/functions/math/min/
 */
void AdapterUHCI::receiveTask() {
    ESP_ERROR_CHECK(
        uhci_receive(this->uhci_ctrl, rxData.data(), rxData.max_size()));

    uhci_event_t evt;
    while (true) {
        // A queue in task for receiving event triggered by UHCI.
        constexpr TickType_t MAX_RX_WAIT = 1000 / portTICK_PERIOD_MS;
        if (xQueueReceive(uHCIContext.rx.uhci_queue, &evt, MAX_RX_WAIT) ==
            pdTRUE) {
            if (evt == UHCI_EVT_EOF) {
                ESP_LOGI(TAG, "Rx size: %d\n", uHCIContext.rx.receive_size);
                break;
            }

            else {
                // xQueueReceive sets the task state to blocked and does not
                // busy wait
                ESP_LOGW(TAG, "RX timeout\n");

                // Wait at least 1 tick or for 1 byte, allowing the blocking
                // receive function to wait for the rest of the message
                constexpr uint32_t SEC_PER_MS = 1'000; // Conversion Factor
                static_assert(4'000'000 * static_cast<uint64_t>(SEC_PER_MS) <
                                  UINT32_MAX,
                              "Be careful with the UHCI baud rate math!");
                constexpr uint32_t BAUD_RATE_Bpms =
                    UART_CFG.baud_rate * SEC_PER_MS /
                    (8 + 1); // Convert - 8 data bits + 1 stop bit
                // Get time to block
                constexpr uint32_t TMP_TIME_PER_BYTE_ms = 1 / BAUD_RATE_Bpms;
                constexpr uint32_t TIME_PER_BYTE_ms =
                    max(static_cast<uint32_t>(1),
                        TMP_TIME_PER_BYTE_ms); // Constrain - min is 1
                // Get time in ticks per byte
                constexpr TickType_t BAUD_tICKSpB =
                    portTICK_PERIOD_MS / BAUD_RATE_Bpms;
                constexpr TickType_t RETRY_DELAY = BAUD_tICKSpB;
                vTaskDelay(RETRY_DELAY);
            }
        }
    }
}
