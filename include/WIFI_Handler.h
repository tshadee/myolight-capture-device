#pragma once
#ifndef WIFI_HOST_HANDLER_H
#define WIFI_HOST_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#include "COMMON_DEFS.h"

class WHostHandler
{
   private:
   public:
    WHostHandler(WiFiClient* client);
};

#endif  // WIFI_HANDLER_H