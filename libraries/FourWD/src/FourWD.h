#ifndef FOURWD_H
#define FOURWD_H

#include <Arduino.h>

/**
 * @brief Helper class for a 4‑wheel drive kids car using four BTS7960 H‑bridges.
 *
 *  ▸ Reads a 0‑5 V throttle on an analog pin.
 *  ▸ Reads a 3‑position toggle: FAST, SLOW, REVERSE (uses INPUT_PULLUP).
 *  ▸ Drives two PWM pins: RPWM (forward) and LPWM (reverse).
 *  ▸ Internally ramps PWM for soft start/stop and enforces speed caps.
 */
class FourWD {
public:
    //--------------------------------------------------------------------
    // Constructor — all pins default to the schematic you supplied.
    //--------------------------------------------------------------------
    FourWD(uint8_t rpwmPin  = 5,   ///< RPWM → BTS7960 forward input
           uint8_t lpwmPin  = 6,   ///< LPWM → BTS7960 reverse input
           uint8_t fastSwPin = 3,  ///< FAST switch position (to GND when active)
           uint8_t revSwPin  = 4,  ///< REVERSE switch position (to GND when active)
           uint8_t thrPin    = A0  ///< 0‑5 V throttle pedal
           );

    //--------------------------------------------------------------------
    // Life‑cycle
    //--------------------------------------------------------------------
    void begin();   ///< Call once from setup()
    void poll();    ///< Call every loop() iteration

    //--------------------------------------------------------------------
    // Optional run‑time tuning helpers
    //--------------------------------------------------------------------
    void setDeadband(uint16_t adcCounts); ///< Ignore pedal noise below this ADC value (default = 30)
    void setRampStep(float step);         ///< ΔPWM per poll() (default = 3)
    void setSlowPct(uint8_t pct);         ///< % cap when switch is in SLOW (default = 50)
    void setRevPct(uint8_t pct);          ///< % cap in REVERSE           (default = 30)

private:
    // Pin mapping (fixed per instance)
    const uint8_t _rpwmPin, _lpwmPin, _fastSwPin, _revSwPin, _thrPin;

    // Tunables
    uint16_t _deadBand = 30;   // ADC counts treated as zero
    float    _rampStep = 3.0;  // PWM counts per poll()
    uint8_t  _slowPct  = 50;   // % ceiling in slow mode
    uint8_t  _revPct   = 30;   // % ceiling in reverse

    // State variables
    float _targetPWM  = 0.0f;
    float _currentPWM = 0.0f;

    // Helper
    void _applyRamp();
};

#endif  // FOURWD_H
