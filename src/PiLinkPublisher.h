#ifndef PiLinkPublisher_h_
#define PiLinkPublisher_h_

#include "PiLink.h"

class PiLinkPublisher : public BasePublisher
{
  public:
    PiLinkPublisher() {};

    void init(const NinjaModel& model) { PiLink::getInstance().init(); }; //TODO: subscribe on control constants and variables change and publish them

    void publish(const NinjaModel& model) { PiLink::getInstance().receive(); };
#ifdef DEBUG_HERMS
    void publish(const NinjaModel& model, double pTerm, double iTerm, double dTerm) { publish(model); };
#endif
  private:
};

#endif
