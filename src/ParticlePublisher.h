#ifndef ParticlePublisher_h
#define ParticlePublisher_h

#include "BasePublisher.h"

class ParticlePublisher : public BasePublisher
{
  public:
    void init(const Model&);
    void publish(const Model&);

    static int setNewSetPoint(String);
    static int setPIDModeAuto(String);
    static int setPIDModeManual(String);
    static int setHeatPIDManual(String);
    static int setHeatPIDAuto(String);
    static int reboot(String);
    static int startControl(String);
    static int stopControl(String);
    static int saveState(String);
    static int getOneWireDevices(String);
    static int reinitLCD(String);
};

#endif
