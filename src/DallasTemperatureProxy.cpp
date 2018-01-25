#include "DallasTemperatureProxy.h"

DallasTemperatureProxy::DallasTemperatureProxy(OneWire *oneWire)
{
  _oneWire = oneWire;
  _devicesAddressMap["0x28,0x31,0x9b,0xec,0x07,0x00,0x00,0x22"] = FRIDGE_TEMPERATURE;
  _devicesAddressMap["0x28,0x9d,0x87,0xe8,0x8,0x0,0x0,0xf"] = BEER_TEMPERATURE;
  _devicesAddressMap["0x28,0x78,0x28,0x85,0x8,0x0,0x0,0x9c"] = BEER_TEMPERATURE;
}

int DallasTemperatureProxy::Init()
{
  byte address[8];
  String addressString;
  probe *new_probe;

  //check for device presence on bus
  if(_oneWire->reset() == 0)
    return -1;

  _oneWire->reset_search();

  while(_oneWire->search(address) == 1)
  {
     addressString = "";
     for(int i = 0; i < 8; i++)
     {
         addressString += "0x" + String(address[i], HEX);
         if(i < 7)
          addressString += ",";
     }
     if(_devicesAddressMap.count(addressString) == 1)
     {
       new_probe = new probe(_oneWire, address);
       _devicesMap[_devicesAddressMap[addressString]] = new_probe;
     }
  }

  for (auto it = _devicesMap.begin(); it != _devicesMap.end(); ++it) {
    //check device on bus by reading scratchpad on devices address and performing CRC check
    if(it->second->init() == false)
      return -2;
  }

  probe::startConv();

  return 0;
}

int DallasTemperatureProxy::ReadTemperatures()
{
  if(probe::isReady())
  {
    //check for device presence on bus
    if(_oneWire->reset() == 0)
      return -1;

    for (auto it = _devicesMap.begin(); it != _devicesMap.end(); ++it) {
      int i = 0;
      while(true)
      {
        //if CRC check return false, retry
        if(it->second->update() == true)
          break;
        // if number of retries to read temp is more (or equal) than TEMP_READ_RETRY_COUNT, return -2
        if(i++ >= TEMP_READ_RETRY_COUNT)
          return -2;
      }
    }

    probe::startConv();
    return 1;
  }
  return 0;
}
double DallasTemperatureProxy::GetTemperature(DallasTemperatureProbeAssignment probe_assignment)
{
    if(_devicesMap.count(probe_assignment) == 0)
      return TEMP_ERROR;
    else
      return _devicesMap[probe_assignment]->getTemp();
}

double DallasTemperatureProxy::GetFilteredTemperature(DallasTemperatureProbeAssignment probe_assignment)
{
  if(_devicesMap.count(probe_assignment) == 0)
    return TEMP_ERROR;
  else
    return _devicesMap[probe_assignment]->getFilter();
}
