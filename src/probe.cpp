#include "probe.h"

#ifndef DEBUG
#define DEBUG false
#endif

#define WAIT_CYCLES 5000

OneWire* probe::_myWire = 0;  // static member initialization
double probe::_sampleHz = 1;
boolean probe::_sampled = true;
unsigned long probe::_lastSample = 0;
unsigned int probe::_offset = 350;

probe::probe(OneWire* onewire)
{
  if (!_myWire)
    _myWire = onewire;

  _getAddr();

  calibrationPointLow = 0;
  calibrationPointHigh = 100;
  calibrationValueLow = 0;
  calibrationValueHigh = 100;
}

probe::probe(OneWire* onewire, byte address[8])
{
  if (!_myWire)
    _myWire = onewire;

  for(int i = 0; i < 8; i++)
    _address[i] = address[i];

  calibrationPointLow = 0;
  calibrationPointHigh = 100;
  calibrationValueLow = 0;
  calibrationValueHigh = 100;
}

bool probe::init() {
  byte data[9];

  if(isDeviceOnBus() == false)
  {
    //address CRC check failed
    return false;
  }

  if(_myWire->reset() == 0)
  {
    //no devices on the bus or bus is corrupted
    return false;
  }

  _myWire->select(_address);
  _myWire->write(0xBE);
  for (int i = 0; i < 9; i++) data[i] = _myWire->read();
  if (OneWire::crc8(data, 8) != data[8]) return false;  // return false if crc check fails

  return true;
}

void probe::startConv() {  //  initiate temperature conversion with _offset(ms) to temp/filter update

    _myWire->reset();
    _myWire->skip();
    _myWire->write(0x44);
    _lastSample = millis();

}

boolean probe::isReady() {  //  initiate temperature and filter update with frequency = sampleHz
  /*if (millis() >= (unsigned long)(_lastSample + 1000/_sampleHz)) {
    _lastSample = millis();
    _sampled = true;
    int wait = -1;
    do
    {
        wait++;
        if(wait > WAIT_CYCLES)
        {
            failed = true;
            break;
        }
    } while (_isConv());

    #if DEBUG == true
      Serial.println(wait);
    #endif
    if (!wait) _offset++;
      else _offset--;
    return true;
  }
  return false;*/
  if(millis() >= (unsigned long)(_lastSample + 1000/_sampleHz))
    return true;
  else
    return false;
}

bool probe::update() {
    if(_updateTemp() == false)
        return false;
    _updateFilter();
    return true;
}

boolean probe::_getAddr() {  // find next OneWire device, store rom address in probe struct
  _myWire->search(_address);
  if (OneWire::crc8(_address, 7) != _address[7]) return false;  // return false if crc check fails
  #if DEBUG == true
    Serial.print(F("New DS18B20 sensor found, addr:"));
    for (int i = 0; i < 6; i++) {
      Serial.print(_address[i], HEX);
      Serial.print(F(" "));
    }
    Serial.println(_address[7], HEX);
  #endif
  return true;
}

boolean probe::isDeviceOnBus()
{
    if (OneWire::crc8(_address, 7) != _address[7])
        return false;
    else
        return true;
}

byte* probe::getDeviceAddress()
{
  return _address;
}

void probe::setCalibration(double newCalibrationPointLow, double newCalibrationValueLow, double newCalibrationPointHigh, double newCalibrationValueHigh)
{
  calibrationPointLow = newCalibrationPointLow;
  calibrationPointHigh = newCalibrationPointHigh;
  calibrationValueLow = newCalibrationValueLow;
  calibrationValueHigh = newCalibrationValueHigh;
}

/*bool probe::getFailedFlag()
{
    return failed;
}*/

