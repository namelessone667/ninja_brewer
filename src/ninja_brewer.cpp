// This #include statement was automatically added by the Particle IDE.
#include "theApp.h"
#include "globals.h"
#include "secrets.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);
STARTUP(WiFi.selectAntenna(ANT_AUTO));

void wd_reboot()
{
    System.reset();
}

ApplicationWatchdog wd(60000, wd_reboot, 1024);

void setup()
{
    theApp::getInstance().init();

    wd.checkin();
}

void loop()
{
  theApp::getInstance().run();
  wd.checkin();
}
