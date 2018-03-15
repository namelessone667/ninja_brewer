#ifndef PublisherProxy_h
#define PublisherProxy_h

#include "BasePublisher.h"
#include <list>

class PublisherProxy
{
  public:
    void init(const NinjaModel&);
    void publish(const NinjaModel&);
  private:
    std::list<BasePublisher*> _publishers;
};

#endif
