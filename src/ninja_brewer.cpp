// This #include statement was automatically added by the Particle IDE.
#include "theApp.h"
#include "globals.h"
#include "secrets.h"
#ifdef USE_PAPERTAIL
#include "papertrail.h"
#endif
#ifdef TEMP_PROFILES
#include "TemperatureProfile.h"
#endif
#include "EEPROMNinjaModelSerializer.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);
//STARTUP(WiFi.selectAntenna(ANT_AUTO));
#ifdef USE_PAPERTAIL
PapertrailLogHandler papertailHandler(PAPERTAIL_SERVER, PAPERTAIL_PORT, "ninja_brewer_2", System.deviceID(), LOG_LEVEL);
#endif
//SerialLogHandler logHandler1;

void wd_reboot()
{
    if(theApp::getInstance().getModel().AppState == RUNNING)
    {
      theApp::getInstance().saveState();
#ifdef TEMP_PROFILES
      TemperatureProfile& tempProfile = theApp::getInstance().getTemperatureProfile();

      if(tempProfile.IsActiveTemperatureProfile())
      {
        EEPROMNinjaModelSerializer serializer;
        serializer.SaveTempProfileRuntimeParameters(true, tempProfile.GetCurrentStepIndex(), millis() - tempProfile.GetCurrentStepStartTimestamp());
      }
#endif
    }

    System.reset();
}

ApplicationWatchdog wd(60000, wd_reboot, 1024);

void setup()
{
#ifdef USE_WIFI
    WiFi.connect();
    // wait for WiFi to connect (to be able to log to Papertail)
    long wifi_timeout = 15000;
    waitFor(WiFi.ready, wifi_timeout);
#endif
    theApp::getInstance().init();

    wd.checkin();
}

void loop()
{
  theApp::getInstance().run();
  wd.checkin();
}
