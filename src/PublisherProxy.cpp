#include "PublisherProxy.h"
#include "ParticlePublisher.h"
#include "UbidotsPublisher.h"
#include "BlynkPublisher.h"

void PublisherProxy::init(const NinjaModel &model)
{
  _publishers.push_back(new ParticlePublisher());
  //_publishers.push_back(new UbidotsPublisher());
  //_publishers.push_back(new BlynkPublisher());

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

#ifdef HERMS_MODE
void PublisherProxy::publish(const NinjaModel &model, double pTerm, double iTerm, double dTerm)
{
  for (BasePublisher *p : _publishers) {
      p->publish(model, pTerm, iTerm, dTerm);
  }
}
#endif
