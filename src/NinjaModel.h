#ifndef NinjaModel_h
#define NinjaModel_h

#include "application.h"
#include "PID_v1.h"
#include "Property.h"
#include "enum.h"
#include "globals.h"
#include <map>
//#include <variant>
//#include "boost/variant.hpp"

//define a key for every Property
//make a collection (map) of propetries in model

enum NinjaModelPropertyKey
{
  KEY_SETPOINT,
  KEY_OUTPUT,
  KEY_PEAKESTIMATOR,
  KEY_PIDKP,
  KEY_PIDKI,
  KEY_PIDKD,
  KEY_PIDMODE,
  KEY_HEATOUTPUT,
  KEY_HEATPIDKP,
  KEY_HEATPIDKI,
  KEY_HEATPIDKD,
  KEY_HEATPIDMODE,
  KEY_STANDBY,
  KEY_IDLEDIFF,
  KEY_PEAKDIFF,
  KEY_COOLMINOFF,
  KEY_COOLMINON,
  KEY_COOLMAXON,
  KEY_PEAKMAXTIME,
  KEY_PEAKMAXWAIT,
  KEY_HEATMINOFF,
  KEY_HEATWINDOW,
  KEY_MINIDLETIME,
  KEY_NOHEATBELOW,
  KEY_NOCOOLABOVE,
  KEY_CONTROLLERMODE,
  KEY_MINTEMP,
  KEY_MAXTEMP,
  KEY_HEATMINPERCENT,
  KEY_HEATMAXPERCENT,
  KEY_HEATMANUALOUTPUT,
  KEY_FRIDGETEMP,
  KEY_BEERTEMP,
  KEY_CONTROLLERSTATE,
  KEY_APPSTATE,
  KEY_EXTERNALTEMPPROFILEACTIVE
};

template<typename T, NinjaModelPropertyKey key>
class NinjaModelProperty : public Property<T>
{
public:
  NinjaModelProperty()
  {
    Property<T>();
  }

  NinjaModelProperty(const T value) : Property<T>(value) { };

  NinjaModelProperty(const Property<T>& value) : Property<T>(value.Get()) { };

  NinjaModelProperty(const NinjaModelProperty& value) : Property<T>(value.Get()) { };

  NinjaModelPropertyKey getKey() const { return key; };

  NinjaModelProperty& operator=(const T& other)
  {
    Property<T>::operator= (other);
    return *this;
  }

  NinjaModelProperty& operator=(const Property<T>& other)
  {
    Property<T>::operator= (other);
    return *this;
  }
};

class NinjaModel
{
  // typedef std::variant< NinjaModelProperty<double, NinjaModelPropertyKey>,
  //                       NinjaModelProperty<int, NinjaModelPropertyKey>,
  //                       NinjaModelProperty<bool, NinjaModelPropertyKey>,
  //                       NinjaModelProperty<opMode, NinjaModelPropertyKey>,
  //                       NinjaModelProperty<opState, NinjaModelPropertyKey>,
  //                       NinjaModelProperty<ApplicationState, NinjaModelPropertyKey>> NinjaModelPropertyVariant;
public:
    NinjaModelProperty<double, KEY_SETPOINT> SetPoint; // target beer temp
    NinjaModelProperty<double, KEY_OUTPUT> Output; // main PID output temperature, also setpoint for heatPID
    NinjaModelProperty<double, KEY_PEAKESTIMATOR> PeakEstimator;
    NinjaModelProperty<double, KEY_PIDKP> PID_Kp;
    NinjaModelProperty<double, KEY_PIDKI> PID_Ki;
    NinjaModelProperty<double, KEY_PIDKD> PID_Kd;
    NinjaModelProperty<int, KEY_PIDMODE> PIDMode;
    NinjaModelProperty<double, KEY_HEATOUTPUT> HeatOutput;
    NinjaModelProperty<double, KEY_HEATPIDKP> HeatPID_Kp;
    NinjaModelProperty<double, KEY_HEATPIDKI> HeatPID_Ki;
    NinjaModelProperty<double, KEY_HEATPIDKD> HeatPID_Kd;
    NinjaModelProperty<int, KEY_HEATPIDMODE> HeatPIDMode;
    NinjaModelProperty<bool, KEY_STANDBY> StandBy;

    NinjaModelProperty<double, KEY_IDLEDIFF> IdleDiff;          // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
    NinjaModelProperty<double, KEY_PEAKDIFF> PeakDiff;          // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
    NinjaModelProperty<int, KEY_COOLMINOFF> CoolMinOff;  // minimum compressor off time, seconds (5 min)
    NinjaModelProperty<int, KEY_COOLMINON> CoolMinOn;   // minimum compressor on time, seconds (1.5 min)
    NinjaModelProperty<int, KEY_COOLMAXON> CoolMaxOn;   // maximum compressor on time, seconds (45 min)
    NinjaModelProperty<int, KEY_PEAKMAXTIME> PeakMaxTime; // maximum runTime to consider for peak estimation, seconds (20 min)
    NinjaModelProperty<int, KEY_PEAKMAXWAIT> PeakMaxWait; // maximum wait on peak, seconds (30 min)
    NinjaModelProperty<int, KEY_HEATMINOFF> HeatMinOff;  // minimum HEAT off time, seconds (5 min)
    NinjaModelProperty<int, KEY_HEATWINDOW> HeatWindow; // window size for HEAT time proportioning, ms (5 min)
    NinjaModelProperty<int, KEY_MINIDLETIME> MinIdleTime;// minimum idle time between cool -> heat or heat -> cool
    NinjaModelProperty<double, KEY_NOHEATBELOW> NoHeatBelow;     // dont turn on heating when tempretare is bellow this
    NinjaModelProperty<double, KEY_NOCOOLABOVE> NoCoolAbove;     // dont turn on cooling when tempretare is abowe this
    NinjaModelProperty<opMode, KEY_CONTROLLERMODE> ControllerMode;
    NinjaModelProperty<double, KEY_MINTEMP> MinTemperature;
    NinjaModelProperty<double, KEY_MAXTEMP> MaxTemperature;
    NinjaModelProperty<double, KEY_HEATMINPERCENT> HeatMinPercent;
    NinjaModelProperty<double, KEY_HEATMAXPERCENT> HeatMaxPercent;
    NinjaModelProperty<double, KEY_HEATMANUALOUTPUT> HeatManualOutputPercent;

    NinjaModelProperty<double, KEY_FRIDGETEMP> FridgeTemp = -127.0; //also input for heatPID
    NinjaModelProperty<double, KEY_BEERTEMP> BeerTemp = -127.0; //also Input for mainPID
    NinjaModelProperty<opState, KEY_CONTROLLERSTATE> ControllerState = IDLE;
    NinjaModelProperty<ApplicationState, KEY_APPSTATE> AppState = UNDEFINED;

    NinjaModelProperty<bool, KEY_EXTERNALTEMPPROFILEACTIVE> ExternalProfileActive; // true if Fermentrack temperature profile is active

private:
    //TODO: include boost library or implement a ligthweigth version of boost:variant type
    //std::map<NinjaModelPropertyKey, NinjaModelPropertyVariant> _properties;
};

#endif
