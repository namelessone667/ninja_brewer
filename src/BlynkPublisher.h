#ifndef BlynkPublisher_h
#define BlynkPublisher_h

#include "BasePublisher.h"
#include "secrets.h"

#ifdef HERMS_MODE
#define BLYNK_PUBLISH_INTERVAL 10000
#else
#define BLYNK_PUBLISH_INTERVAL 60000
#endif
#define BLYNK_CONNECTION_RETRY_INTERVAL 30000
#define BLYNK_CONNECTION_TIMEOUT 2000

class BlynkPublisher : public BasePublisher
{
  public:
    BlynkPublisher();
    void init(const NinjaModel&);
    void publish(const NinjaModel&);
#ifdef HERMS_MODE
    void publish(const NinjaModel&, double pTerm, double iTerm, double dTerm);
#endif
    static double _newSetPoint;
    static void setNewSetPoint();
  private:
    long _lastPublishTimestamp;
    long _lastReconnectTimestamp;
};

#endif
