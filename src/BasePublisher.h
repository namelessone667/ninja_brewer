#ifndef BasePublisher_h
#define BasePublisher_h

#include "NinjaModel.h"

class BasePublisher
{
  public:
    virtual void init(const NinjaModel&) = 0;
    virtual void publish(const NinjaModel&) = 0;
};

#endif
