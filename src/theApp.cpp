#include "theApp.h"

//TODO create structures to store onewire device addresses
//TODO implement dynamic onewire device discovery and initialization
//TODO implement capability to add new onewire devices throug menu

theApp::theApp() : _view(this), _oneWire(ONE_WIRE_BUS_PIN), _tempProxy(&_oneWire)
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
  getModel()._appState.app_state = INIT;
  _view.init();
  _tempProxy.Init();

  getModel()._appState.app_state = INIT;
}

void theApp::run()
{
  if(_tempProxy.ReadTemperatures() == 1)
  {
    getModel()._appState.fridgeTemp = _tempProxy.GetFilteredTemperature(FRIDGE_TEMPERATURE);
    getModel()._appState.beerTemp = _tempProxy.GetFilteredTemperature(BEER_TEMPERATURE);
  }

  _view.draw();
}

Model& theApp::getModel()
{
  return _model;
}

/*AppConfig theApp::getAppConfigValues()
{
  return _model.getApplicationConfig();
}

void theApp::setNewAppConfigValues(AppConfig newAppConfig)
{
  _model.setApplicationConfig(newAppConfig);
  _model.saveAppConfigToEEPROM();
}

AppState theApp::getAppStateValues()
{
  return _model.getApplicationState();
}*/
