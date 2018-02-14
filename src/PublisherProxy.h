#ifndef PublisherProxy_h
#define PublisherProxy_h

#include "BasePublisher.h"
#include <list>

class PublisherProxy
{
  public:
    void init(const Model&);
    void publish(const Model&);
  private:
    std::list<BasePublisher*> _publishers;
};

#endif
