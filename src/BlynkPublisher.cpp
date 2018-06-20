#include "BlynkPublisher.h"
#include "application.h"
#include "globals.h"
#include "Blynk.h"
#include "theApp.h"

double BlynkPublisher::_newSetPoint;

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
  Blynk.virtualWrite(V1, _newSetPoint);
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

BLYNK_WRITE(V0)
{
// button ON/OFF pushed
  int btn_status = param.asInt();
  if(btn_status == 1)
    theApp::getInstance().ActivateController();
  else
    theApp::getInstance().DisableController();
}

BLYNK_WRITE(V1)
{
// new setpoint value
  BlynkPublisher::_newSetPoint = param.asDouble();
}

BLYNK_WRITE(V2)
{
// Set new setpoint button pushed
  int btn_status = param.asInt();
  if(btn_status == 1)
    BlynkPublisher::setNewSetPoint();
}
