#include "theApp.h"

//TODO create structures to store onewire device addresses
//TODO implement dynamic onewire device discovery and initialization
//TODO implement capability to add new onewire devices throug menu

theApp::theApp() : _view(this), _oneWire(ONE_WIRE_BUS_PIN), _tempProxy(&_oneWire), _log("ninja_brewer"), _controller(COOLER_SSR_PIN, HEATER_SSR_PIN)
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
  getLogger().info("initializing...");
  getLogger().info("connecting to Cloud");
  Particle.connect();

  getLogger().info("loading configuration from EEPROM");
  _model.loadAppConfigFromEEPROM();
  _model._appState.app_state = INIT;
  getLogger().info("initializing UI");
  _view.init();
  getLogger().info("initializing sensors");
  int result = _tempProxy.Init();

  if(result < 0)
  {
      getLogger().code(result).error("failed to initialize OneWire sensors");
      setErrorState("Sensor init fail");
      return;
  }
  else if(result != 2)
  {
    getLogger().code(result).error(String::format("invalid number of detected sensors: %d, expected 2", result));
    setErrorState("Inv. sens. count");
    return;
  }

  _publisherProxy.init(_model);

  getLogger().info("initialization complete");
}

void theApp::run()
{
  switch(_model._appState.app_state)
  {
    case INIT:
      //try to read valid temperatures from sensors, when succes set application state to RUNNING
      if(readSensors())
      {
        _model._appState.app_state = RUNNING;
        _controller.Activate();
      }
      break;
    case RUNNING:
      {
        readSensors();
        if(_tempProxy.GetValidDataAge() > TEMP_ERR_INTERVAL)
        {
          getLogger().error(String::format("failed to read valid temperature for %d miliseconds", TEMP_ERR_INTERVAL));
          setErrorState("Sensor failure");
        }
        _controller.Update(_model._appState.fridgeTemp, _model._appConfig.output, _model._appConfig.heatOutput, _tempProxy.PeakDetect(FRIDGE_TEMPERATURE));
        _model._appState.controller_state = _controller.GetState();
        _publisherProxy.publish(_model);
      }
    default:
      _view.draw();
      break;
  }

}

bool theApp::readSensors()
{
  int result = _tempProxy.ReadTemperatures();
  if(result < 0)
  {
    getLogger().code(result).error("failed to read temperature from sensors");
    //setErrorState(String::format("Sensor fail:%d", result));
    return false;
  }
  else if(result == 1)
  {
    _model._appState.fridgeTemp = _tempProxy.GetFilteredTemperature(FRIDGE_TEMPERATURE);
    _model._appState.beerTemp = _tempProxy.GetFilteredTemperature(BEER_TEMPERATURE);
    return true;
  }
  return false;
}

const Model& theApp::getModel()
{
  return _model;
}

const Logger& theApp::getLogger()
{
  return _log;
}

void theApp::setNewAppConfigValues(AppConfig newAppConfig)
{
  getLogger().info("saving new configuration to EEPROM");
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
