#include "theApp.h"
#include "DefaultNinjaModelSerializer.h"
#include "EEPROMNinjaModelSerializer.h"
#include "VirtualTempSensor.h"
#ifdef BREWPI_LINK
#include "PiLink.h"
#endif
//TODO create structures to store onewire device addresses
//TODO implement dynamic onewire device discovery and initialization
//TODO implement capability to add new onewire devices throug menu

theApp::theApp()
  :
    _model(),
    _view(this),
    _oneWire(ONE_WIRE_BUS_PIN),
    _log("ninja_brewer"),
    _controller(COOLER_SSR_PIN, HEATER_SSR_PIN),
    _mainPID(_model.BeerTemp, _model.Output, _model.SetPoint, _model.PID_Kp, _model.PID_Ki, _model.PID_Kd, PID_DIRECT),
    _heatPID(_model.FridgeTemp, _model.HeatOutput, _model.Output, _model.HeatPID_Kp, _model.HeatPID_Ki, _model.HeatPID_Kd, PID_DIRECT),
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
#ifdef TEMP_PROFILES
  bool LoadTempProfileRuntimeParameters = false;
#endif
  getLogger().info("initializing...");

#ifdef USE_PARTICLE
  if(WiFi.ready())
  {
    getLogger().info("connecting to Cloud");
    Particle.connect();
  }
#endif
  getLogger().info("loading configuration from EEPROM");
  EEPROMNinjaModelSerializer eepromSerializer;
  DefaultNinjaModelSerializer defaultSerializer;

  if(eepromSerializer.Load(_model) == false)
  {
    getLogger().info("loading configuration from EEPROM failed, loading default configuration");
    defaultSerializer.Load(_model);
#ifdef TEMP_PROFILES
    _tempProfile.ClearProfile();
#endif
  }
#ifdef TEMP_PROFILES
  else
  {
    if(eepromSerializer.LoadTempProfile(_tempProfile) == false)
      _tempProfile.ClearProfile();
    else
    {
      LoadTempProfileRuntimeParameters = true;
    }
  }

  if(LoadTempProfileRuntimeParameters)
  {
    bool isActive;
    int stepIndex;
    long timestamp;
    eepromSerializer.LoadTempProfileRuntimeParameters(isActive, stepIndex, timestamp);
    if(isActive)
    {
      _tempProfile.ActivateAtStep(stepIndex, millis() - timestamp);
    }
  }
#endif
  _model.AppState = INIT;
  getLogger().info("initializing UI");
  _view.init();
  getLogger().info("initializing sensors");

  int result = initSensors();

  if(result == 0)
  {
      getLogger().code(result).error("failed to initialize OneWire sensors");
      setErrorState("Sensor init fail");
      return;
  }
#ifndef SIMULATE_TEMP_SENSORS
  getLogger().info("Sensor1 address: " + _tempSensor1->GetAddress().ToString());
  getLogger().info("Sensor2 address: " + _tempSensor2->GetAddress().ToString());

  OneWireAddress lastSensorAddress1 = EEPROMNinjaModelSerializer::LoadSensorAddress(TEMP_SENSOR_ADDR1);
  OneWireAddress lastSensorAddress2 = EEPROMNinjaModelSerializer::LoadSensorAddress(TEMP_SENSOR_ADDR2);

  getLogger().info("Last sensor1 address: " + lastSensorAddress1.ToString());
  getLogger().info("Last sensor2 address: " + lastSensorAddress2.ToString());

  if(_tempSensor1->GetAddress() == lastSensorAddress2 && _tempSensor2->GetAddress() == lastSensorAddress1)
  {
    switchSensors();
    getLogger().info("Switching sensors");
  }

  EEPROMNinjaModelSerializer::SaveSensorAddress(_tempSensor1->GetAddress(), TEMP_SENSOR_ADDR1);
  EEPROMNinjaModelSerializer::SaveSensorAddress(_tempSensor2->GetAddress(), TEMP_SENSOR_ADDR2);
#endif
  //_mainPID.SetTunings(_model.PID_Kp, _model.PID_Ki, _model.PID_Kd);    // set tuning params
  _mainPID.SetSampleTime(1000);       // (ms) matches sample rate (1 hz)

  _mainPID.SetOutputLimits(_model.MinTemperature, _model.MaxTemperature);  // deg C (~32.5 - ~100 deg F)

  _mainPID.SetIntegratorClamping(true);

  _mainPID.setOutputType(PID_FILTERED);

  _mainPID.setFilterConstant(1000);

  _mainPID.SetMode(_model.PIDMode);  // set man/auto

  _model.PIDMode.ValueChanged.Subscribe(this, &theApp::handlePIDModeChanged);
  _model.MinTemperature.ValueChanged.Subscribe(this, &theApp::handleOutputLimitsChangedChanged);
  _model.MaxTemperature.ValueChanged.Subscribe(this, &theApp::handleOutputLimitsChangedChanged);

  _model.Output.Bind(_mainPID.Output);

  //_heatPID.SetTunings(_model.HeatPID_Kp, _model.HeatPID_Ki, _model.HeatPID_Kd);
  _heatPID.SetSampleTime(1000);       // sampletime = time proportioning window length

  _heatPID.SetOutputLimits(_model.HeatMinPercent, _model.HeatMaxPercent);  // _heatPID output = duty time per window

  _heatPID.SetIntegratorClamping(true);

  _heatPID.setOutputType(PID_FILTERED);

  _heatPID.setFilterConstant(10);

  _heatPID.SetMode(_model.HeatPIDMode);

  _model.HeatPIDMode.ValueChanged.Subscribe(this, &theApp::handleHeatPIDModeChanged);
