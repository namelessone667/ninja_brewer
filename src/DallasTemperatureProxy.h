#ifndef DallasTemperatureProxy_h
#define DallasTemperatureProxy_h

#include "probe.h"
#include <map>

#define TEMP_READ_RETRY_COUNT 5

enum DallasTemperatureProbeAssignment {
  FRIDGE_TEMPERATURE,
  BEER_TEMPERATURE
};

class DallasTemperatureProxy
{
  public:
    DallasTemperatureProxy(OneWire *oneWire);
    int Init();
    int ReadTemperatures();
    double GetTemperature(DallasTemperatureProbeAssignment probe_assignment);
    double GetFilteredTemperature(DallasTemperatureProbeAssignment probe_assignment);
  private:
    OneWire *_oneWire;
    std::map<String, DallasTemperatureProbeAssignment> _devicesAddressMap;
    std::map<DallasTemperatureProbeAssignment, probe*> _devicesMap;
};

#endif
