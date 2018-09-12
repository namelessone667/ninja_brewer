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
  ExternalProfileActive(KEY_EXTERNALTEMPPROFILEACTIVE)
{
  //NinjaModelPropertyVariant v1 = &SetPoint;
  //_properties[KEY_SETPOINT, v1];
  /*_properties[SetPoint.getKey(), SetPoint];
  _properties[Output.getKey(), Output];
  _properties[PeakEstimator.getKey(), PeakEstimator];
  _properties[PID_Kp.getKey(), PID_Kp];
  _properties[PID_Ki.getKey(), PID_Ki];
  _properties[PID_Kd.getKey(), PID_Kd];
  _properties[PIDMode.getKey(), PIDMode];
  _properties[HeatOutput.getKey(), HeatOutput];
  _properties[HeatPID_Kp.getKey(), HeatPID_Kp];
  _properties[HeatPID_Ki.getKey(), HeatPID_Ki];
  _properties[HeatPID_Kd.getKey(), HeatPID_Kd];
  _properties[HeatPIDMode.getKey(), HeatPIDMode];
  _properties[StandBy.getKey(), StandBy];
  _properties[IdleDiff.getKey(), IdleDiff];
  _properties[PeakDiff.getKey(), PeakDiff];
  _properties[CoolMinOff.getKey(), CoolMinOff];
  _properties[CoolMinOn.getKey(), CoolMinOn];
  _properties[CoolMaxOn.getKey(), CoolMaxOn];
  _properties[PeakMaxTime.getKey(), PeakMaxTime];
  _properties[PeakMaxWait.getKey(), PeakMaxWait];
  _properties[HeatMinOff.getKey(), HeatMinOff];
  _properties[HeatWindow.getKey(), HeatWindow];
  _properties[MinIdleTime.getKey(), MinIdleTime];
  _properties[NoHeatBelow.getKey(), NoHeatBelow];
  _properties[NoCoolAbove.getKey(), NoCoolAbove];
  _properties[ControllerMode.getKey(), ControllerMode];
  _properties[MaxTemperature.getKey(), MaxTemperature];
  _properties[HeatMinPercent.getKey(), HeatMinPercent];
  _properties[HeatMaxPercent.getKey(), HeatMaxPercent];
  _properties[HeatManualOutputPercent.getKey(), HeatManualOutputPercent];
  _properties[FridgeTemp.getKey(), FridgeTemp];
  _properties[BeerTemp.getKey(), BeerTemp];
  _properties[ControllerState.getKey(), ControllerState];
  _properties[AppState.getKey(), AppState];
  _properties[ExternalProfileActive.getKey(), ExternalProfileActive];*/
}
