#ifndef theApp_h
#define theApp_h

#include "application.h"
#include "globals.h"
#include "probe.h"
#include "Model.h"
#include "theAppUI.h"

class theApp
{
    public:
        static theApp& getInstance();
        theApp(theApp const&)               = delete;
        void operator=(theApp const&)       = delete;
        void init();
        void run();
        AppConfig getAppConfigValues();
        void setNewAppConfigValues(AppConfig newAppConfig);
    private:
        theApp();
        Model _model;
        theAppUI _view;
};

#endif
