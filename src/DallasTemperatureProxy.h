#ifndef DallasTemperatureProxy_h
#define DallasTemperatureProxy_h

#include "probe.h"
#include <map>

enum DallasTemperatureProbeAssignment {
  FRIDGE_TEMPERATURE,
  BEER_TEMPERATURE
};

class DallasTemperatureProxy
{
  public:
    DallasTemperatureProxy(OneWire *oneWire);
    void Init();
  private:
    OneWire *_oneWire;
    std::map<String, DallasTemperatureProbeAssignment> _devicesAddressMap;
    std::map<DallasTemperatureProbeAssignment, probe*> _devicesMap;
};

#endif
