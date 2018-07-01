#include "BlynkPublisher.h"
#include "application.h"
#include "globals.h"
#include "theApp.h"
#include "Blynk.h"

double BlynkPublisher::_newSetPoint;
double BlynkPublisher::_stepTemperature;
long BlynkPublisher::_stepDuration;
TemperatureProfileStepDuration BlynkPublisher::_stepDurationUnit;
int BlynkPublisher::_id = 0;
int BlynkPublisher::_stepType = 0;

BlynkPublisher::BlynkPublisher()
{
  _lastPublishTimestamp = millis() - BLYNK_PUBLISH_INTERVAL;
}

void BlynkPublisher::init(const NinjaModel &model)
{
  Blynk.config(BLYNK_AUTH);
  Blynk.connect(BLYNK_CONNECTION_TIMEOUT*3);
  _lastReconnectTimestamp = millis();
  _newSetPoint = model.SetPoint;
  Blynk.syncVirtual(PIN_NEW_SETPOINT, PIN_STEP_TEMPERATURE, PIN_STEP_DURATION, PIN_STEP_DURATION_UNIT, PIN_TEMPROFILESTEP_TYPE);
  Blynk.virtualWrite(V1, _newSetPoint);
  Blynk.virtualWrite(PIN_TEMPROFILE_ON_OFF_BTN, theApp::getInstance().getTemperatureProfile().IsActiveTemperatureProfile() ? 1 : 0);

  WidgetLED _ledStatus(PIN_STATUS_LED);
  _ledStatus.off();

}

void BlynkPublisher::publish(const NinjaModel &model)
{
  if(!WiFi.ready())
    return;


  long now = millis();

  if(!Blynk.connected())
  {
    if(_lastReconnectTimestamp + BLYNK_CONNECTION_RETRY_INTERVAL > now)
      return;

    _lastReconnectTimestamp = now;
    if(!Blynk.connect(BLYNK_CONNECTION_TIMEOUT))
      return;
  }

  Blynk.run();

  if(_lastPublishTimestamp + BLYNK_PUBLISH_INTERVAL < now)
  {
    Blynk.virtualWrite(PIN_FRIDGE_TEMP, model.FridgeTemp);
    Blynk.virtualWrite(PIN_BEER_TEMP, model.BeerTemp);
    Blynk.virtualWrite(PIN_PID_OUTPUT, model.Output);
    Blynk.virtualWrite(PIN_PID_SETPOINT, model.SetPoint);
    Blynk.virtualWrite(PIN_HEATPID_OUTPUT, model.HeatOutput);
    Blynk.virtualWrite(PIN_BTN_ON_OFF, model.StandBy ? 0 : 1);

    WidgetLED _ledStatus(PIN_STATUS_LED);
    if(model.StandBy)
    {
      _ledStatus.off();
    }
    else
    {
      _ledStatus.on();
      switch(model.ControllerState)
      {
        case COOL:
          Blynk.setProperty(PIN_STATUS_LED, "color", "#00BFFF"); //blue
          break;
        case HEAT:
          Blynk.setProperty(PIN_STATUS_LED, "color", "#D3435C"); //red
          break;
        default:
          Blynk.setProperty(PIN_STATUS_LED, "color", "#F5F5DC"); //beige
          break;
      }
    }

    Blynk.virtualWrite(PIN_TEMP_PROFILE, model.TempProfileTemperature);
    Blynk.virtualWrite(PIN_TEMPROFILE_ON_OFF_BTN, theApp::getInstance().getTemperatureProfile().IsActiveTemperatureProfile() ? 1 : 0);

    _lastPublishTimestamp = now;
  }
}

#ifdef HERMS_MODE
void BlynkPublisher::publish(const NinjaModel &model, double pTerm, double iTerm, double dTerm)
{
  long timestamp = _lastPublishTimestamp;
  publish(model);
  if(_lastPublishTimestamp != timestamp)
  {
    Blynk.virtualWrite(HPID_PTERM, pTerm);
    Blynk.virtualWrite(HPID_ITERM, iTerm);
    Blynk.virtualWrite(HPID_DTERM, dTerm);
  }
}
#endif

