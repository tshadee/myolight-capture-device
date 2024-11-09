#include "LM27762_Handler.h"

LM27762::LM27762(uint8_t EN) : enablePin(EN)
{
    pinMode(enablePin, OUTPUT);
    digitalWrite(enablePin, LOW);
};

void LM27762::enableIC(void) { digitalWrite(enablePin, HIGH); };

void LM27762::disableIC(void) { digitalWrite(enablePin, LOW); };
