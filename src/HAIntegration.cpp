#include "HAIntegration.h"
#include "Credentials.h"

#include <ArduinoHA.h>
#include <WiFi.h>

//Adapted via:
//  https://github.com/dawidchyrzynski/arduino-home-assistant/blob/main/examples/nano33iot/nano33iot.ino

#define LED_PIN     LED_BUILTIN

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
HASwitch led("led");
HASensor pressure("pressure");

void HAIntegration::configure() {

    //Prepare LED:

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);    

    // arduino-pico defaults to 10-bit; the conversion assumes 0-4095
    analogReadResolution(12);
    
    //Set device ID as MAC address

    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));

    //Device metadata:

    device.setName("Pico W Pressure Sensor");
    device.setSoftwareVersion("0.2");

    // Configure LED switch
    led.onCommand(switchHandler);
    led.setName("Board LED");

    // Configure pressure sensor
    pressure.setName("Pressure");
    pressure.setDeviceClass("pressure");
    pressure.setUnitOfMeasurement("psi");
    pressure.setIcon("mdi:gauge");

    Serial.print("Connecting to MQTT\n");
    
    if (mqtt.begin(MQTT_BROKER, MQTT_LOGIN, MQTT_PASSWORD) == true) {
        Serial.print("Connected to MQTT broker");
    } else {
        Serial.print("Could not connect to MQTT broker");
    }
}

void HAIntegration::switchHandler(bool state, HASwitch* sender) {
    digitalWrite(LED_PIN, (state ? HIGH : LOW));
    sender->setState(state);  // report state back to Home Assistant
}


void HAIntegration::loop() {
    mqtt.loop();

    // Publish pressure reading at fixed intervals
    unsigned long now = millis();
    if (now - lastPressurePublish >= PRESSURE_INTERVAL_MS) {
        lastPressurePublish = now;
        float psi = readPressurePsi();
        
        // Convert float to string for HASensor
        char psi_str[16];
        snprintf(psi_str, sizeof(psi_str), "%.2f", psi);
        pressure.setValue(psi_str);
        Serial.printf("Pressure: %.2f psi\n", psi);
    }
}

float HAIntegration::readPressurePsi() {
    // Read and average multiple ADC samples
    uint32_t sum = 0;
    for (int i = 0; i < PRESSURE_SAMPLES; i++) {
        sum += analogRead(PRESSURE_PIN);
    }
    uint16_t rawAdc = sum / PRESSURE_SAMPLES;

    // Convert ADC reading (0-4095 for 0-3.3V) to voltage
    float adcVoltage = rawAdc * (3.3f / 4095.0f);

    // Apply divider: actual sensor voltage = ADC voltage * divider ratio
    float sensorVoltage = adcVoltage * PRESSURE_DIVIDER;

    // Linear 0.5-4.5 V -> 0-200 psi, offset by the calibrated zero
    float psi = (sensorVoltage - PRESSURE_ZERO_V) * (PRESSURE_MAX_PSI / (PRESSURE_MAX_V - PRESSURE_MIN_V));
    Serial.printf("ADC raw=%u adcV=%.3f sensorV=%.3f unclamped=%.2f psi\n", rawAdc, adcVoltage, sensorVoltage, psi);

    // Clamp to valid range [0, 200] psi
    if (psi < 0.0f) psi = 0.0f;
    if (psi > PRESSURE_MAX_PSI) psi = PRESSURE_MAX_PSI;

    return psi;
}
