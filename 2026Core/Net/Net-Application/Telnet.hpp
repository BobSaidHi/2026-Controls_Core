/**
 * @file Telnet.hpp
 */
#include "../Net-Phy/AdapterWLAN.hpp"
#include <Arduino.h>
#include <ESPTelnet.h>
#include <EscapeCodes.h>
#include <NacelleConfig.hpp>

namespace TELENT {
    // MARK: Constants
    static constexpr const char *TAG = "NTN";
    static constexpr const char *LINE =
        "================================================================";

    // MARK: Objects
    ESPTelnet telnet;
    IPAddress ip;
    EscapeCodes ansi;

    // TODO: WIFI - switch to net adapter

    bool isConnected() { return (WiFi.status() == WL_CONNECTED); }

    bool connectToWiFi(const char *ssid, const char *password,
                       int max_tries = 20, int pause = 500) {
        int i = 0;
        WiFi.mode(WIFI_STA);

        WiFi.begin(ssid, password);
        do {
            delay(pause);
            Serial.print(".");
        } while (!isConnected() || i++ < max_tries);
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        return isConnected();
    }

    // MARK: Setup
    bool setupTelnet() {
        // passing on functions for various telnet events
        telnet.onConnect(onTelnetConnect);
        telnet.onConnectionAttempt(onTelnetConnectionAttempt);
        telnet.onReconnect(onTelnetReconnect);
        telnet.onDisconnect(onTelnetDisconnect);
        telnet.onInputReceived(onTelnetInput);

        Serial.print("- Telnet: ");
        if (telnet.begin(TELNET::PORT)) {
            ESP_LOGI(TAG, "Telnet server started");
        } else {
            ESP_LOGE(TAG, "Failed to start Telnet server");
            return false;
        }
    }

    // MARK: Callbacks
    // (optional) callback functions for telnet events
    void onTelnetConnect(String ip) {
        ESP_LOGI(TAG, "Telnet client connected: %s", ip.c_str());

        telnet.print(ansi.setFG(ANSI_BRIGHT_WHITE));
        telnet.print(LINE);
        telnet.println("\nWelcome " + telnet.getIP());
        telnet.println("(Use ^] + q  to disconnect.)");
        telnet.print(ansi.reset());
    }

    void onTelnetDisconnect(String ip) {
        ESP_LOGI(TAG, "Telnet client disconnected: %s", ip.c_str());
    }

    void onTelnetReconnect(String ip) {
        Serial.print("- Telnet: ");
        Serial.print(ip);
        Serial.println(" reconnected");
    }

    void onTelnetConnectionAttempt(String ip) {
        Serial.print("- Telnet: ");
        Serial.print(ip);
        Serial.println(" tried to connected");
    }

    void onTelnetInput(String str) {
        // checks for a certain command
        if (str == "ping") {
            telnet.print(ansi.setFG(ANSI_BRIGHT_WHITE));
            telnet.println("> pong");
            telnet.print(ansi.reset());
            Serial.println("- Telnet: pong");
            // disconnect the client
        } else if (str == "bye") {
            telnet.print(ansi.setFG(ANSI_BRIGHT_WHITE));
            telnet.println("> disconnecting you...");
            telnet.print(ansi.reset());

            telnet.disconnectClient();
        }
    }

    // MARK: Setup
    void setup() {
        Serial.print("- Wifi: ");
        connectToWiFi(WIFI_SSID, WIFI_PASSWORD);

        if (isConnected()) {
            ip = WiFi.localIP();
            Serial.println();
            Serial.print("- Telnet: ");
            Serial.print(ip);
            Serial.print(":");
            Serial.println(port);
            setupTelnet();
        } else {
            Serial.println();
            errorMsg("Error connecting to WiFi");
        }
    }

    void loop() {
        telnet.loop();

        // send serial input to telnet as output
        if (Serial.available()) {
            telnet.print(Serial.read());
        }
    }

} // namespace TELENT