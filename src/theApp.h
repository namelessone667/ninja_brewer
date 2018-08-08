#ifndef theApp_h
#define theApp_h

#include "application.h"
#include "globals.h"
#include "probe.h"
#include "theAppUI.h"
#include "OneWire.h"
#include "PublisherProxy.h"
#include "CoolerHeaterContoller.h"
#include "PIDProxy.h"
#include "NinjaModel.h"
#include "DS18B20Sensor.h"
#include "TemperatureProfile.h"

class theApp : public CEventReceiver
{
    public:
        static theApp& getInstance();
        theApp(theApp const&)               = delete;
        void operator=(theApp const&)       = delete;
        void init();
        void run();
        NinjaModel& getModel();
        void setErrorState(String error_message);
        String getErrorMessage();
        const Logger& getLogger();
        void ActivateController();
        void DisableController();
        void setPID(int, double);
        void setHeatPID(int, double);
        void setNewTargetTemp(double);
        void reboot();
        void reinitLCD();
        void saveState();
        void switchSensors();
        TemperatureProfile& getTemperatureProfile();
    private:
        theApp();
        int initSensors();

        void handlePIDModeChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        void handleHeatPIDModeChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        void handleTempProfileStepsChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        NinjaModel _model;
        theAppUI _view;
        OneWire _oneWire;
        PublisherProxy _publisherProxy;
        String _error;
        Logger _log;
        CoolerHeaterContoller _controller;
        PIDProxy _mainPID;
        PIDProxy _heatPID;
        DS18B20Sensor* _tempSensor1 = NULL;
        DS18B20Sensor* _tempSensor2 = NULL;

        TemperatureProfile _tempProfile;

        long _pid_log_timestamp;
        long _sensorDataTimestamp;
        long _error_timestamp = -1;

        bool _reboot;

        bool readSensors();
};

#endif
