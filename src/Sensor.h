#ifndef _Sensor_h
#define _Sensor_h

#include "Property.h"

template<typename T>
class Sensor
{
public:
  virtual bool Init() = 0;
  virtual int ReadSensor() = 0;
  virtual ~Sensor() = default;
  const T& GetValue()
  {
    //todo implement value filtering, etc. on this level using templates
    return _value.Get();
  }
  virtual void SetFiltered(bool) = 0;
  virtual bool PeakDetect() = 0;
protected:
  Property<T> _value;
};

#endif
