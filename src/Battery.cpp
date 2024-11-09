#include "Battery.h"

Battery::Battery(uint8_t inputPin) : inp(inputPin) { pinMode(inp, INPUT); };

float Battery::readBatt(void)
{
    for (int i = 0; i < 16; i++)
    {
        vbatt += analogReadMilliVolts(inp);
    };
    return (vbatt * 2 / 16 / 1000);
};
