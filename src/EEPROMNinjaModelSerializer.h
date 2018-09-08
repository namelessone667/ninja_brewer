#ifndef EEPROMNinjaModelSerializer_h
#define EEPROMNinjaModelSerializer_h

#include "INinjaModelSerializer.h"
#include "EEPROM_MAP.h"
#include "DS18B20Sensor.h"
#ifdef TEMP_PROFILES
#include "TemperatureProfile.h"
#endif

//TODO: refactor
// define a starting eeprom address for profile
// create a map of property keys - eeprom address (offset)

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

    address = EEPROMGetInternal(address, model.MinTemperature);
    address = EEPROMGetInternal(address, model.MaxTemperature);
    address = EEPROMGetInternal(address, model.HeatMinPercent);
    address = EEPROMGetInternal(address, model.HeatMaxPercent);
    address = EEPROMGetInternal(address, model.HeatManualOutputPercent);

    address = EEPROMGetInternal(address, model.ExternalProfileActive);

    if(model.HeatWindow == 0)
    {
      theApp::getInstance().getLogger().error("EEPROM data corrupted!");
      return false;
    }

    return true;
  }

  inline bool Save(const NinjaModel& model)
  {
    if(model.HeatWindow == 0)
    {
      theApp::getInstance().getLogger().error("Model data invalid, save to EEPROM not executed!");
      return false;
    }

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

    address = EEPROMPutInternal(address, model.MinTemperature.Get());
    address = EEPROMPutInternal(address, model.MaxTemperature.Get());
    address = EEPROMPutInternal(address, model.HeatMinPercent.Get());
    address = EEPROMPutInternal(address, model.HeatMaxPercent.Get());
    address = EEPROMPutInternal(address, model.HeatManualOutputPercent.Get());

    address = EEPROMPutInternal(address, model.ExternalProfileActive.Get());

    return true;
  }

  static bool SaveSensorAddress(const OneWireAddress& sensorAddress, int eeprom_address)
  {
    for(int i = 0; i < 8; i++)
    {
      theApp::getInstance().getLogger().info("SaveSensorAddress to EEPROM, address:" + String(eeprom_address) + ", " + String(sizeof(sensorAddress.address[i])) + " bytes written, value: " + String(sensorAddress.address[i]));
      EEPROM.put(eeprom_address, sensorAddress.address[i]);
      eeprom_address += sizeof(sensorAddress.address[i]);
    }
    return true;
  }

  static OneWireAddress LoadSensorAddress(int eeprom_address)
  {
    OneWireAddress sensorAddress;
    for(int i = 0; i < 8; i++)
    {
      EEPROM.get(eeprom_address, sensorAddress.address[i]);
      theApp::getInstance().getLogger().info("LoadSensorAddress from EEPROM, address:" + String(eeprom_address) + ", " + String(sizeof(sensorAddress.address[i])) + " bytes loaded, value: " + String(sensorAddress.address[i]));
      eeprom_address += sizeof(sensorAddress.address[i]);
    }
    return sensorAddress;
  }
