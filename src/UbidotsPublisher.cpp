#include "UbidotsPublisher.h"
#include "application.h"

UbidotsPublisher::UbidotsPublisher() : _ubidots(UBIDOTS_TOKEN)
{
  _lastPublishTimestamp = millis() - UBIDOTS_PUBLISH_INTERVAL;
}

void UbidotsPublisher::init(const NinjaModel &model)
{
  _ubidots.setDatasourceName(DATA_SOURCE_NAME);
  _ubidots.setDatasourceTag(DATA_SOURCE_KEY);
}

void UbidotsPublisher::publish(const NinjaModel &model)
{
  long now = millis();
  if(_lastPublishTimestamp + UBIDOTS_PUBLISH_INTERVAL < now)
  {
    _ubidots.add("beertemperature", model.BeerTemp);
    _ubidots.add("fridgetemperature", model.FridgeTemp);
    _ubidots.add("targetbeertemperature", model.SetPoint);
    _ubidots.add("pidoutputtemperature", model.Output);
    _ubidots.add("heatpidoutput", model.HeatOutput);

    _ubidots.sendAll();

    _lastPublishTimestamp = now;
  }
}
