#include "application.h"
#include "PID_v1.h"

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
};

struct AppState
{
  double fridgeTemp; //also input for heatPID
  double beerTemp; //also Input for mainPID
  ApplicationState app_state;
};

const struct AppConfig defaultAppConfig =
{
  .setpoint = 18.0, // target beer temp
  .output = 18.0, // main PID output temperature, also setpoint for heatPID
  .peakEstimator = 5.0,
  .pid_Kp = 3.0,
  .pid_Ki = 5.0E-4,
  .pid_Kd = 0.0,
  .pid_mode = PID_MANUAL,
  .heatOutput = 0.0,
  .heatpid_Kp = 3.0,
  .heatpid_Ki = 0.0025,
  .heatpid_Kd = 0.0,
  .heatpid_mode = PID_MANUAL,
  .standBy = true
};

const struct AppState defaultAppState =
{
  .fridgeTemp = -127.0, //also input for heatPID
  .beerTemp = -127.0, //also Input for mainPID
  .app_state = UNDEFINED
};

class Model
{
  public:
    Model();
    void loadAppConfigFromEEPROM();
    void saveAppConfigToEEPROM();
    AppState getApplicationState();
    AppConfig getApplicationConfig();
    void setApplicationState(ApplicationState);
  private:
    AppState _appState;
    AppConfig _appConfig;
};
