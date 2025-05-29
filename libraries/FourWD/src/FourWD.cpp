// -----------------------------------------------------------------------------
// FourWD.cpp  —  Implementation of the 4‑wheel‑drive helper class
// -----------------------------------------------------------------------------
#include "FourWD.h"

//--------------------------------------------------------------------
// Constructor — simply store the pin numbers
//--------------------------------------------------------------------
FourWD::FourWD(uint8_t rpwmPin, uint8_t lpwmPin,
               uint8_t fastSwPin, uint8_t revSwPin,
               uint8_t thrPin)
    : _rpwmPin(rpwmPin), _lpwmPin(lpwmPin),
      _fastSwPin(fastSwPin), _revSwPin(revSwPin),
      _thrPin(thrPin) {}

//--------------------------------------------------------------------
// begin() — one‑time hardware initialisation
//--------------------------------------------------------------------
void FourWD::begin()
{
    // Motor PWM pins
    pinMode(_rpwmPin, OUTPUT);
    pinMode(_lpwmPin, OUTPUT);

    // Toggle switch pins use INPUT_PULLUP, so wiring to GND == active LOW
    pinMode(_fastSwPin, INPUT_PULLUP);
    pinMode(_revSwPin,  INPUT_PULLUP);

    // Keep motors off until first poll()
    analogWrite(_rpwmPin, 0);
    analogWrite(_lpwmPin, 0);

#ifdef DEBUG
    Serial.begin(115200);
#endif
}

//--------------------------------------------------------------------
// poll() — core control loop (call as fast as you like)
//--------------------------------------------------------------------
void FourWD::poll()
{
    // 1) Read throttle (0–1023) and apply dead‑band
    _lastAdc = analogRead(_thrPin);
    int adc  = _lastAdc;
    if (adc < _deadBand) adc = 0;

    // 2) Determine speed cap based on switch position
    uint8_t limitPct;
    if (digitalRead(_revSwPin) == LOW)          // reverse?
        limitPct = _revPct;
    else if (digitalRead(_fastSwPin) == LOW)    // fast?
        limitPct = 100;
    else                                        // centre = slow
        limitPct = _slowPct;

    // 3) Map throttle → target PWM (0‑255), then apply cap
    uint8_t pwmMax   = (255UL * limitPct) / 100;
    _targetPWM = map(adc, 0, 1023, 0, pwmMax);

    // 4) Slew toward target and update outputs
    _applyRamp();

#ifdef DEBUG
    // ───── heartbeat every 500 ms ─────
    static uint32_t dbgTimer = 0;
    if (millis() - dbgTimer >= 500) {
        dbgTimer = millis();
        Serial.print(F("Throttle:")); Serial.print(_lastAdc);
        Serial.print(F("  Limit%:")); Serial.print(limitPct);
        Serial.print(F("  PWM:"));    Serial.println(_currentPWM);
    }
#endif
}

//--------------------------------------------------------------------
// Runtime‑tuneable setters
//--------------------------------------------------------------------
void FourWD::setDeadband(uint16_t v) { _deadBand = v; }
void FourWD::setRampStep(float s)    { _rampStep = s; }
void FourWD::setSlowPct(uint8_t p)   { _slowPct  = p; }
void FourWD::setRevPct(uint8_t p)    { _revPct   = p; }

//--------------------------------------------------------------------
// _applyRamp() — protect drivetrain by limiting dPWM/dt
//--------------------------------------------------------------------
void FourWD::_applyRamp()
{
    // 1) Nudge _currentPWM toward _targetPWM by ±_rampStep
    if (_currentPWM < _targetPWM)
        _currentPWM = min(_currentPWM + _rampStep, _targetPWM);
    else if (_currentPWM > _targetPWM)
        _currentPWM = max(_currentPWM - _rampStep, _targetPWM);

    // 2) Drive exactly one direction pin at a time
    if (digitalRead(_revSwPin) == LOW) {
        analogWrite(_lpwmPin, static_cast<int>(_currentPWM)); // reverse
        analogWrite(_rpwmPin, 0);
    } else {
        analogWrite(_rpwmPin, static_cast<int>(_currentPWM)); // forward
        analogWrite(_lpwmPin, 0);
    }
}

// -----------------------------------------------------------------------------
// End of FourWD.cpp
// -----------------------------------------------------------------------------
