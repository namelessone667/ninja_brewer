#ifndef PROBE_H
#define PROBE_H

#include "application.h"
#include "math.h"
//#include <avr/wdt.h>
#include <OneWire.h>

#define ONE_WIRE_BUS_PIN D6
#define TEMP_ERROR NAN

class probe {
    static OneWire* _myWire;
    static double _sampleHz;
    static boolean _sampled;
    static unsigned long _lastSample;
    static unsigned int _offset;

    byte _address[8];
    double _temperature[4];
    double _filter[4];

    static boolean _isConv() { return !_myWire->read(); }
    boolean _getAddr();
    boolean _updateTemp();
    void _updateFilter();

    //ZPU
    double calibrationPointLow;
		double calibrationPointHigh;
		double calibrationValueLow;
		double calibrationValueHigh;
    bool firstRead = true;
    int logcounter = 0;

  public:
    probe(OneWire* onewire);
    probe(OneWire* onewire, byte address[8]);
    boolean isDeviceOnBus();
    bool init();
    bool update();
    boolean peakDetect();
    double getTemp() { return firstRead ? TEMP_ERROR : _temperature[0]; }
    double getFilter() { return firstRead ? TEMP_ERROR : _filter[0]; }

    //static bool getFailedFlag();
    static void setSampleHz(double hz) { _sampleHz = hz; }
    static boolean isReady();
    static void startConv();
    static double tempCtoF(double tempC) { return ((tempC * 9 / 5) + 32); }
    static double tempFtoC(double tempF) { return ((tempF - 32) * 5 / 9); }

    byte* getDeviceAddress();

    void setCalibration(double , double , double , double );
};

#endif
