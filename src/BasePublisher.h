#ifndef BasePublisher_h
#define BasePublisher_h

#include "NinjaModel.h"
#include "globals.h"

class BasePublisher
{
  public:
    virtual void init(const NinjaModel&) = 0;
    virtual void publish(const NinjaModel&) = 0;
#ifdef DEBUG_HERMS
    virtual void publish(const NinjaModel&, double, double, double) = 0;
#endif
};

#endif
