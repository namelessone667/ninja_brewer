#ifndef ParticlePublisher_h
#define ParticlePublisher_h

#include "BasePublisher.h"

class ParticlePublisher : public BasePublisher
{
  private:
    bool registeredToCloud = false;
  public:
    void init(const NinjaModel&);
    void publish(const NinjaModel&);
#ifdef HERMS_MODE
    void publish(const NinjaModel&, double pTerm, double iTerm, double dTerm);
#endif

    static int setNewSetPoint(String);
    static int setPIDModeAuto(String);
    static int setPIDModeManual(String);
    static int setHeatPIDManual(String);
    static int setHeatPIDAuto(String);
    static int reboot(String);
    static int startControl(String);
    static int stopControl(String);
    static int saveState(String);
    static int reinitLCD(String);
    static int switchSensors(String);
};

#endif
