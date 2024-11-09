#include "PowerSupply_Handler.h"

PowerSupply::PowerSupply(uint8_t EN) : enablePin(EN)
{
    pinMode(enablePin, OUTPUT);
    digitalWrite(enablePin, LOW);
};

void PowerSupply::enableIC(void) { digitalWrite(enablePin, HIGH); };

void PowerSupply::disableIC(void) { digitalWrite(enablePin, LOW); };