#ifdef TEMP_PROFILES
  _tempProfile.TemperatureProfileStepsChanged.Subscribe(this, &theApp::handleTempProfileStepsChanged);
  eepromSerializer.ClearTempProfileRuntimeParameters();
#endif
  _publisherProxy.init(_model);

  _controller.Configure(_model);

  _model.ControllerMode.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  _model.IdleDiff.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  _model.PeakDiff.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  _model.CoolMinOff.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  _model.CoolMinOn.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  _model.CoolMaxOn.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  _model.PeakMaxTime.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  _model.PeakMaxWait.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  _model.HeatMinOff.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  _model.HeatWindow.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  _model.NoHeatBelow.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  _model.NoCoolAbove.ValueChanged.Subscribe(this, &theApp::handleControllerSettingsChanged);
  
#ifdef BREWPI_LINK
  PiLink::init();
#endif

  getLogger().info("initialization complete");
}

int theApp::initSensors()
{
  OneWireAddress sensorAddress;
  int retries = 10;
  while(true)
  {
    if(retries-- < 0)
      return 0;
#ifdef SIMULATE_TEMP_SENSORS
    _tempSensor1 = new VirtualTempSensor();
    _tempSensor2 = new VirtualTempSensor();
#else
    if(_tempSensor1 != NULL)
    {
      delete(_tempSensor1);
      _tempSensor1 = NULL;
    }
    if(_tempSensor2 != NULL)
    {
      delete(_tempSensor2);
      _tempSensor2 = NULL;
    }

    _oneWire.reset_search();

    if(_oneWire.search(sensorAddress.address) != 1)
      continue;

    _tempSensor1 = new DS18B20Sensor(sensorAddress, &_oneWire);

    if(_oneWire.search(sensorAddress.address) != 1)
      continue;

    _tempSensor2 = new DS18B20Sensor(sensorAddress, &_oneWire);
#endif

    if(_tempSensor1->Init() && _tempSensor2->Init())
    {
#ifdef HERMS_MODE
      _tempSensor1->SetFiltered(false);
      _tempSensor2->SetFiltered(false);
#else
      _tempSensor1->SetFiltered(true);
      _tempSensor2->SetFiltered(true);
#endif
      break;
    }
  }

  return 1;
}

void theApp::run()
{
  if(_reboot)
  {
#ifdef TEMP_PROFILES
    if(_tempProfile.IsActiveTemperatureProfile())
    {
      EEPROMNinjaModelSerializer serializer;
      serializer.SaveTempProfileRuntimeParameters(true, _tempProfile.GetCurrentStepIndex(), millis() - _tempProfile.GetCurrentStepStartTimestamp());
    }
#endif
    System.reset();
  }

  switch(_model.AppState)
  {
    case INIT:
      //try to read valid temperatures from sensors, when succes set application state to RUNNING
      if(readSensors())
      {
        _model.AppState = RUNNING;
        _pid_log_timestamp = millis();
        _sensorDataTimestamp = millis();
        _mainPID.initHistory();
        _mainPID.SetITerm(_model.SetPoint);
        _heatPID.initHistory();
        _heatPID.SetITerm(0);
      }
      break;
    case RUNNING:

      if(readSensors())
      {
        _sensorDataTimestamp = millis();
      }
      if(millis()-_sensorDataTimestamp > TEMP_ERR_INTERVAL)
      {
        getLogger().error(String::format("failed to read valid temperature for %d miliseconds", TEMP_ERR_INTERVAL));
        setErrorState("Sensor failure");
      }
#ifdef TEMP_PROFILES
      // Temperature profile functionality
      if(_tempProfile.IsActiveTemperatureProfile())
      {
        double temp;
        if(_tempProfile.GetCurrentTargetTemperature(temp))
        {
          _model.TempProfileTemperature = temp;
          _model.SetPoint = temp;
        }
      }
#endif
      if(_model.StandBy)
      {
        _controller.Disable();
      }
      else
      {
        _controller.Activate();

        if(_mainPID.GetMode() == PID_MANUAL)
        _mainPID.Output = _model.SetPoint;

        _mainPID.Compute();
        if(_model.ControllerState == HEAT)
          _heatPID.Compute();

        //TODO Create Binding _mainPID.output -> _model.Output
        //TODO Also for heat PID
        //  _model.Output = _model.SetPoint;
        //else
        //_model.Output = _mainPID.GetOutput();

        if(_heatPID.GetMode() == PID_AUTOMATIC)
          _model.HeatOutput = _heatPID.Output;
        else
          _model.HeatOutput = _model.HeatManualOutputPercent;
      }

      /*if(millis() - _pid_log_timestamp > 60000)
      {
        _pid_log_timestamp = millis();
        getLogger().info(String::format("PID p-term: %.4f, PID i-term: %.4f, PID output: %.4f", _mainPID.GetPTerm(), _mainPID.GetITerm(), _mainPID.Output.Get()));
        getLogger().info(String::format("Heat PID p-term: %.4f, Heat PID i-term: %.4f", _heatPID.GetPTerm(), _heatPID.GetITerm()));
      }*/

      _controller.Update(_model.FridgeTemp, _model.Output, _model.HeatOutput, _tempSensor1->PeakDetect());
      _model.ControllerState = _controller.GetState();
      if(_model.PeakEstimator.Get() != _controller.GetPeakEstimator())
      {
        _model.PeakEstimator = _controller.GetPeakEstimator();
        saveState();
      }
#ifdef DEBUG_HERMS
      _publisherProxy.publish(_model, _heatPID.GetPTerm(), _heatPID.GetITerm(), _heatPID.GetDTerm());
#else
      _publisherProxy.publish(_model);
#endif
      _view.draw();
#ifdef BREWPI_LINK
      PiLink::receive();
#endif
      break;
    case IN_ERROR:
      if(_error_timestamp < 0)
        _error_timestamp = millis();
      else if(millis() - _error_timestamp > 30000)
        reboot();

      _view.draw();
      break;
    default:
      _view.draw();
      break;
  }

}

