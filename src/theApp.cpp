#include "theApp.h"

//TODO create structures to store onewire device addresses
//TODO implement dynamic onewire device discovery and initialization
//TODO implement capability to add new onewire devices throug menu

theApp::theApp() : _view(this)
{

}

theApp& theApp::getInstance()
{
  static theApp instance; // Guaranteed to be destroyed.
                          // Instantiated on first use.
  return instance;
}

void theApp::init()
{
  _model.loadAppConfigFromEEPROM();
  _model.setApplicationState(INIT);
  _view.init();
}

void theApp::run()
{
  _view.draw();
}

AppConfig theApp::getAppConfigValues()
{
  return _model.getApplicationConfig();
}

void theApp::setNewAppConfigValues(AppConfig newAppConfig)
{
  _model.setApplicationConfig(newAppConfig);
  _model.saveAppConfigToEEPROM();
}
