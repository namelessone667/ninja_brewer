#ifndef ParticlePublisher_h
#define ParticlePublisher_h

#include "BasePublisher.h"

class ParticlePublisher : public BasePublisher
{
  public:
    void init(const Model&);
    void publish(const Model&);
};

#endif