#ifdef TEMP_PROFILES
  bool LoadTempProfile(TemperatureProfile& tempProfile)
  {
    theApp::getInstance().getLogger().info("Loading temperature profile from eeprom");
    byte eeprom_ver = -1;
    EEPROM.get(APP_VERSION_ADDR, eeprom_ver);

    if(eeprom_ver != EEPROM_MAP_VER)
    {
      theApp::getInstance().getLogger().info("Failed to load temperature profile, eeprom version mismatch");
      return false;
    }

    int address = TEMP_PROFILE_ADDR;
    int stepsCount;

    TemperatureProfileStepType stepType;
    double targetTemp;
    long duration;
    TemperatureProfileStepDuration durationUnit;
    int temp1, temp2;

    address = EEPROMGetInternal(address, stepsCount);

    theApp::getInstance().getLogger().info("Number of temperature profile steps: " + String(stepsCount));

    tempProfile.ClearProfile();
    address = TEMP_PROFILE_ADDR + 100;

    for(int i = 0; i < stepsCount; i++)
    {
      address = EEPROMGetInternal(address, targetTemp);
      address = EEPROMGetInternal(address, duration);
      address = EEPROMGetInternal(address, temp1);
      address = EEPROMGetInternal(address, temp2);

      durationUnit = (TemperatureProfileStepDuration)temp1;
      stepType = (TemperatureProfileStepType)temp2;

      switch(stepType)
      {
        case CONSTANT:
          tempProfile.AddProfileStep<ConstantTemperatureProfileStepType>(targetTemp, duration, durationUnit);
          break;
        case LINEAR:
          tempProfile.AddProfileStep<LinearTemperatureProfileStepType>(targetTemp, duration, durationUnit);
          break;
        default:
          theApp::getInstance().getLogger().info("Invalid temperature profile step type");
          break;
      }
    }

    return true;
  }

  bool SaveTempProfile(const TemperatureProfile& tempProfile)
  {
    theApp::getInstance().getLogger().info("Saving temperature profile to eeprom");
    byte eeprom_ver = -1;
    EEPROM.get(APP_VERSION_ADDR, eeprom_ver);

    if(eeprom_ver != EEPROM_MAP_VER)
    {
      theApp::getInstance().getLogger().info("Failed to save temperature profile, eeprom version mismatch");
      return false;
    }

    int address = TEMP_PROFILE_ADDR;

    int stepsCount = tempProfile.GetProfileSteps().size();
    //bool isActive = tempProfile.IsActiveTemperatureProfile();
    //int activeStepIndex = tempProfile.GetCurrentStepIndex();
    //long activeStepStartTimestamp = tempProfile.GetCurrentStepStartTimestamp();

    address = EEPROMPutInternal(address, stepsCount);
    //address = EEPROMPutInternal(address, isActive);
    //address = EEPROMPutInternal(address, activeStepIndex);
    //address = EEPROMPutInternal(address, activeStepStartTimestamp);

    address = TEMP_PROFILE_ADDR + 100;

    for(auto it = tempProfile.GetProfileSteps().cbegin(); it != tempProfile.GetProfileSteps().cend(); it++)
    {
      //address = EEPROMPutInternal(address, (*it)->GetStartTemperature());
      address = EEPROMPutInternal(address, (*it)->GetTargetTemperature());
      address = EEPROMPutInternal(address, (*it)->GetDuration());
      address = EEPROMPutInternal(address, (int)(*it)->GetDurationUnit());
      address = EEPROMPutInternal(address, (int)(*it)->GetTemperatureProfileStepType());

      theApp::getInstance().getLogger().info("Saved step " + String((*it)->GetTargetTemperature()));
    }

    return true;
  }

  void SaveTempProfileRuntimeParameters(bool isActive, int activeStepIndex, long activeStepDuration)
  {
    int address = TEMP_PROFILE_RUNTIME_ADDR;
    address = EEPROMPutInternal(address, isActive);
    address = EEPROMPutInternal(address, activeStepIndex);
    address = EEPROMPutInternal(address, activeStepDuration);
  }

  bool LoadTempProfileRuntimeParameters(bool& isActive, int& activeStepIndex, long& activeStepDuration)
  {
    byte eeprom_ver = -1;
    EEPROM.get(APP_VERSION_ADDR, eeprom_ver);

    if(eeprom_ver != EEPROM_MAP_VER)
    {
      theApp::getInstance().getLogger().info("Failed to load temperature profile runtime parameters, eeprom version mismatch");
      return false;
    }

    int address = TEMP_PROFILE_RUNTIME_ADDR;
    address = EEPROMGetInternal(address, isActive);

    if(isActive)
    {
      address = EEPROMGetInternal(address, activeStepIndex);
      address = EEPROMGetInternal(address, activeStepDuration);
    }
    return true;
  }

  void ClearTempProfileRuntimeParameters()
  {
    int address = TEMP_PROFILE_RUNTIME_ADDR;
    address = EEPROMPutInternal(address, false);
    address = EEPROMPutInternal(address, (int)0);
    address = EEPROMPutInternal(address, (long)0);
  }
#endif
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

  template <typename T> int EEPROMGetInternal( int idx, T &value )
  {
    EEPROM.get(idx, value);
    theApp::getInstance().getLogger().info("EEPROMGetInternal address:" + String(idx) + ", " + String(sizeof(T)) + " bytes loaded, value: " + String(value));
    return idx + sizeof(T);
  }
};

#endif
