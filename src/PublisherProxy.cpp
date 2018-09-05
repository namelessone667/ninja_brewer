#include "PublisherProxy.h"
#ifdef USE_PARTICLE
#include "ParticlePublisher.h"
#endif
#ifdef USE_UBIDOTS
#include "UbidotsPublisher.h"
#endif
#ifdef USE_BLYNK
#include "BlynkPublisher.h"
#endif
#ifdef BREWPI_LINK
#include "PiLinkPublisher.h"
#endif

void PublisherProxy::init(const NinjaModel &model)
{
#ifdef USE_PARTICLE
  _publishers.push_back(new ParticlePublisher());
#endif
#ifdef USE_UBIDOTS
  _publishers.push_back(new UbidotsPublisher());
#endif
#ifdef USE_BLYNK
  _publishers.push_back(new BlynkPublisher());
#endif
#ifdef BREWPI_LINK
  _publishers.push_back(new PiLinkPublisher());
#endif

  for (BasePublisher *p : _publishers) {
      p->init(model);
  }
}

void PublisherProxy::publish(const NinjaModel &model)
{
  for (BasePublisher *p : _publishers) {
      p->publish(model);
  }
}

#ifdef DEBUG_HERMS
void PublisherProxy::publish(const NinjaModel &model, double pTerm, double iTerm, double dTerm)
{
  for (BasePublisher *p : _publishers) {
      p->publish(model, pTerm, iTerm, dTerm);
  }
}
#endif
