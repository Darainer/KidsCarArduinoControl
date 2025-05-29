// -----------------------------------------------------------------------------
// FourWD.h   —   Public interface for the 4‑wheel‑drive helper library
// -----------------------------------------------------------------------------
//  • Reads a Hall‑effect / pot throttle (0‑5 V) on an analog pin.
//  • Reads a 3‑position toggle (FAST / SLOW / REVERSE) on two digital pins.
//  • Drives two PWM pins (RPWM / LPWM) that fan out to four BTS7960 boards.
//  • Applies a slew‑rate limiter (“ramp”) and speed caps for smooth control.
//  • Can stream live debug once you compile with ‑DDEBUG.
// -----------------------------------------------------------------------------
#ifndef FOURWD_H
#define FOURWD_H

#include <Arduino.h>

/**
 * @class FourWD
 * @brief Single‑class wrapper that hides all the boring I/O glue so your
 *        sketch stays tiny: just call `begin()` once, then `poll()` in loop().
 */
class FourWD {
public:
    //--------------------------------------------------------------------
    // Constructor — change pins if your wiring differs from the diagram.
    //--------------------------------------------------------------------
    FourWD(uint8_t rpwmPin   = 5,   ///< RPWM → BTS7960 forward input
           uint8_t lpwmPin   = 6,   ///< LPWM → BTS7960 reverse input
           uint8_t fastSwPin = 3,   ///< FAST switch (to GND when active)
           uint8_t revSwPin  = 4,   ///< REVERSE switch (to GND when active)
           uint8_t thrPin    = A0   ///< 0‑5 V throttle wiper
    );

    //--------------------------------------------------------------------
    // Life‑cycle
    //--------------------------------------------------------------------
    void begin();   ///< Must be called from `setup()`
    void poll();    ///< Call as often as possible from `loop()`

    //--------------------------------------------------------------------
    // Run‑time tuning helpers (serial or GUI can call these live)
    //--------------------------------------------------------------------
    void setDeadband(uint16_t adcCounts); ///< Ignore throttle noise below this ADC value (default = 30)
    void setRampStep(float step);         ///< PWM Δ per `poll()` (default = 3)
    void setSlowPct(uint8_t pct);         ///< % cap when switch is in SLOW (default = 50)
    void setRevPct(uint8_t pct);          ///< % cap in REVERSE           (default = 30)

    //--------------------------------------------------------------------
    // Debug / telemetry getters (read‑only)
    //--------------------------------------------------------------------
    uint16_t currentThrottleRaw() const { return _lastAdc;  } ///< Most recent raw ADC (0–1023)
    uint8_t  currentPwm()         const { return _currentPWM; } ///< Current PWM duty (0–255)

private:
    // -----------------------------------------------------------------
    // Immutable wiring (set once in the constructor)
    // -----------------------------------------------------------------
    const uint8_t _rpwmPin, _lpwmPin, _fastSwPin, _revSwPin, _thrPin;

    // -----------------------------------------------------------------
    // Tunable parameters (may be changed at run‑time)
    // -----------------------------------------------------------------
    uint16_t _deadBand = 30;   ///< ADC counts treated as “no pedal”
    float    _rampStep = 3.0;  ///< ΔPWM per poll() call
    uint8_t  _slowPct  = 50;   ///< Speed cap in SLOW mode  (percent)
    uint8_t  _revPct   = 30;   ///< Speed cap in REVERSE    (percent)

    // -----------------------------------------------------------------
    // Real‑time state variables (updated every poll)
    // -----------------------------------------------------------------
    uint16_t _lastAdc    = 0;   ///< Raw throttle reading saved for debug
    float    _targetPWM  = 0.0f;///< Desired PWM after mapping & caps
    float    _currentPWM = 0.0f;///< Actual PWM being output (after ramp)

    // -----------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------
    void _applyRamp();          ///< Slew‑rate‑limit _currentPWM → motors
};

#endif // FOURWD_H
