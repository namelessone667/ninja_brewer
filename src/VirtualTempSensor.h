#ifndef _VirtualTempSensor_h
#define _VirtualTempSensor_h

#include "DS18B20Sensor.h"


class VirtualTempSensor : public IDS18B20Sensor
{
public:
  VirtualTempSensor() : VirtualTempSensor((OneWireAddress){{0,0,0,0,0,0,0,0}})
  {

  }
  VirtualTempSensor(const OneWireAddress& address)
  {
    _address = address;
    _value.Set(18);
  }

  ~VirtualTempSensor()
  {
  }

  bool Init()
  {
    return true;
  }

  int ReadSensor()
  {
    return 1;
  }

  void SetFiltered(bool)
  {

  }

  bool PeakDetect()
  {
    return false;
  }


};

#endif
