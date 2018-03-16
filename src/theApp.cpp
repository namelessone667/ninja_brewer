#include "theApp.h"
#include "DefaultNinjaModelSerializer.h"
#include "EEPROMNinjaModelSerializer.h"

//TODO create structures to store onewire device addresses
//TODO implement dynamic onewire device discovery and initialization
//TODO implement capability to add new onewire devices throug menu

theApp::theApp()
  :
    _model(),
    _view(this),
    _oneWire(ONE_WIRE_BUS_PIN),
    _tempProxy(&_oneWire),
    _log("ninja_brewer"),
    _controller(COOLER_SSR_PIN, HEATER_SSR_PIN),
    _mainPID(_model.BeerTemp, _model.Output, _model.SetPoint, 0, 0, 0, PID_DIRECT),
    _heatPID(_model.FridgeTemp, _model.HeatOutput, _model.Output, 0, 0, 0, PID_DIRECT),
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
  EEPROMNinjaModelSerializer eepromSerializer;
  DefaultNinjaModelSerializer defaultSerializer;

  if(eepromSerializer.Load(_model) == false)
  {
    getLogger().info("loading configuration from EEPROM failed, loading default configuration");
    defaultSerializer.Load(_model);
  }
  _model.AppState = INIT;
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

  _mainPID.SetTunings(_model.PID_Kp, _model.PID_Ki, _model.PID_Kd);    // set tuning params
  _mainPID.SetSampleTime(1000);       // (ms) matches sample rate (1 hz)
  _mainPID.SetOutputLimits(MIN_FRIDGE_TEMP, MAX_FRIDE_TEMP);  // deg C (~32.5 - ~100 deg F)
  _mainPID.setOutputType(PID_FILTERED);
  _mainPID.setFilterConstant(1000);
  _mainPID.initHistory();
  _mainPID.SetMode(_model.PIDMode);  // set man/auto
  _mainPID.SetITerm(_model.SetPoint);

  _heatPID.SetTunings(_model.HeatPID_Kp, _model.HeatPID_Ki, _model.HeatPID_Kd);
  _heatPID.SetSampleTime(1000);       // sampletime = time proportioning window length
  _heatPID.SetOutputLimits(HEAT_MIN_PERCENT, HEAT_MAX_PERCENT);  // _heatPID output = duty time per window
  _heatPID.SetMode(_model.HeatPIDMode);
  _heatPID.SetITerm(0);

  _publisherProxy.init(_model);

  _controller.Configure(_model);

  getLogger().info("initialization complete");
}

void theApp::run()
{
  if(_reboot)
    System.reset();

  switch(_model.AppState)
  {
    case INIT:
      //try to read valid temperatures from sensors, when succes set application state to RUNNING
      if(readSensors())
      {
        _model.AppState = RUNNING;
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

        if(_model.StandBy)
        {
          _controller.Disable();
        }
        else
        {
          _controller.Activate();

          if(_mainPID.GetMode() == PID_MANUAL)
            _model.Output = _model.SetPoint;

          _mainPID.Compute();
          _heatPID.Compute();
        }

        if(millis() - _pid_log_timestamp > 60000)
        {
          _pid_log_timestamp = millis();
          getLogger().info(String::format("PID p-term: %.4f, PID i-term: %.4f", _mainPID.GetPTerm(), _mainPID.GetITerm()));
          getLogger().info(String::format("Heat PID p-term: %.4f, Heat PID i-term: %.4f", _heatPID.GetPTerm(), _heatPID.GetITerm()));
        }

        _controller.Update(_model.FridgeTemp, _model.SetPoint, _model.HeatOutput, _tempProxy.PeakDetect(FRIDGE_TEMPERATURE));
        _model.ControllerState = _controller.GetState();
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
    _model.FridgeTemp = _tempProxy.GetFilteredTemperature(FRIDGE_TEMPERATURE);
    _model.BeerTemp = _tempProxy.GetFilteredTemperature(BEER_TEMPERATURE);
    return true;
  }
  return false;
}

NinjaModel& theApp::getModel()
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
  //_model._appConfig = newAppConfig;
  //_model.saveAppConfigToEEPROM();
}

void theApp::setErrorState(String error_message)
{
  _error = error_message;
  _model.AppState = IN_ERROR;
}

String theApp::getErrorMessage()
{
  return _error;
}

void theApp::ActivateController()
{
  _model.StandBy = false;
  saveState();
}

void theApp::DisableController()
{
  _model.StandBy = true;
  saveState();
}

void theApp::setPID(int pid_mode, double new_output)
{
  _mainPID.SetMode(pid_mode);
  _model.PIDMode = pid_mode;
  if(pid_mode == PID_MANUAL)
  {
    _model.Output = new_output;

  }
  saveState();
}

void theApp::setHeatPID(int pid_mode, double new_output)
{
  _heatPID.SetMode(pid_mode);
  _model.HeatPIDMode = pid_mode;
  if(pid_mode == PID_MANUAL)
  {
    _model.HeatOutput = new_output;
  }
  saveState();
}

void theApp::setNewTargetTemp(double new_setpoint)
{
  _model.SetPoint = new_setpoint;
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
  EEPROMNinjaModelSerializer serializer;
  serializer.Save(_model);
}
