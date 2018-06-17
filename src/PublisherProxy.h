#ifndef PublisherProxy_h
#define PublisherProxy_h

#include "BasePublisher.h"
#include <list>

class PublisherProxy
{
  public:
    void init(const NinjaModel&);
    void publish(const NinjaModel&);
#ifdef HERMS_MODE
    void publish(const NinjaModel&, double pTerm, double iTerm, double dTerm);
#endif
  private:
    std::list<BasePublisher*> _publishers;
};

#endif
