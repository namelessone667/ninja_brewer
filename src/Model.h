#ifndef Model_h
#define Model_h

#include "application.h"
#include "PID_v1.h"
#include "CoolerHeaterContoller.h"

enum ApplicationState
{
  INIT,
  RUNNING,
  STAND_BY,
  IN_ERROR,
  UNDEFINED = -1
};

struct AppConfig
{
  double setpoint; // target beer temp
  double output; // main PID output temperature, also setpoint for heatPID
  double peakEstimator;
  double pid_Kp;
  double pid_Ki;
  double pid_Kd;
  int pid_mode;
  double heatOutput;
  double heatpid_Kp;
  double heatpid_Ki;
  double heatpid_Kd;
  int heatpid_mode;
  bool standBy;

  double idleDiff;          // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
  double peakDiff;          // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
  int coolMinOff;  // minimum compressor off time, seconds (5 min)
  int coolMinOn;   // minimum compressor on time, seconds (1.5 min)
  int coolMaxOn;   // maximum compressor on time, seconds (45 min)
  int peakMaxTime; // maximum runTime to consider for peak estimation, seconds (20 min)
  int peakMaxWait; // maximum wait on peak, seconds (30 min)
  int heatMinOff;  // minimum HEAT off time, seconds (5 min)
  int heatWindow; // window size for HEAT time proportioning, ms (5 min)
  int minIdleTime;// minimum idle time between cool -> heat or heat -> cool
  double no_heat_below;     // dont turn on heating when tempretare is bellow this
  double no_cool_above;     // dont turn on cooling when tempretare is abowe this
  opMode controller_mode;
};

struct AppState
{
  double fridgeTemp; //also input for heatPID
  double beerTemp; //also Input for mainPID
  ApplicationState app_state;
  opState controller_state;
};

const struct AppConfig defaultAppConfig =
{
  .setpoint = 18.0, // target beer temp
  .output = 18.0, // main PID output temperature, also setpoint for heatPID
  .peakEstimator = 30.0,
  .pid_Kp = 3.0,
  .pid_Ki = 5.0E-4,
  .pid_Kd = 0.0,
  .pid_mode = PID_MANUAL,
  .heatOutput = 0.0,
  .heatpid_Kp = 3.0,
  .heatpid_Ki = 0.0025,
  .heatpid_Kd = 0.0,
  .heatpid_mode = PID_MANUAL,
  .standBy = true,

  .idleDiff = 0.5,     // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
  .peakDiff = 0.25,      // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
  .coolMinOff = 600,     // minimum compressor off time, seconds (5 min)
  .coolMinOn = 60,  // minimum compressor on time, seconds (1.5 min)
  .coolMaxOn = 2700,     // maximum compressor on time, seconds (45 min)
  .peakMaxTime = 1200,   // maximum runTime to consider for peak estimation, seconds (20 min)
  .peakMaxWait = 1800,   // maximum wait on peak, seconds (30 min)
  .heatMinOff = 600,     // minimum HEAT off time, seconds (5 min)
  .heatWindow = 60,  // window size for HEAT time proportioning, ms (5 min)
  .minIdleTime = 120, // minimum idle time between cool -> heat or heat -> cool
  .no_heat_below = 10,
  .no_cool_above = 25,
  .controller_mode = COOLER_HEATER
};

const struct AppState defaultAppState =
{
  .fridgeTemp = -127.0, //also input for heatPID
  .beerTemp = -127.0, //also Input for mainPID
  .app_state = UNDEFINED,
  .controller_state = IDLE
};

class Model
{
  public:
    Model();
    void loadAppConfigFromEEPROM();
    void saveAppConfigToEEPROM();
    //AppState getApplicationState();
    //AppConfig getApplicationConfig();
    //void setApplicationState(ApplicationState);
    //void setApplicationConfig(AppConfig newAppConfig);
    AppState _appState;
    AppConfig _appConfig;
  private:

};

#endif
