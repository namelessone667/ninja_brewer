#include "BlynkPublisher.h"
#include "application.h"
#include "globals.h"
#include "Blynk.h"

BlynkPublisher::BlynkPublisher()
{
  _lastPublishTimestamp = millis() - BLYNK_PUBLISH_INTERVAL;
}

void BlynkPublisher::init(const NinjaModel &model)
{
  Blynk.config(BLYNK_AUTH);
  Blynk.connect(BLYNK_CONNECTION_TIMEOUT*3);
  _lastReconnectTimestamp = millis();
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

    _lastPublishTimestamp = now;
  }
}
