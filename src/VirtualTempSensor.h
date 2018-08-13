#ifndef _VirtualTempSensor_h
#define _VirtualTempSensor_h

#include "Sensor.h"


class VirtualTempSensor : public Sensor<double>
{
public:
  VirtualTempSensor()
  {
    _value.Set(18);
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
