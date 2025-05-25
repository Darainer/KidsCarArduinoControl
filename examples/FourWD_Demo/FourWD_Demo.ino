#include <FourWD.h>

FourWD car;           // defaults match the schematic

void setup() { car.begin(); }
void loop()  { car.poll();  }
