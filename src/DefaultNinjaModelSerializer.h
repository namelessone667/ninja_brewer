#ifndef DefaultNinjaModelSerializer_h
#define DefaultNinjaModelSerializer_h

#include "INinjaModelSerializer.h"

class DefaultNinjaModelSerializer : INinjaModelSerializer
{
public:
  inline bool Load(NinjaModel& model)
  {
    model.SetPoint = 18.0; // target beer temp
    model.Output = 18.0; // main PID output temperature, also setpoint for heatPID
    model.PeakEstimator = 30.0;
    model.PID_Kp = 3.0;
    model.PID_Ki = 5.0E-4;
    model.PID_Kd = 0.0;
    model.PIDMode = PID_MANUAL;
    model.HeatOutput = 0.0;
    model.HeatPID_Kp = 3.0;
    model.HeatPID_Ki = 0.0025;
    model.HeatPID_Kd = 0.0;
    model.HeatPIDMode = PID_MANUAL;
    model.StandBy = true;

    model.IdleDiff = 0.5;     // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
    model.PeakDiff = 0.25;      // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
    model.CoolMinOff = 600;     // minimum compressor off time, seconds (5 min)
    model.CoolMinOn = 120;  // minimum compressor on time, seconds (1.5 min)
    model.CoolMaxOn = 2700;     // maximum compressor on time, seconds (45 min)
    model.PeakMaxTime = 1200;   // maximum runTime to consider for peak estimation, seconds (20 min)
    model.PeakMaxWait = 1800;   // maximum wait on peak, seconds (30 min)
    model.HeatMinOff = 600;     // minimum HEAT off time, seconds (5 min)
    model.HeatWindow = 60;  // window size for HEAT time proportioning, ms (5 min)
    model.MinIdleTime = 120; // minimum idle time between cool -> heat or heat -> cool
    model.NoHeatBelow = 10;
    model.NoCoolAbove = 25;
    model.ControllerMode = COOLER_HEATER;

    //model.FridgeTemp = -127.0; //also input for heatPID
    //model.BeerTemp = -127.0; //also Input for mainPID
    //model.AppState = UNDEFINED;
    //model.ControllerState = IDLE;

    return true;
  }

  inline bool Save(const NinjaModel& model)
  {
    return false;
  }
};

#endif
