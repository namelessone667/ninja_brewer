#ifndef _DS18B20Sensor_h
#define _DS18B20Sensor_h

#include "Sensor.h"
#include "probe.h"

struct OneWireAddress
{
  byte address[8];

  OneWireAddress& operator=(const &OneWireAddress other)
    {
        for(int i = 0; i < 8; i++)
        {
          this.address[i] = other.address[i];
        }
        return *this;
    }
};

class DS18B20Sensor : public Sensor<double>
{
public:
  DS18B20Sensor(const OneWireAddress& address, OneWire* oneWire)
  {
    _address = address;
    _probe = new probe(oneWire, _address.address);
  }

  bool Init()
  {
    _isInitialized = _probe->init();

    return _isInitialized;
  }

  int ReadSensor()
  {
    if(!_isInitialized)
      return -1;

    if(_startConversion)
    {
      _startConversion = false;
      probe::startConv();
      return 0;
    }

    if(!probe::isReady())
    {
      return 0;
    }

    if(_probe->update())
    {
      _lastUpdate = millis();
      if(_filtered)
        _value = probe->getFilter();
      else
        _value = probe->getTemp();

      _startConversion = true;
      return 1;
    }

    _startConversion = true;
    return -2;

  }

  bool PeakDetect()
  {
    return _probe->peakDetect();
  }
  
private:
  OneWireAddress _address;
  probe* _probe;
  bool _isInitialized = false;
  static bool _startConversion = true;
  unsigned long _lastUpdate = 0;
  bool _filtered = false;
};

#endif
