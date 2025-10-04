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
      _thrPin(thrPin),
      _currentPWM(0.0f),    // initialize as float
      _lastDirection(false) // track last direction for safe swaps
{}

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
    uint8_t limitPct = determineSpeedCap();

    // Map effective throttle range → target PWM (0‑255)
    uint8_t pwmMax = (255UL * limitPct) / 100;
    uint16_t adcClamp = constrain(_lastAdc, _thrMin, _thrMax);
    _targetPWM = map(adcClamp, _thrMin, _thrMax, 0, pwmMax);

    // Handle direction changes safely: ramp down before swap
    if (_isReverse != _lastDirection && _currentPWM > 0.0f) {
        // Ramp down _currentPWM to 0 before changing direction
        if (_currentPWM > _brakeRampStep) {
            _currentPWM = max(_currentPWM - _brakeRampStep, 0.0f);
            // Do NOT update direction until ramped down
            return;
        } else {
            _currentPWM = 0.0f;
            _lastDirection = _isReverse;
            // Now allow direction change
        }
    }

    // Apply ramping logic - Nudge _currentPWM toward _targetPWM by ±_rampStep
    if (_currentPWM < _targetPWM)
        _currentPWM = min(_currentPWM + _rampStep, (float)_targetPWM);
    else if (_currentPWM > _targetPWM)
        _currentPWM = max(_currentPWM - _brakeRampStep, (float)_targetPWM);

    _currentPWM = constrain(_currentPWM, 0.0f, (float)pwmMax); // final clamp
}

void FourWD::writeToMotor()
{
    uint8_t pwmOut = static_cast<uint8_t>(round(_currentPWM));

    // Drive exactly one direction pin at a time
    if (_isReverse) {
        analogWrite(_lpwmPin, pwmOut); // reverse
        analogWrite(_rpwmPin, 0);
    } else {
        analogWrite(_rpwmPin, pwmOut); // forward
        analogWrite(_lpwmPin, 0);
    }
}

uint8_t FourWD::determineSpeedCap()
{
    if (_isReverse)   return _revPct;   // REVERSE always wins
    if (_isFastMode)  return _fastPct;  // FAST next
    return _slowPct;                    // otherwise SLOW
}
//--------------------------------------------------------------------
// Runtime‑tuneable setters
//--------------------------------------------------------------------
void FourWD::setDeadband(uint16_t v) { _deadBand = v; }
void FourWD::setLowerThrottleCap(uint16_t low ){ _thrMin = low; }
void FourWD::setUpperThrottleCap(uint16_t high) { _thrMax = high; }
void FourWD::setRampStep(float s)    { _rampStep = s; }
void FourWD::setBrakeRampStep(float s) { _brakeRampStep = s; }
void FourWD::setSlowPct(uint8_t p)   { _slowPct  = p; }
void FourWD::setRevPct(uint8_t p)    { _revPct   = p; }

// -----------------------------------------------------------------------------
// End of FourWD.cpp
// -----------------------------------------------------------------------------