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
  Log.trace("initializing...");
  Log.trace("connecting to Cloud");
  Particle.connect();

  Log.trace("loading configuration from EEPROM");
  _model.loadAppConfigFromEEPROM();
  _model._appState.app_state = INIT;
  Log.trace("initializing UI");
  _view.init();
  Log.trace("initializing sensors");
  int result = _tempProxy.Init();
  if(result < 0)
  {
      Log.code(result).error("failed to initialize OneWire sensors");
      setErrorState("Sensor init fail");
      return;
  }

  _model._appState.app_state = RUNNING;
  Log.trace("initialization complete");
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
          Log.code(result).error("failed to read temperature from sensors");
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
  Log.trace("saving new configuration to EEPROM");
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
