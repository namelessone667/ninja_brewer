// This #include statement was automatically added by the Particle IDE.
#include "theApp.h"
#include "globals.h"
#include "secrets.h"
#include "papertrail.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);
//STARTUP(WiFi.selectAntenna(ANT_AUTO));

PapertrailLogHandler papertailHandler(PAPERTAIL_SERVER, PAPERTAIL_PORT, "ninja_brewer_2", System.deviceID(), LOG_LEVEL);
SerialLogHandler logHandler1;

void wd_reboot()
{
    System.reset();
}

ApplicationWatchdog wd(60000, wd_reboot, 1024);

void setup()
{
    WiFi.connect();
    // wait for WiFi to connect (to be able to log to Papertail)
    while(!WiFi.ready());

    theApp::getInstance().init();

    wd.checkin();
}

void loop()
{
  theApp::getInstance().run();
  wd.checkin();
}
