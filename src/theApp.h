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
        void setNewAppConfigValues(AppConfig newAppConfig);
        const Model& getModel();
        void setErrorState(String error_message);
        String getErrorMessage(); 
    private:
        theApp();
        Model _model;
        theAppUI _view;
        OneWire _oneWire;
        DallasTemperatureProxy _tempProxy;
        String _error;
};

#endif
