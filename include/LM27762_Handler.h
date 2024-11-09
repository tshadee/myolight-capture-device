#pragma once
#ifndef LM27762_HANDLER_H
#define LM27762_HANDLER_H

#include <Arduino.h>

class LM27762
{
   private:
    uint8_t enablePin;

   public:
    LM27762(uint8_t EN);

    void enableIC(void);
    void disableIC(void);
};

#endif  // LM27762_HANDLER_H
