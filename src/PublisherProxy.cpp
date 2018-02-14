#include "PublisherProxy.h"
#include "ParticlePublisher.h"
#include "UbidotsPublisher.h"

void PublisherProxy::init(const Model &model)
{
  _publishers.push_back(new ParticlePublisher());
  _publishers.push_back(new UbidotsPublisher());

  for (BasePublisher *p : _publishers) {
      p->init(model);
  }
}

void PublisherProxy::publish(const Model &model)
{
  for (BasePublisher *p : _publishers) {
      p->publish(model);
  }
}
