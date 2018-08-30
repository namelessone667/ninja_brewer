#ifndef NinjaModel_h
#define NinjaModel_h

#include "application.h"
#include "PID_v1.h"
#include "Property.h"
#include "enum.h"
#include "globals.h"

class NinjaModel
{
public:
    Property<double> SetPoint; // target beer temp
    Property<double> Output; // main PID output temperature, also setpoint for heatPID
    Property<double> PeakEstimator;
    Property<double> PID_Kp;
    Property<double> PID_Ki;
    Property<double> PID_Kd;
    Property<int> PIDMode;
    Property<double> HeatOutput;
    Property<double> HeatPID_Kp;
    Property<double> HeatPID_Ki;
    Property<double> HeatPID_Kd;
    Property<int> HeatPIDMode;
    Property<bool> StandBy;

    Property<double> IdleDiff;          // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
    Property<double> PeakDiff;          // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
    Property<int> CoolMinOff;  // minimum compressor off time, seconds (5 min)
    Property<int> CoolMinOn;   // minimum compressor on time, seconds (1.5 min)
    Property<int> CoolMaxOn;   // maximum compressor on time, seconds (45 min)
    Property<int> PeakMaxTime; // maximum runTime to consider for peak estimation, seconds (20 min)
    Property<int> PeakMaxWait; // maximum wait on peak, seconds (30 min)
    Property<int> HeatMinOff;  // minimum HEAT off time, seconds (5 min)
    Property<int> HeatWindow; // window size for HEAT time proportioning, ms (5 min)
    Property<int> MinIdleTime;// minimum idle time between cool -> heat or heat -> cool
    Property<double> NoHeatBelow;     // dont turn on heating when tempretare is bellow this
    Property<double> NoCoolAbove;     // dont turn on cooling when tempretare is abowe this
    Property<opMode> ControllerMode;
    Property<double> MinTemperature;
    Property<double> MaxTemperature;
    Property<double> HeatMinPercent;
    Property<double> HeatMaxPercent;
    Property<double> HeatManualOutputPercent;

    Property<double> FridgeTemp = -127.0; //also input for heatPID
    Property<double> BeerTemp = -127.0; //also Input for mainPID
    Property<opState> ControllerState = IDLE;
    Property<ApplicationState> AppState = UNDEFINED;

    Property<bool> ExternalProfileActive; // true if Fermentrack temperature profile is active
#ifdef TEMP_PROFILES
    Property<double> TempProfileTemperature;
#endif
};

#endif
