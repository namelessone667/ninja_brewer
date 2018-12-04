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
#ifdef TEMP_PROFILES
#include "TemperatureProfile.h"
#endif

//TODO: rename theApp class to ninjaBrewer
//TODO: implement behavior in different states by separate classes
//TODO: device manager, hwmanager
//TODO: configuration_profiles - herms, fermenter, lager

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
        const String& getLCDText();
        double GetPIDPTerm() { return _mainPID.GetPTerm(); };
        double GetPIDITerm() { return _mainPID.GetITerm(); };
        double GetPIDDTerm() { return _mainPID.GetDTerm(); };
        String GetSensor1Address();
        String GetSensor2Address();
#ifdef TEMP_PROFILES
        TemperatureProfile& getTemperatureProfile();
#endif
    private:
        theApp();
        int initSensors();

        void handlePIDModeChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        void handlePIDIntegratorClampingChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        void handleHeatPIDIntegratorClampingChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        void handleHeatPIDModeChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        void handleOutputLimitsChangedChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        void handleHeatOutputLimitsChangedChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        void handleControllerSettingsChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        void handleConnectToCloudChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        NinjaModel _model;
        theAppUI _view;
        OneWire _oneWire;
        PublisherProxy _publisherProxy;
        String _error;
        Logger _log;
        CoolerHeaterContoller _controller;
        PIDProxy _mainPID;
        PIDProxy _heatPID;
        IDS18B20Sensor* _tempSensor1 = NULL;
        IDS18B20Sensor* _tempSensor2 = NULL;
#ifdef TEMP_PROFILES
        TemperatureProfile _tempProfile;
        void handleTempProfileStepsChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
        void handleTempProfileStatusChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs);
#endif
        long _pid_log_timestamp;
        long _sensorDataTimestamp;
        long _error_timestamp = -1;

        bool _reboot;

        bool readSensors();
};

#endif
