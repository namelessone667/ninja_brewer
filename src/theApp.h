#ifndef theApp_h
#define theApp_h

#include "application.h"
#include "globals.h"
#include "probe.h"
#include "Model.h"
#include "theAppUI.h"
#include "OneWire.h"
#include "DallasTemperatureProxy.h"

class theApp
{
    public:
        static theApp& getInstance();
        theApp(theApp const&)               = delete;
        void operator=(theApp const&)       = delete;
        void init();
        void run();
        //AppConfig getAppConfigValues();
        //AppState getAppStateValues();
        //void setNewAppConfigValues(AppConfig newAppConfig);
        Model& getModel();
    private:
        theApp();
        Model _model;
        theAppUI _view;
        OneWire _oneWire;
        DallasTemperatureProxy _tempProxy;
};

#endif
