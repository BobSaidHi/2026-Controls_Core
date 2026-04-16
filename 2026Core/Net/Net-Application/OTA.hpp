#pragma once

#include <Arduino.h>
// #define CONFIG_ASYNC_TCP_STACK_SIZE 4096 // Alex, 2025-25: reduce the stack
// size (default is 16K) #include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
// #define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#include <ElegantOTA.h>

namespace OTA {
    constexpr const char *OTA_USER = "CPWPAdmin";
    constexpr const char *OTA_PASS = "CPWP2026OTA";

    constexpr const char *SSID = "CPWP2026_Service";
    constexpr const char *NET_PASS = "CPWP2026Net";

    constexpr const char *TAG = "OTA";

    constexpr uint32_t OTA_LOOP_PERIOD_MS = 100;

    AsyncWebServer server(80);

    bool setup() {
        if (WiFi.begin(SSID, NET_PASS) == WL_CONNECT_FAILED) {
            ESP_LOGE(TAG, "Failed to connect to WiFi network with SSID: %s",
                     SSID);
            return false;
        }

        // Wait for connection
        if (WiFi.status() != WL_CONNECTED) {
            delay(500);
        }
        if (WiFi.status() != WL_CONNECTED) {
            ESP_LOGE(TAG, "WiFI not connected");
        }
        ESP_LOGI(TAG, "Connected to WiFi network with SSID: %s", SSID);
        ESP_LOGI(TAG, "IP address: %s", WiFi.localIP().toString().c_str());

        // WiFi.enableLongRange(true); // todo
        // WiFi.hostname("CPWP-ESP32-OTA"); // todo

        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/plain",
                          "Hi! 2026 Nacelle Controller here.");
        });

        server.begin();

        ElegantOTA.begin(&server, OTA_USER, OTA_PASS);

        return true;
    }

    void xTaskLoop() {
        ElegantOTA.loop();
        delay(OTA_LOOP_PERIOD_MS);
    }
} // namespace OTA
