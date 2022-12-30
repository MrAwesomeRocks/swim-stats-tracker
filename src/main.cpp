#include "config.h"
#include "connections.hpp"
#include "data.hpp"
#include "mpu.hpp"
#include "server.hpp"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <pgmspace.h>
#include <WiFi.h>

// Store whether our LED is on or not so we can blink it
bool blinkState = false;

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void
print_chip_debug_info()
{
    // Chip info
    log_d("%s v%d", ESP.getChipModel(), ESP.getChipRevision());
    log_d("%u x CPU @ %lu MHz", ESP.getChipCores(), ESP.getCpuFreqMHz());
    log_d(
        "%.2f MB flash @ %lu MHz, mode: %#x", ESP.getFlashChipSize() / 1024.0 / 1024.0,
        ESP.getFlashChipSpeed() / 1000 / 1000, ESP.getFlashChipMode()
    );
    log_d("Chip ID: %llX", ESP.getEfuseMac());

    // Sketch info
    log_d("Sketch MD5: %s", ESP.getSketchMD5().c_str());
    log_d(
        "Used sketch space: %lu B/%lu B", ESP.getSketchSize(),
        ESP.getFreeSketchSpace() + ESP.getSketchSize()
    );

    // Memory info
    log_d(
        "Used heap: %lu B/%lu B", ESP.getHeapSize() - ESP.getFreeHeap(),
        ESP.getHeapSize()
    );
    log_d(
        "Used PSRAM: %lu B/%lu B", ESP.getPsramSize() - ESP.getFreePsram(),
        ESP.getPsramSize()
    );

    // Library versions
    log_d("ESP-IDF %s", ESP.getSdkVersion());
    log_d(
        "Arduino v%u.%u.%u", ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MAJOR,
        ESP_ARDUINO_VERSION_PATCH
    );
    log_d("WiFiManager %s", wifi_manager_version());
}

void
setup()
{
    // initialize serial communication
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    // Initial log
    print_chip_debug_info();

    // Connect to WiFi
    log_i("Attempting to connect to WiFi...");

    if (!wifi_connect()) {
        log_e("WiFi connection failed, rebooting in 3 seconds...");
        delay(3000);
        ESP.restart();
    }

    log_i("Wifi successfully connected!");
    wifi_print_status();

#ifdef USE_mDNS
    // Start mDNS
    log_i("Attempting to start mDNS responder");

    if (mdns_setup())
        log_i("mDNS setup successfully");
    else
        log_w("mDNS setup failed! Nice hostnames will not be available.");
#endif

    // Sync time
    log_i("Syncing time with NTP...");
    configTime(
        NTP_GMT_OFFSET_sec, NTP_DST_OFFSET_sec, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3
    );

    // Initialize LittleFS
    log_i("Mounting LittleFS...");

    if (!LittleFS.begin()) {
        log_e("Error mounting LittleFS, rebooting in 3 seconds...");
        delay(3000);
        ESP.restart();
    }

    log_i("LittleFS mounted successfully!");
    log_d("Used LittleFS: %lu B/%lu B", LittleFS.usedBytes(), LittleFS.totalBytes());

    // Setup web server
    log_i("Starting web server...");

    if (!web_server_setup()) {
        log_e("Error starting web server, rebooting in 3 seconds...");
        delay(3000);
        ESP.restart();
    }

    log_i("Web server started successfully");

#ifndef TEST_WEBSERVER
    // Configure the MPU6050
    log_i("Starting MPU6050 setup...");

    if (!mpu_setup()) {
        log_e("MPU6050 setup failed, rebooting in 3 seconds...");
        delay(3000);
        ESP.restart();
    }

    log_i("MPU6050 setup completed successfully!");
#endif

    // configure LED for output
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    log_i("Setup completed successfully!");
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void
loop()
{
    if (Serial.available()) {
        char cmd;
        switch (cmd = Serial.read()) {
            case 'c':
                log_i("Clearing WiFi settings and rebooting in 3 seconds!");
                delay(3000);
                WiFi.disconnect(true, true);
                ESP.restart();
                break;

            case 'h':
                log_i("Commands: (c)lear wifi settings, (h)elp");
                break;

            default:
                log_w("Unknown command '%c'", cmd);
                break;
        }
    }

    mpu_data_t mpu_data;

#ifdef TEST_WEBSERVER
    JsonArray accelReal = doc.createNestedArray("accel");
    accelReal.add((double)esp_random());
    accelReal.add((double)esp_random());
    accelReal.add((double)esp_random());

    JsonArray ypr = doc.createNestedArray("ypr");
    ypr.add((double)esp_random());
    ypr.add((double)esp_random());
    ypr.add((double)esp_random());

    doc["temp"] = 65.6;

    web_server_send_event("mpuData", doc);

    delay(1000);
#else
    static unsigned long poll_miss_count = 0;

    // if programming failed, don't try to do anything
    if (!dmp_ready)
        return;

    // read a packet from FIFO
    if (mpu_data_available()) { // Get the Latest packet
        /*
            YPR
        */
        mpu_get_ypr(mpu_data.ypr);

        /*
            REAL ACCELERATION (no gravity)
        */
        VectorInt16 accel_real;
        mpu_get_real_accel(&accel_real);

        // Scale in terms of m/s
        mpu_data.accel[0] = mpu_int_to_mps(accel_real.x);
        mpu_data.accel[1] = mpu_int_to_mps(accel_real.y);
        mpu_data.accel[2] = mpu_int_to_mps(accel_real.z);

        /*
            TEMPERATURE
        */
        mpu_data.temp = mpu_get_temp();

        process_measurement(mpu_data);

        static auto last_sample_time = 0;
        auto cur_time = millis();
        auto sample_time = cur_time - last_sample_time;
        last_sample_time = cur_time;

        // log_d("Sample time: %lu ms", sample_time);
        // log_d("Poll misses: %lu", poll_miss_count);

        // blink LED to indicate activity
        blinkState = !blinkState;
        digitalWrite(LED_PIN, blinkState);

        poll_miss_count = 0;

#  if MPU_SAMPLE_RATE > 70   // We overflow the FIFO buffer and need to compensate
        delay(MPU_SAMPLE_RATE - 11);
#  elif MPU_SAMPLE_RATE > 10 // Default sample rate is 10ms
        delay(MPU_SAMPLE_RATE);
#  endif
    } else {
        poll_miss_count++;
    }
#endif
}