bool theApp::readSensors()
{
  int result1 = _tempSensor1->ReadSensor();
  int result2 = _tempSensor2->ReadSensor();

  /*if(result1 != 0 || result2 != 0)
    getLogger().info("readSensor result: " + String(result1) + " / " + String(result2) + " / " + String((result1 == 1 && result2 == 1)));*/

  if(result1 == -1)
    _tempSensor1->Init();
  else if(result1 == -2)
    getLogger().code(result1).error("failed to read temperature from sensor 1");
  else if(result1 == 1)
    _model.FridgeTemp = _tempSensor1->GetValue();

  if(result2 == -1)
    _tempSensor2->Init();
  else if(result2 == -2)
    getLogger().code(result2).error("failed to read temperature from sensor 2");
  else if(result2 == 1)
    _model.BeerTemp = _tempSensor2->GetValue();

  return (result1 == 1 && result2 == 1);
}

NinjaModel& theApp::getModel()
{
  return _model;
}

const Logger& theApp::getLogger()
{
  return _log;
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
  _model.PIDMode = pid_mode;
  saveState();
}

void theApp::setHeatPID(int pid_mode, double new_output)
{
  _model.HeatPIDMode = pid_mode;
  if(pid_mode == PID_MANUAL && new_output >= _model.HeatMinPercent && new_output <= _model.HeatMaxPercent)
  {
    _model.HeatManualOutputPercent = new_output;
  }
  saveState();
}

void theApp::setNewTargetTemp(double new_setpoint)
{
  if(new_setpoint > _model.MaxTemperature || new_setpoint < _model.MinTemperature)
    return;

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
  getLogger().info("Saving state to EEPROM");
  EEPROMNinjaModelSerializer serializer;
  serializer.Save(_model);
#ifdef TEMP_PROFILES
  serializer.SaveTempProfile(_tempProfile);
#endif
  getLogger().info("Save state to EEPROM - done");
}

void theApp::switchSensors()
{
  IDS18B20Sensor *temp = _tempSensor1;
  _tempSensor1 = _tempSensor2;
  _tempSensor2 = temp;
#ifndef SIMULATE_TEMP_SENSORS
  EEPROMNinjaModelSerializer::SaveSensorAddress(_tempSensor1->GetAddress(), TEMP_SENSOR_ADDR1);
  EEPROMNinjaModelSerializer::SaveSensorAddress(_tempSensor2->GetAddress(), TEMP_SENSOR_ADDR2);
#endif
}

void theApp::handlePIDModeChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
{
  _mainPID.SetMode(((CValueChangedEventArgs<int>*)EvArgs)->NewValue());
}

void theApp::handleHeatPIDModeChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
{
  _heatPID.SetMode(((CValueChangedEventArgs<int>*)EvArgs)->NewValue());
}
#ifdef TEMP_PROFILES
TemperatureProfile& theApp::getTemperatureProfile()
{
  return _tempProfile;
}

void theApp::handleTempProfileStepsChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
{
  EEPROMNinjaModelSerializer serializer;
  serializer.SaveTempProfile(_tempProfile);
}
#endif

void theApp::handleOutputLimitsChangedChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
{
  _mainPID.SetOutputLimits(_model.MinTemperature, _model.MaxTemperature);  // deg C
}

const String& theApp::getLCDText()
{
  return _view.getLCDText();
}

void theApp::handleControllerSettingsChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
{
  _controller.Configure(_model);
}
