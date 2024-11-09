#pragma once
#ifndef POWERSUPPLY_HANDLER_H
#define POWERSUPPLY_HANDLER_H

#include <Arduino.h>

class PowerSupply
{
   private:
    uint8_t enablePin;

   public:
    PowerSupply(uint8_t EN);

    void enableIC(void);
    void disableIC(void);
};

#endif  // POWERSUPPLY_HANDLER_H
