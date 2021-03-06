#ifndef theApp_h
#define theApp_h

#include "application.h"
#include "globals.h"
#include "probe.h"
#include "Model.h"
#include "theAppUI.h"
#include "OneWire.h"
#include "DallasTemperatureProxy.h"
#include "PublisherProxy.h"
#include "CoolerHeaterContoller.h"
#include "PID_v1.h"

class theApp
{
    public:
        static theApp& getInstance();
        theApp(theApp const&)               = delete;
        void operator=(theApp const&)       = delete;
        void init();
        void run();
        void setNewAppConfigValues(AppConfig newAppConfig);
        const Model& getModel();
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
    private:
        theApp();
        Model _model;
        theAppUI _view;
        OneWire _oneWire;
        DallasTemperatureProxy _tempProxy;
        PublisherProxy _publisherProxy;
        String _error;
        Logger _log;
        CoolerHeaterContoller _controller;
        PID _mainPID;
        PID _heatPID;

        long _pid_log_timestamp;

        bool _reboot;

        bool readSensors();
};

#endif
