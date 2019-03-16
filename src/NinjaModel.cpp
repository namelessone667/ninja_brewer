#include "NinjaModel.h"

NinjaModel::NinjaModel() :
  SetPoint(KEY_SETPOINT),
  Output(KEY_OUTPUT),
  PeakEstimator(KEY_PEAKESTIMATOR),
  PID_Kp(KEY_PIDKP),
  PID_Ki(KEY_PIDKI),
  PID_Kd(KEY_PIDKD),
  PID_IntegratorClampingError(KEY_PIDINTEGRCLAMPERR),
  PIDMode(KEY_PIDMODE),
  HeatOutput(KEY_HEATOUTPUT),
  HeatPID_Kp(KEY_HEATPIDKP),
  HeatPID_Ki(KEY_HEATPIDKI),
  HeatPID_Kd(KEY_HEATPIDKD),
  HeatPID_IntegratorClampingError(KEY_HEATPIDINTEGRCLAMPERR),
  HeatPIDMode(KEY_HEATPIDMODE),
  StandBy(KEY_STANDBY),
  IdleDiff(KEY_IDLEDIFF),
  PeakDiff(KEY_PEAKDIFF),
  CoolMinOff(KEY_COOLMINOFF),
  CoolMinOn(KEY_COOLMINON),
  CoolMaxOn(KEY_COOLMAXON),
  PeakMaxTime(KEY_PEAKMAXTIME),
  PeakMaxWait(KEY_PEAKMAXWAIT),
  HeatMinOff(KEY_HEATMINOFF),
  HeatWindow(KEY_HEATWINDOW),
  MinIdleTime(KEY_MINIDLETIME),
  NoHeatBelow(KEY_NOHEATBELOW),
  NoCoolAbove(KEY_NOCOOLABOVE),
  ControllerMode(KEY_CONTROLLERMODE),
  MinTemperature(KEY_MINTEMP),
  MaxTemperature(KEY_MAXTEMP),
  HeatMinPercent(KEY_HEATMINPERCENT),
  HeatMaxPercent(KEY_HEATMAXPERCENT),
  HeatManualOutputPercent(KEY_HEATMANUALOUTPUT),
  FridgeTemp(KEY_FRIDGETEMP, -127.0),
  BeerTemp(KEY_BEERTEMP, -127.0),
  ControllerState(KEY_CONTROLLERSTATE, IDLE),
  AppState(KEY_APPSTATE, UNDEFINED),
  ExternalProfileActive(KEY_EXTERNALTEMPPROFILEACTIVE),
  ConnectToCloud(KEY_CONNECTTOCLOUD)
{

}

void NinjaModel::ResetToDefaults()
{
  SetPoint = 18.0; // target beer temp
  Output = 18.0; // main PID output temperature, also setpoint for heatPID
  PeakEstimator = 30.0;
  PID_Kp = 3.0;
  PID_Ki = 5.0E-4;
  PID_Kd = 0.0;
  PID_IntegratorClampingError = 2; // dont modify integrator term when error is higher than +- 3C
  PIDMode = PID_MANUAL;
  HeatOutput = 0.0;
  HeatPID_Kp = 10.0;
  HeatPID_Ki = 0.001;
  HeatPID_Kd = 0.0;
  HeatPID_IntegratorClampingError = 5; // dont modify heat integrator term when error is higher than +- 3C
  HeatPIDMode = PID_MANUAL;
  StandBy = true;

  IdleDiff = 0.5;     // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
  PeakDiff = 0.25;      // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
  CoolMinOff = 600;     // minimum compressor off time, seconds (5 min)
  CoolMinOn = 120;  // minimum compressor on time, seconds (1.5 min)
  CoolMaxOn = 2700;     // maximum compressor on time, seconds (45 min)
  PeakMaxTime = 1200;   // maximum runTime to consider for peak estimation, seconds (20 min)
  PeakMaxWait = 1800;   // maximum wait on peak, seconds (30 min)
  HeatMinOff = 600;     // minimum HEAT off time, seconds (5 min)
  HeatWindow = 60;  // window size for HEAT time proportioning, ms (5 min)
  MinIdleTime = 120; // minimum idle time between cool -> heat or heat -> cool
  NoHeatBelow = 10;
  NoCoolAbove = 25;
  ControllerMode = COOLER_HEATER;

  MinTemperature = 0;
  MaxTemperature = 35;
  HeatMinPercent = 0;
  HeatMaxPercent = 50;
  HeatManualOutputPercent = 10;

  ExternalProfileActive = false;
#ifdef USE_PARTICLE
  ConnectToCloud = true;
#else
  ConnectToCloud = false;
#endif
}
