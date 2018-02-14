#include "UbidotsPublisher.h"
#include "application.h"

UbidotsPublisher::UbidotsPublisher() : _ubidots(UBIDOTS_TOKEN)
{
  _lastPublishTimestamp = millis() - UBIDOTS_PUBLISH_INTERVAL;
}

void UbidotsPublisher::init(const Model &model)
{
  _ubidots.setDatasourceName(DATA_SOURCE_NAME);
  _ubidots.setDatasourceTag(DATA_SOURCE_KEY);
}

void UbidotsPublisher::publish(const Model &model)
{
  long now = millis();
  if(_lastPublishTimestamp + UBIDOTS_PUBLISH_INTERVAL < now)
  {
    _ubidots.add("beertemperature", model._appState.beerTemp);
    _ubidots.add("fridgetemperature", model._appState.fridgeTemp);
    _ubidots.add("targetbeertemperature", model._appConfig.setpoint);
    _ubidots.add("pidoutputtemperature", model._appConfig.output);
    _ubidots.add("heatpidoutput", model._appConfig.heatOutput);

    _ubidots.sendAll();

    _lastPublishTimestamp = now;
  }
}
