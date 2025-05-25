#ifndef FOURWD_H
#define FOURWD_H

#include <Arduino.h>

class FourWD {
public:
    FourWD(uint8_t rpwm = 5,  uint8_t lpwm = 6,
           uint8_t fastSw = 3, uint8_t revSw = 4,
           uint8_t throttle = A0);

    void begin();
    void poll();                // call repeatedly from loop()

    // optional tweaks
    void setDeadband(uint16_t adc);
    void setRampStep(float step);
    void setSlowPct(uint8_t pct);
    void setRevPct(uint8_t pct);

private:
    const uint8_t _rpwm, _lpwm, _fastSw, _revSw, _throttle;
    uint16_t _deadBand  = 30;
    float    _rampStep  = 3.0;
    uint8_t  _slowPct   = 50;
    uint8_t  _revPct    = 30;

    float _targetPWM = 0;
    float _currentPWM = 0;

    void _ramp();
};

#endif
