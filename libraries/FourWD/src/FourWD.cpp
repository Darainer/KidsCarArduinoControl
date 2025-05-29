// -----------------------------------------------------------------------------
// FourWD.cpp  —  Implementation of the 4‑wheel‑drive helper class
// -----------------------------------------------------------------------------
#include "FourWD.h"

//--------------------------------------------------------------------
// Constructor — store pin numbers in member initialiser list
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
    // Motor direction pins
    pinMode(_rpwmPin, OUTPUT);
    pinMode(_lpwmPin, OUTPUT);

    // Toggle switch pins use the internal pull‑up so an active switch
    // simply shorts the pin to GND.
    pinMode(_fastSwPin, INPUT_PULLUP);
    pinMode(_revSwPin,  INPUT_PULLUP);

    // Ensure wheels stay still on power‑up.
    analogWrite(_rpwmPin, 0);
    analogWrite(_lpwmPin, 0);

#ifdef DEBUG
    Serial.begin(115200);
#endif
}

//--------------------------------------------------------------------
// poll() — call as fast as loop() allows (main control loop)
//--------------------------------------------------------------------
void FourWD::poll()
{
    // 1) Read throttle and apply dead‑band
    int adc = analogRead(_thrPin);          // 0–1023
    if (adc < _deadBand) adc = 0;           // ignore tiny noise

    // 2) Pick speed cap depending on switch position
    uint8_t limitPct;
    if (digitalRead(_revSwPin) == LOW)          // REVERSE active?
        limitPct = _revPct;
    else if (digitalRead(_fastSwPin) == LOW)    // FAST active?
        limitPct = 100;
    else                                        // otherwise SLOW
        limitPct = _slowPct;

    // 3) Map throttle to PWM 0‑255, then apply % cap
    uint8_t pwmMax  = (255UL * limitPct) / 100; // 0‑255 scale
    _targetPWM = map(adc, 0, 1023, 0, pwmMax);

    // 4) Slew‑rate limit and output
    _applyRamp();

#ifdef DEBUG
    static uint32_t t0 = 0;
    if (millis() - t0 > 500) {               // twice per second
        t0 = millis();
        Serial.print("ADC:");   Serial.print(adc);
        Serial.print(" cap%:"); Serial.print(limitPct);
        Serial.print(" PWM:");  Serial.println(_currentPWM);
    }
#endif
}

//--------------------------------------------------------------------
// Public setters — allow runtime tuning without recompiling
//--------------------------------------------------------------------
void FourWD::setDeadband(uint16_t v) { _deadBand = v; }
void FourWD::setRampStep(float s)    { _rampStep = s; }
void FourWD::setSlowPct(uint8_t p)   { _slowPct  = p; }
void FourWD::setRevPct(uint8_t p)    { _revPct   = p; }

//--------------------------------------------------------------------
// _applyRamp() — smooth acceleration / deceleration
//--------------------------------------------------------------------
void FourWD::_applyRamp()
{
    // 1) Move current PWM toward target by ±_rampStep, no overshoot.
    if (_currentPWM < _targetPWM)
        _currentPWM = min(_currentPWM + _rampStep, _targetPWM);
    else if (_currentPWM > _targetPWM)
        _currentPWM = max(_currentPWM - _rampStep, _targetPWM);

    // 2) Drive the correct direction pin
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
