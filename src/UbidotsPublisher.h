#ifndef UbidotsPublisher_h
#define UbidotsPublisher_h

#include "BasePublisher.h"
#include "Ubidots.h"
#include "secrets.h"

#define DATA_SOURCE_NAME "Ninja Brewer"
#define DATA_SOURCE_KEY "ninja-brewer"
#define UBIDOTS_PUBLISH_INTERVAL 60000

class UbidotsPublisher : public BasePublisher
{
  public:
    UbidotsPublisher();
    void init(const NinjaModel&);
    void publish(const NinjaModel&);
#ifdef HERMS_MODE
    void publish(const NinjaModel&, double pTerm, double iTerm, double dTerm);
#endif
  private:
    long _lastPublishTimestamp;
    Ubidots _ubidots;
};

#endif
