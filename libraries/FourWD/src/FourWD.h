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
    void setDeadband(uint16_t adcCounts); ///< Ignore throttle noise below this ADC value (default = 200)
    void setLowerThrottleCap(uint8_t lower); ///< Min ADC reading (default = 230)
    void setUpperThrottleCap(uint8_t upper); ///< Max ADC reading (default = 800)
    void setRampStep(float step);         ///< PWM Δ per `poll()` (default = 3)
    void setBrakeRampStep(float step); ///< PWM Δ per `poll()` when ramping down (default = 3)
    void setSlowPct(uint8_t pct);         ///< % cap when switch is in SLOW (default = 50)
    void setRevPct(uint8_t pct);          ///< % cap in REVERSE           (default = 30)

    //--------------------------------------------------------------------
    // Debug / telemetry getters (read‑only)
    //--------------------------------------------------------------------
    uint16_t currentThrottleRaw() const { return _lastAdc;  } ///< Most recent raw ADC (0–1023)
    uint8_t  currentPwm()         const { return _currentPWM; } ///< Current PWM duty (0–255)
    uint8_t  currentTargetPwm()   const { return _targetPWM;  } ///< Target PWM after mapping & caps
    uint8_t  currentMappedThrottle() const { return _thrMapped;  } ///< Mapped throttle (0–255) after deadband, mapping, and speed cap
    uint8_t  isFastMode()         const { return _isFastMode; } ///< Is FAST mode active? (HIGH = fast, LOW = slow)
    uint8_t  isReverse()         const { return _isReverse; } ///< Is REVERSE mode active? (HIGH = reverse, LOW = forward)

private:
    // -----------------------------------------------------------------
    // Immutable wiring (set once in the constructor)
    // -----------------------------------------------------------------
    const uint8_t _rpwmPin, _lpwmPin, _fastSwPin, _revSwPin, _thrPin;

    // -----------------------------------------------------------------
    // Tunable parameters (may be changed at run‑time)
    // -----------------------------------------------------------------
    uint16_t _deadBand = 200;   ///< ADC counts treated as “no pedal”
    uint16_t _thrMin   = 230;   ///< Min ADC reading (default = 230)
    uint16_t _thrMax   = 800;   ///< Max ADC reading (default = 800)
    float    _rampStep = 1.0;  ///< ΔPWM per poll() call
    float   _brakeRampStep = 3.0; ///< ΔPWM per poll() when ramping down
    uint8_t  _slowPct  = 50;   ///< Speed cap in SLOW mode  (percent)
    uint8_t  _fastPct  = 100;  ///< Speed cap in FAST mode  (percent)
    uint8_t  _revPct   = 30;   ///< Speed cap in REVERSE    (percent)

    // -----------------------------------------------------------------
    // Real‑time state variables (updated every poll)
    // -----------------------------------------------------------------
    uint8_t  _isFastMode   = 0;   ///< FAST switch state (HIGH = fast, LOW = slow)
    uint8_t  _isReverse  = 0;   ///< REVERSE switch state (HIGH = reverse)
    uint16_t _lastAdc    = 0;   ///< Raw throttle reading saved for debug
    uint8_t  _thrMapped  = 0;   ///< Mapped throttle (0–255) after deadband, mapping, and speed cap
    float    _targetPWM  = 0.0f;///< Desired PWM after mapping & caps
    float    _currentPWM = 0.0f;///< Actual PWM being output (after ramp)

    // -----------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------
    void readInputs();         ///< Read ADC and switch states
    void generateDesiredPWM(); ///< Generate target PWM based on throttle and switch states
    void writeToMotor();       ///< Write PWM to motors based on _currentPWM and _isReverse
};

#endif // FOURWD_H
