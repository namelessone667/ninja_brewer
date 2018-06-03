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

  int result = initSensors();

  if(result == 0)
  {
      getLogger().code(result).error("failed to initialize OneWire sensors");
      setErrorState("Sensor init fail");
      return;
  }

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

  //_mainPID.SetTunings(_model.PID_Kp, _model.PID_Ki, _model.PID_Kd);    // set tuning params
  _mainPID.SetSampleTime(1000);       // (ms) matches sample rate (1 hz)
  _mainPID.SetOutputLimits(MIN_FRIDGE_TEMP, MAX_FRIDE_TEMP);  // deg C (~32.5 - ~100 deg F)
  _mainPID.setOutputType(PID_FILTERED);
  _mainPID.setFilterConstant(1000);
  _mainPID.initHistory();
  _mainPID.SetMode(_model.PIDMode);  // set man/auto
  _mainPID.SetITerm(_model.SetPoint);

  _model.PIDMode.ValueChanged.Subscribe(this, &theApp::handlePIDModeChanged);

  _model.Output.Bind(_mainPID.Output);

  //_heatPID.SetTunings(_model.HeatPID_Kp, _model.HeatPID_Ki, _model.HeatPID_Kd);
  _heatPID.SetSampleTime(1000);       // sampletime = time proportioning window length
  _heatPID.SetOutputLimits(HEAT_MIN_PERCENT, HEAT_MAX_PERCENT);  // _heatPID output = duty time per window
  _heatPID.SetMode(_model.HeatPIDMode);
  _heatPID.SetITerm(0);

  _model.HeatPIDMode.ValueChanged.Subscribe(this, &theApp::handleHeatPIDModeChanged);

  _publisherProxy.init(_model);

  _controller.Configure(_model);

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

    if(_tempSensor1->Init() && _tempSensor2->Init())
    {
      _tempSensor1->SetFiltered(true);
      _tempSensor2->SetFiltered(true);
      break;
    }
  }

  return 1;
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
        _sensorDataTimestamp = millis();
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
      }

      if(millis() - _pid_log_timestamp > 60000)
      {
        _pid_log_timestamp = millis();
        getLogger().info(String::format("PID p-term: %.4f, PID i-term: %.4f, PID output: %.4f", _mainPID.GetPTerm(), _mainPID.GetITerm(), _mainPID.Output.Get()));
        getLogger().info(String::format("Heat PID p-term: %.4f, Heat PID i-term: %.4f", _heatPID.GetPTerm(), _heatPID.GetITerm()));
      }

      _controller.Update(_model.FridgeTemp, _model.Output, _model.HeatOutput, _tempSensor1->PeakDetect());
      _model.ControllerState = _controller.GetState();
      _publisherProxy.publish(_model);
      _view.draw();
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
  getLogger().info("Saving state to EEPROM");
  EEPROMNinjaModelSerializer serializer;
  serializer.Save(_model);
  getLogger().info("Save state to EEPROM - done");
}

void theApp::switchSensors()
{
  DS18B20Sensor *temp = _tempSensor1;
  _tempSensor1 = _tempSensor2;
  _tempSensor2 = temp;

  EEPROMNinjaModelSerializer::SaveSensorAddress(_tempSensor1->GetAddress(), TEMP_SENSOR_ADDR1);
  EEPROMNinjaModelSerializer::SaveSensorAddress(_tempSensor2->GetAddress(), TEMP_SENSOR_ADDR2);
}

void theApp::handlePIDModeChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
{
  _mainPID.SetMode(((CValueChangedEventArgs<int>*)EvArgs)->NewValue());
}

void theApp::handleHeatPIDModeChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
{
  _heatPID.SetMode(((CValueChangedEventArgs<int>*)EvArgs)->NewValue());
}
