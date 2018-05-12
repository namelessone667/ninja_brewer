#ifndef EEPROMNinjaModelSerializer_h
#define EEPROMNinjaModelSerializer_h

#include "INinjaModelSerializer.h"
#include "EEPROM_MAP.h"

class EEPROMNinjaModelSerializer : INinjaModelSerializer
{
public:
  inline bool Load(NinjaModel& model)
  {
    byte eeprom_ver = -1;
    EEPROM.get(APP_VERSION_ADDR, eeprom_ver);

    theApp::getInstance().getLogger().info("EEPROM version: " + String(eeprom_ver));

    if(eeprom_ver != EEPROM_MAP_VER)
      return false;

    int address = APP_CONFIG_ADDR;

    address = EEPROMGetInternal(address, model.SetPoint); // target beer temp
    address = EEPROMGetInternal(address, model.Output); // main PID output temperature, also setpoint for heatPID
    address = EEPROMGetInternal(address, model.PeakEstimator);
    address = EEPROMGetInternal(address, model.PID_Kp);
    address = EEPROMGetInternal(address, model.PID_Ki);
    address = EEPROMGetInternal(address, model.PID_Kd);
    address = EEPROMGetInternal(address, model.PIDMode);
    address = EEPROMGetInternal(address, model.HeatOutput);
    address = EEPROMGetInternal(address, model.HeatPID_Kp);
    address = EEPROMGetInternal(address, model.HeatPID_Ki);
    address = EEPROMGetInternal(address, model.HeatPID_Kd);
    address = EEPROMGetInternal(address, model.HeatPIDMode);
    address = EEPROMGetInternal(address, model.StandBy);

    address = EEPROMGetInternal(address, model.IdleDiff);          // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
    address = EEPROMGetInternal(address, model.PeakDiff);          // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
    address = EEPROMGetInternal(address, model.CoolMinOff);  // minimum compressor off time, seconds (5 min)
    address = EEPROMGetInternal(address, model.CoolMinOn);   // minimum compressor on time, seconds (1.5 min)
    address = EEPROMGetInternal(address, model.CoolMaxOn);   // maximum compressor on time, seconds (45 min)
    address = EEPROMGetInternal(address, model.PeakMaxTime); // maximum runTime to consider for peak estimation, seconds (20 min)
    address = EEPROMGetInternal(address, model.PeakMaxWait); // maximum wait on peak, seconds (30 min)
    address = EEPROMGetInternal(address, model.HeatMinOff);  // minimum HEAT off time, seconds (5 min)
    address = EEPROMGetInternal(address, model.HeatWindow); // window size for HEAT time proportioning, ms (5 min)
    address = EEPROMGetInternal(address, model.MinIdleTime);// minimum idle time between cool -> heat or heat -> cool
    address = EEPROMGetInternal(address, model.NoHeatBelow);     // dont turn on heating when tempretare is bellow this
    address = EEPROMGetInternal(address, model.NoCoolAbove);     // dont turn on cooling when tempretare is abowe this
    address = EEPROMGetInternal(address, model.ControllerMode);

    if(model.HeatWindow == 0)
    {
      theApp::getInstance().getLogger().error("EEPROM data corrupted!"); 
      return false;
    }

    return true;
  }

  inline bool Save(const NinjaModel& model)
  {
    byte eeprom_ver = EEPROM_MAP_VER;
    EEPROM.put(APP_VERSION_ADDR, eeprom_ver);

    int address = APP_CONFIG_ADDR;

    address = EEPROMPutInternal(address, model.SetPoint.Get()); // target beer temp
    address = EEPROMPutInternal(address, model.Output.Get()); // main PID output temperature, also setpoint for heatPID
    address = EEPROMPutInternal(address, model.PeakEstimator.Get());
    address = EEPROMPutInternal(address, model.PID_Kp.Get());
    address = EEPROMPutInternal(address, model.PID_Ki.Get());
    address = EEPROMPutInternal(address, model.PID_Kd.Get());
    address = EEPROMPutInternal(address, model.PIDMode.Get());
    address = EEPROMPutInternal(address, model.HeatOutput.Get());
    address = EEPROMPutInternal(address, model.HeatPID_Kp.Get());
    address = EEPROMPutInternal(address, model.HeatPID_Ki.Get());
    address = EEPROMPutInternal(address, model.HeatPID_Kd.Get());
    address = EEPROMPutInternal(address, model.HeatPIDMode.Get());
    address = EEPROMPutInternal(address, model.StandBy.Get());

    address = EEPROMPutInternal(address, model.IdleDiff.Get());          // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
    address = EEPROMPutInternal(address, model.PeakDiff.Get());          // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
    address = EEPROMPutInternal(address, model.CoolMinOff.Get());  // minimum compressor off time, seconds (5 min)
    address = EEPROMPutInternal(address, model.CoolMinOn.Get());   // minimum compressor on time, seconds (1.5 min)
    address = EEPROMPutInternal(address, model.CoolMaxOn.Get());   // maximum compressor on time, seconds (45 min)
    address = EEPROMPutInternal(address, model.PeakMaxTime.Get()); // maximum runTime to consider for peak estimation, seconds (20 min)
    address = EEPROMPutInternal(address, model.PeakMaxWait.Get()); // maximum wait on peak, seconds (30 min)
    address = EEPROMPutInternal(address, model.HeatMinOff.Get());  // minimum HEAT off time, seconds (5 min)
    address = EEPROMPutInternal(address, model.HeatWindow.Get()); // window size for HEAT time proportioning, ms (5 min)
    address = EEPROMPutInternal(address, model.MinIdleTime.Get());// minimum idle time between cool -> heat or heat -> cool
    address = EEPROMPutInternal(address, model.NoHeatBelow.Get());     // dont turn on heating when tempretare is bellow this
    address = EEPROMPutInternal(address, model.NoCoolAbove.Get());     // dont turn on cooling when tempretare is abowe this
    address = EEPROMPutInternal(address, model.ControllerMode.Get());

    return true;
  }

protected:
  template <typename T> int EEPROMPutInternal( int idx, const T &t )
  {
    theApp::getInstance().getLogger().info("EEPROMPutInternal address:" + String(idx) + ", " + String(sizeof(T)) + " bytes written, value: " + String(t));
    EEPROM.put(idx, t);
    return idx + sizeof(T);
  }

  template <typename T> int EEPROMGetInternal( int idx, Property<T> &t )
  {
    T value;
    EEPROM.get(idx, value);
    t = value;
    theApp::getInstance().getLogger().info("EEPROMGetInternal address:" + String(idx) + ", " + String(sizeof(T)) + " bytes loaded, value: " + String(value));
    return idx + sizeof(T);
  }
};

#endif