void BlynkPublisher::setNewSetPoint()
{
  theApp::getInstance().setNewTargetTemp(_newSetPoint);
}

void BlynkPublisher::addTemperatureProfileStep()
{

  String unit;
  switch(_stepDurationUnit)
  {
    case SECONDS:
      unit = "seconds";
      break;
    case MINUTES:
      unit = "minutes";
      break;
    case HOURS:
      unit = "hours";
      break;
    case DAYS:
      unit = "days";
      break;
  }

  switch(_stepType)
  {
    case 1:
      theApp::getInstance().getTemperatureProfile().AddProfileStep<ConstantTemperatureProfileStepType>(_stepTemperature, _stepDuration, _stepDurationUnit);
      Blynk.virtualWrite(PIN_TEMPROFILE_TABLE, "add", _id, String::format("%.2fC @ %d ", _stepTemperature, _stepDuration)+unit+" (constant)", _id+1);
      break;
    case 2:
      theApp::getInstance().getTemperatureProfile().AddProfileStep<LinearTemperatureProfileStepType>(_stepTemperature, _stepDuration, _stepDurationUnit);
      Blynk.virtualWrite(PIN_TEMPROFILE_TABLE, "add", _id, String::format("%.2fC @ %d ", _stepTemperature, _stepDuration)+unit+" (linear)", _id+1);
      break;
    default:
      return;
  }
  _id++;
}

void BlynkPublisher::clearTemperatureProfile()
{
  theApp::getInstance().getTemperatureProfile().ClearProfile();
  Blynk.virtualWrite(PIN_TEMPROFILE_TABLE, "clr");
  Blynk.virtualWrite(PIN_TEMPROFILE_ON_OFF_BTN, theApp::getInstance().getTemperatureProfile().IsActiveTemperatureProfile() ? 1 : 0);
  _id = 0;
}

void BlynkPublisher::activateTemperatureProfile()
{
  theApp::getInstance().getTemperatureProfile().ActivateTemperatureProfile();
}

void BlynkPublisher::disableTemperatureProfile()
{
  theApp::getInstance().getTemperatureProfile().DeactivateTemperatureProfile();
}

BLYNK_WRITE(PIN_BTN_ON_OFF)
{
// button ON/OFF pushed
  int btn_status = param.asInt();
  if(btn_status == 1)
    theApp::getInstance().ActivateController();
  else
    theApp::getInstance().DisableController();
}

BLYNK_WRITE(PIN_NEW_SETPOINT)
{
// new setpoint value
  BlynkPublisher::_newSetPoint = param.asDouble();
}

BLYNK_WRITE(PIN_BTN_SET_NEW_SETPOINT)
{
// Set new setpoint button pushed
  int btn_status = param.asInt();
  if(btn_status == 1)
    BlynkPublisher::setNewSetPoint();
}

BLYNK_WRITE(PIN_STEP_TEMPERATURE)
{
  BlynkPublisher::_stepTemperature = param.asDouble();
}

BLYNK_WRITE(PIN_STEP_DURATION)
{
  BlynkPublisher::_stepDuration = param.asInt();
}

BLYNK_WRITE(PIN_STEP_DURATION_UNIT)
{
  BlynkPublisher::_stepDurationUnit = (TemperatureProfileStepDuration)param.asInt();
}

BLYNK_WRITE(PIN_ADD_STEP_BTN)
{
  int btn_status = param.asInt();
  if(btn_status == 1)
    BlynkPublisher::addTemperatureProfileStep();
}

BLYNK_WRITE(PIN_CLEAR_STEPS_BTN)
{
  int btn_status = param.asInt();
  if(btn_status == 1)
    BlynkPublisher::clearTemperatureProfile();
}

BLYNK_WRITE(PIN_TEMPROFILE_ON_OFF_BTN)
{
  int btn_status = param.asInt();
  if(btn_status == 1)
    BlynkPublisher::activateTemperatureProfile();
  else
    BlynkPublisher::disableTemperatureProfile();
}

BLYNK_WRITE(PIN_TEMPROFILESTEP_TYPE)
{
  BlynkPublisher::_stepType = param.asInt();
}
