// This #include statement was automatically added by the Particle IDE.
#include "theApp.h"
#include "globals.h"
#include "secrets.h"
#include "papertrail.h"
#include "TemperatureProfile.h"
#include "EEPROMNinjaModelSerializer.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);
//STARTUP(WiFi.selectAntenna(ANT_AUTO));

PapertrailLogHandler papertailHandler(PAPERTAIL_SERVER, PAPERTAIL_PORT, PAPERTAIL_APPNAME, System.deviceID(), LOG_LEVEL);
//SerialLogHandler logHandler1;

void wd_reboot()
{
    if(theApp::getInstance().getModel().AppState == RUNNING)
    {
      theApp::getInstance().saveState();
      TemperatureProfile& tempProfile = theApp::getInstance().getTemperatureProfile();

      if(tempProfile.IsActiveTemperatureProfile())
      {
        EEPROMNinjaModelSerializer serializer;
        serializer.SaveTempProfileRuntimeParameters(true, tempProfile.GetCurrentStepIndex(), millis() - tempProfile.GetCurrentStepStartTimestamp());
      }
    }

    System.reset();
}

ApplicationWatchdog wd(60000, wd_reboot, 1024);

void setup()
{
    WiFi.connect();
    // wait for WiFi to connect (to be able to log to Papertail)
    long wifi_timeout = 10000;
    while(!WiFi.ready() || wifi_timeout-- > 0)
    {
      delay(1);
    }

    theApp::getInstance().init();

    wd.checkin();
}

void loop()
{
  theApp::getInstance().run();
  wd.checkin();
}
