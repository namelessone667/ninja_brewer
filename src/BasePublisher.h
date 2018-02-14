#ifndef BasePublisher_h
#define BasePublisher_h

#include "Model.h"

class BasePublisher
{
  public:
    virtual void init(const Model&) = 0;
    virtual void publish(const Model&) = 0;
};

#endif
