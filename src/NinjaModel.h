#ifndef NinjaModel_h
#define NinjaModel_h

#include "application.h"
#include "PID_v1.h"
#include "Property.h"
#include "enum.h"
#include "globals.h"
#include <map>
#include "variant.h"
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
  KEY_PIDINTEGRCLAMPERR,
  KEY_PIDMODE,
  KEY_HEATOUTPUT,
  KEY_HEATPIDKP,
  KEY_HEATPIDKI,
  KEY_HEATPIDKD,
  KEY_HEATPIDINTEGRCLAMPERR,
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

template<typename T>
class NinjaModelProperty : public Property<T>
{
public:

  NinjaModelProperty(NinjaModelPropertyKey key) : Property<T>(), _key(key)
  {
  };

  NinjaModelProperty(NinjaModelPropertyKey key, const T value) : Property<T>(value), _key(key)
  {
  };

  NinjaModelProperty(NinjaModelPropertyKey key, const Property<T>& value) : Property<T>(value.Get()), _key(key)
  {
  };

  NinjaModelProperty(const NinjaModelProperty& value) : Property<T>(value.Get()), _key(value.getKey())
  {
  };

  NinjaModelPropertyKey getKey() const
  {
    return _key;
  };


  NinjaModelProperty<T>& operator=(const T& other)
  {
    Property<T>::operator= (other);
    return *this;
  };

  NinjaModelProperty<T>& operator=(const Property<T>& other)
  {
    Property<T>::operator= (other);
    return *this;
  };

private:
  NinjaModelPropertyKey _key;
};

class NinjaModel
{
  //  typedef nonstd::variant< NinjaModelProperty<double>*,
  //                        NinjaModelProperty<int>*,
  //                        NinjaModelProperty<bool>*,
  //                        NinjaModelProperty<opMode>*,
  //                        NinjaModelProperty<opState>*,
  //                        NinjaModelProperty<ApplicationState>*> NinjaModelPropertyVariant;
public:
    NinjaModel();
    NinjaModelProperty<double> SetPoint; // target beer temp
    NinjaModelProperty<double> Output; // main PID output temperature, also setpoint for heatPID
    NinjaModelProperty<double> PeakEstimator;
    NinjaModelProperty<double> PID_Kp;
    NinjaModelProperty<double> PID_Ki;
    NinjaModelProperty<double> PID_Kd;
    NinjaModelProperty<double> PID_IntegratorClampingError;
    NinjaModelProperty<int> PIDMode;
    NinjaModelProperty<double> HeatOutput;
    NinjaModelProperty<double> HeatPID_Kp;
    NinjaModelProperty<double> HeatPID_Ki;
    NinjaModelProperty<double> HeatPID_Kd;
    NinjaModelProperty<double> HeatPID_IntegratorClampingError;
    NinjaModelProperty<int> HeatPIDMode;
    NinjaModelProperty<bool> StandBy;

    NinjaModelProperty<double> IdleDiff;          // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
    NinjaModelProperty<double> PeakDiff;          // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
    NinjaModelProperty<int> CoolMinOff;  // minimum compressor off time, seconds (5 min)
    NinjaModelProperty<int> CoolMinOn;   // minimum compressor on time, seconds (1.5 min)
    NinjaModelProperty<int> CoolMaxOn;   // maximum compressor on time, seconds (45 min)
    NinjaModelProperty<int> PeakMaxTime; // maximum runTime to consider for peak estimation, seconds (20 min)
    NinjaModelProperty<int> PeakMaxWait; // maximum wait on peak, seconds (30 min)
    NinjaModelProperty<int> HeatMinOff;  // minimum HEAT off time, seconds (5 min)
    NinjaModelProperty<int> HeatWindow; // window size for HEAT time proportioning, ms (5 min)
    NinjaModelProperty<int> MinIdleTime;// minimum idle time between cool -> heat or heat -> cool
    NinjaModelProperty<double> NoHeatBelow;     // dont turn on heating when tempretare is bellow this
    NinjaModelProperty<double> NoCoolAbove;     // dont turn on cooling when tempretare is abowe this
    NinjaModelProperty<opMode> ControllerMode;
    NinjaModelProperty<double> MinTemperature;
    NinjaModelProperty<double> MaxTemperature;
    NinjaModelProperty<double> HeatMinPercent;
    NinjaModelProperty<double> HeatMaxPercent;
    NinjaModelProperty<double> HeatManualOutputPercent;

    NinjaModelProperty<double> FridgeTemp;// = -127.0; //also input for heatPID
    NinjaModelProperty<double> BeerTemp;// = -127.0; //also Input for mainPID
    NinjaModelProperty<opState> ControllerState;// = IDLE;
    NinjaModelProperty<ApplicationState> AppState;// = UNDEFINED;

    NinjaModelProperty<bool> ExternalProfileActive; // true if Fermentrack temperature profile is active

private:
    //TODO: include boost library or implement a ligthweigth version of boost:variant type
    //std::map<NinjaModelPropertyKey, NinjaModelPropertyVariant> _properties;
};

#endif
