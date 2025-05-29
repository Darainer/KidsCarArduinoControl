#define DEBUG  // this will print debug messages to Serial, comment out to disable
#include <FourWD.h>

// serial tuning commands:
/* dead 0 – 100 (ADC counts) example: dead 30
ramp 1 – 20 (PWM counts)  example: ramp 3
slow 20 – 100 (%)       example: slow 50
rev 10 – 100 (%)        example: rev 30
*/

FourWD car;           // defaults match the schematic

void setup() { car.begin(); }
void loop()  { car.poll();  }
