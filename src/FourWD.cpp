#include "FourWD.h"

FourWD::FourWD(uint8_t rpwm, uint8_t lpwm,
               uint8_t fastSw, uint8_t revSw,
               uint8_t throttle)
: _rpwm(rpwm), _lpwm(lpwm),
  _fastSw(fastSw), _revSw(revSw),
  _throttle(throttle) {}

void FourWD::begin() {
    pinMode(_rpwm, OUTPUT);
    pinMode(_lpwm, OUTPUT);
    pinMode(_fastSw, INPUT_PULLUP);
    pinMode(_revSw,  INPUT_PULLUP);
    analogWrite(_rpwm, 0);
    analogWrite(_lpwm, 0);
#ifdef DEBUG
    Serial.begin(115200);
#endif
}

void FourWD::poll() {
    int raw = analogRead(_throttle);
    if (raw < _deadBand) raw = 0;

    uint8_t limitPct;
    if (digitalRead(_revSw) == LOW)      limitPct = _revPct;
    else if (digitalRead(_fastSw) == LOW) limitPct = 100;
    else                                  limitPct = _slowPct;

    uint8_t pwmMax = (255UL * limitPct) / 100;
    _targetPWM = map(raw, 0, 1023, 0, pwmMax);

    _ramp();
#ifdef DEBUG
    static uint32_t t0 = 0;
    if (millis() - t0 > 500) {
        t0 = millis();
        Serial.print("Throttle:"); Serial.print(raw);
        Serial.print("  Limit%:"); Serial.print(limitPct);
        Serial.print("  PWM:");    Serial.println(_currentPWM);
    }
#endif
}

/* ---------- configuration helpers ---------- */
void FourWD::setDeadband(uint16_t v) { _deadBand = v; }
void FourWD::setRampStep(float s)    { _rampStep = s; }
void FourWD::setSlowPct(uint8_t p)   { _slowPct  = p; }
void FourWD::setRevPct(uint8_t p)    { _revPct   = p; }

/* ----------- private ------------------------ */
void FourWD::_ramp() {
    if (_currentPWM < _targetPWM)
        _currentPWM = min(_currentPWM + _rampStep, _targetPWM);
    else if (_currentPWM > _targetPWM)
        _currentPWM = max(_currentPWM - _rampStep, _targetPWM);

    if (digitalRead(_revSw) == LOW) {
        analogWrite(_lpwm, (int)_currentPWM);
        analogWrite(_rpwm, 0);
    } else {
        analogWrite(_rpwm, (int)_currentPWM);
        analogWrite(_lpwm, 0);
    }
}
