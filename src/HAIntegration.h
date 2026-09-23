#ifndef SRC_HAINTEGRATION
#define SRC_HAINTEGRATION

#include <ArduinoHA.h>

// Pressure sensor configuration.
//
// Wiring (see project_context.local.md):
//   Red (IN+)  -> VBUS (5 V)
//   Black (GND) -> GND
//   Green (OUT) -> 2 kΩ -> GP26 (ADC0) -> 1 kΩ -> AGND (1/3 divider)
#define PRESSURE_PIN        A0         // GP26 / ADC0, physical pin 31
#define PRESSURE_DIVIDER    3.0f       // R1+R2 / R2 = (2k+1k)/1k
#define PRESSURE_MIN_V      0.5f       // sensor output at 0 psi
#define PRESSURE_MAX_V      4.5f       // sensor output at 200 psi
#define PRESSURE_MAX_PSI    200.0f
#define PRESSURE_SAMPLES    16         // ADC samples averaged per reading
#define PRESSURE_INTERVAL_MS 5000UL    // publish cadence

class HAIntegration {
    public:
    void loop();
    void configure();
    static void switchHandler(bool state, HASwitch* sender);

    private:
    unsigned long lastPressurePublish = 0;  // timestamp of last pressure publish
    
    // Reads the ADC, applies the divider and converts to psi (clamped 0–200).
    float readPressurePsi();
};

#endif // SRC_HAINTEGRATION
