#ifndef _DS18B20Sensor_h
#define _DS18B20Sensor_h

#include "Sensor.h"
#include "probe.h"

#define TEMP_READ_RETRY_COUNT 5
#define TEMP_INVALID -274.0
#define TEMP_CONVERSION_NOT_READY 85.0

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

  bool operator==(const OneWireAddress &other) const
  {
    for(int i = 0; i < 8; i++)
    {
      if(this->address[i] != other.address[i])
        return false;
    }
    return true;
  }

  String ToString() const
  {
    return String::format("%x:%x:%x:%x:%x:%x:%x:%x", address[0],address[1],address[2],address[3],address[4],address[5],address[6],address[7]);
  }
};

class IDS18B20Sensor : public Sensor<double>
{
public:
  virtual ~IDS18B20Sensor() = default;
  virtual void SetFiltered(bool) = 0;
  virtual bool PeakDetect() = 0;
  const OneWireAddress& GetAddress()
  {
    return _address;
  }
protected:
  OneWireAddress _address;
};

class DS18B20Sensor : public IDS18B20Sensor
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

      if( _lastValue == TEMP_INVALID ||
          _probe->getTemp() != TEMP_CONVERSION_NOT_READY ||
          _lastValue == TEMP_CONVERSION_NOT_READY) // workaround for 85.0 sensor glitch (if sensor read 85.0C we ignore this reading for the first cycle)
      {
        if(_filtered)
          _value = _probe->getFilter();
        else
          _value = _probe->getTemp();
      }

      _lastValue = _probe->getTemp();

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


private:
  probe* _probe;
  bool _isInitialized = false;
  static bool _startConversion;
  bool _startConversionMe = true;
  unsigned long _lastUpdate = 0;
  bool _filtered = false;
  double _lastValue = TEMP_INVALID;
};

#endif
