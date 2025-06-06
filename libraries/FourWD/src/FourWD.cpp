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

}

//--------------------------------------------------------------------
// poll() — core control loop (call as fast as you like)
//--------------------------------------------------------------------
void FourWD::poll()
{
    readInputs();           // SECTION 1: Get sensor inputs
    generateDesiredPWM();   // SECTION 2: Generate desired PWM
    writeToMotor();         // SECTION 3: Send PWM to motor
}

void FourWD::readInputs()
{
    _lastAdc = analogRead(_thrPin);  // Read throttle (0–1023)
    _isReverse = (digitalRead(_revSwPin) == LOW); // reverse switch active?
    _isFastMode = (digitalRead(_fastSwPin) == LOW); // fast switch active?
}

void FourWD::generateDesiredPWM()
{    
    if (_lastAdc < _deadBand) _lastAdc = 0; // ignore noise below deadband -> clamp to 0

    // Determine speed cap based on switch position
    uint8_t limitPct;
    if (_isReverse)         // reverse?
        limitPct = _revPct;
    else if (_isFastMode)   // fast?
        limitPct = _fastPct;
    else                  // centre = slow
        limitPct = _slowPct;

    // Map effective throttle range → target PWM (0‑255)
    uint8_t pwmMax = (255UL * limitPct) / 100;
    _targetPWM = map(_lastAdc, 0, _thrMax, 0, pwmMax);
    
    // Apply ramping logic here
    // Nudge _currentPWM toward _targetPWM by ±_rampStep
    if (_currentPWM < _targetPWM)
        _currentPWM = min(_currentPWM + _rampStep, _targetPWM);
    else if (_currentPWM > _targetPWM)
        _currentPWM = max(_currentPWM - _brakeRampStep, _targetPWM);
}

void FourWD::writeToMotor()
{
    // Drive exactly one direction pin at a time
    if (_isReverse) {
        analogWrite(_lpwmPin, static_cast<int>(_currentPWM)); // reverse
        analogWrite(_rpwmPin, 0);
    } else {
        analogWrite(_rpwmPin, static_cast<int>(_currentPWM)); // forward
        analogWrite(_lpwmPin, 0);
    }
}

//--------------------------------------------------------------------
// Runtime‑tuneable setters
//--------------------------------------------------------------------
void FourWD::setDeadband(uint16_t v) { _deadBand = v; }
void FourWD::setLowerThrottleCap(uint8_t low ){ _thrMin = low; }
void FourWD::setUpperThrottleCap(uint8_t high) { _thrMax = high; }
void FourWD::setRampStep(float s)    { _rampStep = s; }
void FourWD::setBrakeRampStep(float s) { _brakeRampStep = s; }
void FourWD::setSlowPct(uint8_t p)   { _slowPct  = p; }
void FourWD::setRevPct(uint8_t p)    { _revPct   = p; }

// -----------------------------------------------------------------------------
// End of FourWD.cpp
// -----------------------------------------------------------------------------
