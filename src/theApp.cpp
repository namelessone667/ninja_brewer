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
  Particle.connect();

  _model.loadAppConfigFromEEPROM();
  _model._appState.app_state = INIT;
  _view.init();

  if(_tempProxy.Init() < 0)
  {
      setErrorState("Sensor init fail");
      return;
  }

  _model._appState.app_state = RUNNING;
}

void theApp::run()
{
  switch(_model._appState.app_state)
  {
    case RUNNING:
      {
        int result = _tempProxy.ReadTemperatures();
        if(result < 0)
        {
          setErrorState(String::format("Sensor fail:%d", result));
        }
        else if(result == 1)
        {
          _model._appState.fridgeTemp = _tempProxy.GetFilteredTemperature(FRIDGE_TEMPERATURE);
          _model._appState.beerTemp = _tempProxy.GetFilteredTemperature(BEER_TEMPERATURE);
        }
      }
    default:
      _view.draw();
      break;
  }

}

const Model& theApp::getModel()
{
  return _model;
}

void theApp::setNewAppConfigValues(AppConfig newAppConfig)
{
  _model._appConfig = newAppConfig;
  _model.saveAppConfigToEEPROM();
}

void theApp::setErrorState(String error_message)
{
  _error = error_message;
  _model._appState.app_state = IN_ERROR;
}

String theApp::getErrorMessage()
{
  return _error;
}
