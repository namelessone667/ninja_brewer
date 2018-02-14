#ifndef BlynkPublisher_h
#define BlynkPublisher_h

#include "BasePublisher.h"
#include "secrets.h"

#define BLYNK_PUBLISH_INTERVAL 60000
#define BLYNK_CONNECTION_RETRY_INTERVAL 30000
#define BLYNK_CONNECTION_TIMEOUT 2000

class BlynkPublisher : public BasePublisher
{
  public:
    BlynkPublisher();
    void init(const Model&);
    void publish(const Model&);
  private:
    long _lastPublishTimestamp;
    long _lastReconnectTimestamp;
};

#endif