boolean probe::_updateTemp() {  // read OneWire device temperature
  byte data[9];
  _myWire->reset();
  _myWire->select(_address);
  _myWire->write(0xBE);
  for (int i = 0; i < 9; i++) data[i] = _myWire->read();
  if (OneWire::crc8(data, 8) != data[8]) return false;  // return false if crc check fails
  unsigned int raw = ((data[1] << 8) | data[0]);
  byte cfg = (data[4] & 0x60);
  if (cfg == 0x00) raw = raw << 3;        // 9 bit res, 93.75 ms MAX conversion time
    else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms MAX conversion time
    else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms MAX conversion time
  for (int i = 3; i > 0; i--) {
    _temperature[i] = _temperature[i - 1];
  }
  //zpu:commented _temperature[0] = ((data[1] << 8) | data[0]) / 16.0;  // default is 12 bit resolution, 750 ms MAX conversion time
  //zpu start - get calibrated temp
  //double rawtemp = ((data[1] << 8) | data[0]) / 16.0;  // default is 12 bit resolution, 750 ms MAX conversion time
  unsigned int SignBit = raw & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    raw = (raw ^ 0xffff) + 1; // 2's comp
  }
  double rawtemp = (double)raw / (double)16.0;  // default is 12 bit resolution, 750 ms MAX conversion time
  if (SignBit){
    rawtemp = rawtemp * -1;
  }
  double refLow = calibrationPointLow;
  double refHigh = calibrationPointHigh;
  double rawLow = calibrationValueLow;
  double rawHigh = calibrationValueHigh;
  double refRange = refHigh - refLow;
  double rawRange = rawHigh - rawLow;

   _temperature[0] = (((rawtemp - rawLow) * refRange) / rawRange) + refLow;
   if(firstRead)
   {
     _temperature[1] = _temperature[2] = _temperature[3] = _temperature[0];
     _filter[0] = _filter[1] = _filter[2] = _filter[3] = _temperature[0];
     firstRead = false;
   }
  //zpu end - get calibrated temp
    #if DEBUG == true
      Serial.print(F("Temperature:"));
      Serial.print(_temperature[0]);
      Serial.println(F(" read from sensor."));
    #endif
  return true;
}

void probe::_updateFilter() {  // update butterworth filter
  for (int i = 3; i > 0; i--) { _filter[i] = _filter[i - 1]; }

  _filter[0] = (_temperature[3] + _temperature[0] + 3 * (_temperature[2] + _temperature[1]))/1.092799972e+03
                      + (0.6600489526 * _filter[3]) + (-2.2533982563 * _filter[2]) + (2.5860286592 * _filter[1]);

  #if DEBUG == true
    for (int i = 0; i < 3; i++) {
      Serial.print(_filter[i]);
      Serial.print(F(" "));
    }
    Serial.println(_filter[3]);
  #endif
}

boolean probe::peakDetect() {  // detect negative peaks for fridge overshoot tuning
  if ((_filter[0] > _filter[1]) && (_filter[1] <= _filter[2])) return true;
  return false;
}

/*void updateSlowArrays() {  // update SlowFilter and temperature arrays using latest fast array data
  #if DEBUG == true
    Serial.print(F("slow update... "));
    Serial.print((unsigned long)(millis() - lastSlowSample));
    Serial.print(F("ms elapsed. "));
    Serial.print(lastFastSample);
    Serial.print(F(" "));
    Serial.print(lastSlowSample);
    Serial.print(F(" "));
    Serial.print(freeRAM());
    Serial.println(F(" bytes free SRAM remaining"));
  #endif

  lastSlowSample = millis();
  for (int i = 3; i > 0; i--) {
    T0_Slow[i] = T0_Slow[i - 1];
    T0_SlowFilter[i] = T0_SlowFilter[i - 1];

    T1_Slow[i] = T1_Slow[i - 1];
    T1_SlowFilter[i] = T1_SlowFilter[i - 1];
  }
  T0_Slow[0] = T0_Fast[0];  // update slow temperature; update Butterworth filter with cutoff frequency 0.01*sample frequency (FS=0.1Hz)
  T0_SlowFilter[0] = (T0_Slow[3] + T0_Slow[0] + 3 * (T0_Slow[2] + T0_Slow[1]))/3.430944333e+04
                      + (0.8818931306 * T0_SlowFilter[3]) + (-2.7564831952 * T0_SlowFilter[2]) + (2.8743568927 * T0_SlowFilter[1]);
  T1_Slow[0] = T1_Fast[0];
  T1_SlowFilter[0] = (T1_Slow[3] + T1_Slow[0] + 3 * (T1_Slow[2] + T1_Slow[1]))/3.430944333e+04
                      + (0.8818931306 * T1_SlowFilter[3]) + (-2.7564831952 * T1_SlowFilter[2]) + (2.8743568927 * T1_SlowFilter[1]);
  #if DEBUG == true
    for (int i = 0; i < 3; i++) {
      Serial.print(T0_SlowFilter[i]);
      Serial.print(F(" "));
    }
    Serial.println(T0_SlowFilter[3]);
  #endif
}*/
