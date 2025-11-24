#include "AdapterUHCI.hpp"

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

AdapterUHCI::AdapterUHCI(const Pins pins) : pins(pins) {}

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
    uhci_controller_handle_t uhci_ctrl;
    ESP_ERROR_CHECK(uhci_new_controller(&UHCI_CFG, &uhci_ctrl));

    ESP_LOGI(AdapterUHCI::TAG, "UHCI init @%d, tx:%d, rx:%d",
             UART_CFG.baud_rate, pins.tx, pins.rx);

    /** @brief Register Callbacks
     * @see
     * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#register-event-callbacks
     */
    constexpr uhci_event_callbacks_t uHCI_Callbacks = {
        .on_tx_trans_done = AdapterUHCI::txDoneCallback,
        .on_rx_trans_event = AdapterUHCI::rxEventCallback,
        // .on_rx_trans_done = rxCallback, // Does not exist
    };

    ESP_ERROR_CHECK(
        uhci_register_event_callbacks(uhci_ctrl, &uHCI_Callbacks, ctx));
}

/**
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#_CPPv423uhci_tx_done_callback_t
 * @see
 * https://arduino.stackexchange.com/questions/22212/using-millis-and-micros-inside-an-interrupt-routine
 */
void AdapterUHCI::txDoneCallback(uhci_controller_handle_t uhci_ctrl,
                                 const uhci_rx_event_data_t *edata,
                                 void *user_ctx) {
    // Record performance metrics
    static uint_fast32_t lastStatUpdate_uS = 0;
    uint_fast32_t currentTime_uS = (uint_fast32_t)micros();

    this->txBits += edata->size;
    this->txPackets++;

    uint_fast32_t deltaTime_uS = currentTime_uS - lastStatUpdate_uS;
    this->txDataRate_bpuS = edata->size / deltaTime_uS;

    lastStatUpdate_uS = currentTime_uS;
}

/**
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/uhci.html#_CPPv423uhci_tx_done_callback_t
 * @see
 * https://arduino.stackexchange.com/questions/22212/using-millis-and-micros-inside-an-interrupt-routine
 */
bool AdapterUHCI::rxEventCallback(uhci_controller_handle_t uhci_ctrl,
                                  const uhci_rx_event_data_t *edata,
                                  void *user_ctx) {

    /**
     * @brief Cast user context
     * @details parameter `user_ctx` is parsed by the third parameter of
     * function`uhci_register_event_callbacks`
     */
    uhci_context_t *ctx = (uhci_context_t *)user_ctx;

    BaseType_t xTaskWoken = 0;
    uhci_event_t evt = 0;

    ctx->receive_size += edata->recv_size;
    memcpy(ctx->p_receive_data, edata->data, edata->recv_size);
    if (edata->flags.totally_received) {
        evt = UHCI_EVT_EOF;
        // ctx->receive_size += edata->recv_size;
        // memcpy(ctx->p_receive_data, edata->data, edata->recv_size);

        // Record performance metrics
        static uint_fast32_t lastStatUpdate_uS = 0;
        uint_fast32_t currentTime_uS = (uint_fast32_t)micros();

        this->rxBits += ctx->receive_size;
        this->rxPackets++;

        uint_fast32_t deltaTime_uS = currentTime_uS - lastStatUpdate_uS;
        this->rxDataRate_bpuS = edata->size / deltaTime_uS;

        lastStatUpdate_uS = currentTime_uS;
    } else {
        evt = UHCI_EVT_PARTIAL_DATA;
        // ctx->receive_size += edata->recv_size;
        // memcpy(ctx->p_receive_data, edata->data, edata->recv_size);
        ctx->p_receive_data += edata->recv_size;
    }

    xQueueSendFromISR(ctx->uhci_queue, &evt, &xTaskWoken);
    return xTaskWoken;
}

/**
 * @see
 * https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32s3/api-reference/peripherals/uhci.html#initiating-uhci-transmissionf
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
