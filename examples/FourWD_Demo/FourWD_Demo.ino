#define DEBUG  // this will print debug messages to Serial, comment out to disable
#include <FourWD.h>

// serial tuning commands:
/* dead 0 – 100 (ADC counts) example: dead 30
ramp 1 – 20 (PWM counts)  example: ramp 3
slow 20 – 100 (%)       example: slow 50
rev 10 – 100 (%)        example: rev 30
*/

FourWD car;           // defaults match the schematic

void setup() { 
car.begin();
car.setDeadband(200); ///< Ignore throttle noise below this ADC value (default = 200)
car.setLowerThrottleCap(250); ///< Min ADC reading (default = 230)
car.setUpperThrottleCap(840); ///< Max ADC reading (default = 800)
car.setRampStep(0.15);         ///< PWM Δ per `poll()` 
car.setBrakeRampStep(0.25); ///< PWM Δ per `poll()` when ramping down 
#ifdef DEBUG
    Serial.begin(115200);
#endif
}
void loop()  { 
    car.poll(); 
#ifdef DEBUG
        Serial.print(F("Throttle:")); Serial.print(car.currentThrottleRaw());
        Serial.print(F("  PWM:"));    Serial.println(car.currentPwm());
        Serial.print("  Target:"); Serial.print(car.currentTargetPwm());
        Serial.print("FAST="); Serial.print(car.isFastMode());
#endif
}
