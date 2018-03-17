#include "theApp.h"

//TODO create structures to store onewire device addresses
//TODO implement dynamic onewire device discovery and initialization
//TODO implement capability to add new onewire devices throug menu

theApp::theApp()
  : _model(),
    _view(this),
    _oneWire(ONE_WIRE_BUS_PIN),
    _tempProxy(&_oneWire),
    _log("ninja_brewer"),
    _controller(COOLER_SSR_PIN, HEATER_SSR_PIN),
    _mainPID(&_model._appState.beerTemp, &_model._appConfig.output, &_model._appConfig.setpoint, 0, 0, 0, PID_DIRECT),
    _heatPID(&_model._appState.fridgeTemp, &_model._appConfig.heatOutput, &_model._appConfig.output, 0, 0, 0, PID_DIRECT),
    _reboot(false)
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

  _mainPID.SetTunings(_model._appConfig.pid_Kp, _model._appConfig.pid_Ki, _model._appConfig.pid_Kd);    // set tuning params
  _mainPID.SetSampleTime(1000);       // (ms) matches sample rate (1 hz)
  _mainPID.SetOutputLimits(MIN_FRIDGE_TEMP, MAX_FRIDE_TEMP);  // deg C (~32.5 - ~100 deg F)
  _mainPID.setOutputType(PID_FILTERED);
  _mainPID.setFilterConstant(1000);
  _mainPID.initHistory();
  _mainPID.SetMode(_model._appConfig.pid_mode);  // set man/auto
  _mainPID.SetITerm(_model._appConfig.setpoint);

  _heatPID.SetTunings(_model._appConfig.heatpid_Kp, _model._appConfig.heatpid_Ki, _model._appConfig.heatpid_Kd);
  _heatPID.SetSampleTime(1000);       // sampletime = time proportioning window length
  _heatPID.SetOutputLimits(HEAT_MIN_PERCENT, HEAT_MAX_PERCENT);  // _heatPID output = duty time per window
  _heatPID.SetMode(_model._appConfig.heatpid_mode);
  _heatPID.SetITerm(0);

  _publisherProxy.init(_model);

  _controller.Configure(_model._appConfig);

  getLogger().info("initialization complete");
}

void theApp::run()
{
  if(_reboot)
    System.reset();

  switch(_model._appState.app_state)
  {
    case INIT:
      //try to read valid temperatures from sensors, when succes set application state to RUNNING
      if(readSensors())
      {
        _model._appState.app_state = RUNNING;
        _pid_log_timestamp = millis();
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

        if(_model._appConfig.standBy)
        {
          _controller.Disable();
        }
        else
        {
          _controller.Activate();

          if(_mainPID.GetMode() == PID_MANUAL)
            _model._appConfig.output = _model._appConfig.setpoint;

          _mainPID.Compute();
          _heatPID.Compute();
        }

        if(millis() - _pid_log_timestamp > 60000)
        {
          _pid_log_timestamp = millis();
          getLogger().info(String::format("PID p-term: %.4f, PID i-term: %.4f", _mainPID.GetPTerm(), _mainPID.GetITerm()));
          getLogger().info(String::format("Heat PID p-term: %.4f, Heat PID i-term: %.4f", _heatPID.GetPTerm(), _heatPID.GetITerm()));
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

void theApp::ActivateController()
{
  _model._appConfig.standBy = false;
  saveState();
}

void theApp::DisableController()
{
  _model._appConfig.standBy = true;
  saveState();
}

void theApp::setPID(int pid_mode, double new_output)
{
  _mainPID.SetMode(pid_mode);
  _model._appConfig.pid_mode = pid_mode;
  if(pid_mode == PID_MANUAL)
  {
    _model._appConfig.output = new_output;

  }
  saveState();
}

void theApp::setHeatPID(int pid_mode, double new_output)
{
  _heatPID.SetMode(pid_mode);
  _model._appConfig.heatpid_mode = pid_mode;
  if(pid_mode == PID_MANUAL)
  {
    _model._appConfig.heatOutput = new_output;
  }
  saveState();
}

void theApp::setNewTargetTemp(double new_setpoint)
{
  _model._appConfig.setpoint = new_setpoint;
  saveState();
}

void theApp::reboot()
{
  _reboot = true;
}

void theApp::reinitLCD()
{
  _view.reinitLCD();
}

void theApp::saveState()
{
  _model.saveAppConfigToEEPROM();
}
