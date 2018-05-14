#ifndef _Sensor_h
#define _Sensor_h

#include "Property.h"

template<typename T>
class Sensor
{
public:
  virtual bool Init() = 0;
  virtual int ReadSensor() = 0;
  const T& GetValue()
  {
    return _value.Get();
  }
private:
  Property<T> _value;
};

#endif
