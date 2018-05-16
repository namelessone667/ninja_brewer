#ifndef _DS18B20Sensor_h
#define _DS18B20Sensor_h

#include "Sensor.h"
#include "probe.h"

#define TEMP_READ_RETRY_COUNT 5

struct OneWireAddress
{
  byte address[8];

  OneWireAddress& operator=(const OneWireAddress& other)
    {
        for(int i = 0; i < 8; i++)
        {
          this->address[i] = other.address[i];
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

  ~DS18B20Sensor()
  {
    delete _probe;
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

    if(_startConversion && _startConversionMe)
    {
      _startConversion = false;
      _startConversionMe = false;
      probe::startConv();
      return 0;
    }
    _startConversionMe = false;

    if(!probe::isReady())
    {
      return 0;
    }

    for(int i = 0; i < TEMP_READ_RETRY_COUNT; i++)
    {
      if(_probe->update() == false)
        continue;

      _lastUpdate = millis();
      if(_filtered)
        _value = _probe->getFilter();
      else
        _value = _probe->getTemp();

      if(_startConversion == false)
      {
        _startConversion = true;
        _startConversionMe = true;
      }
      return 1;
    }

    if(_startConversion == false)
    {
      _startConversion = true;
      _startConversionMe = true;
    }
    return -2;

  }

  bool PeakDetect()
  {
    return _probe->peakDetect();
  }

  void SetFiltered(bool filtered)
  {
    _filtered = filtered;
  }

  const OneWireAddress& GetAddress()
  {
    return _address;
  }

private:
  OneWireAddress _address;
  probe* _probe;
  bool _isInitialized = false;
  static bool _startConversion;
  bool _startConversionMe = true;
  unsigned long _lastUpdate = 0;
  bool _filtered = false;
};

#endif
