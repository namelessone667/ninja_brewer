#include "PublisherProxy.h"
#include "ParticlePublisher.h"

void PublisherProxy::init(const Model &model)
{
  ParticlePublisher *p1 = new ParticlePublisher();
  p1->init(model);
  _publishers.push_back(p1);
}

void PublisherProxy::publish(const Model &model)
{
  // Iterate and print values of the list
    for (BasePublisher *p : _publishers) {
        p->publish(model);
    }
}
