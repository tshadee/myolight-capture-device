#pragma once
#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

class Battery
{
   private:
    uint32_t vbatt;
    uint8_t inp;

   public:
    Battery(uint8_t inputPin);

    float readBatt(void);
};

#endif  // BATTERY_H