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
car.setLowerThrottleCap(230); ///< Min ADC reading (default = 230)
car.setUpperThrottleCap(800); ///< Max ADC reading (default = 800)
car.setRampStep(1);         ///< PWM Δ per `poll()` (default = 3)
#ifdef DEBUG
    Serial.begin(115200);
#endif
}
void loop()  { 
    car.poll(); 
#ifdef DEBUG
        Serial.print(F("Throttle:")); Serial.print(car.currentThrottleRaw());
        Serial.print(F("  PWM:"));    Serial.println(car.currentPwm());
#endif
}
