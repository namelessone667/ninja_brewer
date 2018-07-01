#ifndef TemperatureProfile_h_
#define TemperatureProfile_h_

#include <list>
#include <time.h>

///////////////////////////////////////////////////////////////////////////////
/* duration: constant (seconds, minutes, hours, days), manual */
/* temperature: constant, linear_rise, rise_per_time_unit */
//////////////////////////////////////////////////////////////////////////////

enum TemperatureProfileStepDuration {
  SECONDS = 1,
  MINUTES = 2,
  HOURS = 3,
  DAYS = 4
};

class TemperatureProfile;

class BaseTemperatureProfileStep
{
protected:
  double targetTemperature;
  double startTemperature;
  long duration;
  TemperatureProfileStepDuration durationUnit;
public:
  BaseTemperatureProfileStep(double temp, long dur, TemperatureProfileStepDuration unit) :
  targetTemperature(temp), duration(dur), durationUnit(unit)
  {

  }
  virtual ~BaseTemperatureProfileStep() = default;
  double GetTargetTemperature()
  {
    return targetTemperature;
  }
  double GetDuration()
  {
    return duration;
  }
  TemperatureProfileStepDuration GetDurationUnit()
  {
    return durationUnit;
  }
  long GetDurationInSeconds()
  {
    switch(durationUnit)
    {
      case MINUTES:
        return duration * 60;
      case HOURS:
        return duration * 60*60;
      case DAYS:
        return duration * 24*60*60;
      default:
        return duration;
    }
  }
  void SetStartTemperature(double temp)
  {
    startTemperature = temp;
  }
  double GetStartTemperature()
  {
    return startTemperature;
  }

  virtual double getCurrentTargetTemperature(long currentDuration) = 0;
};

template<typename ProfileType>
class TemperatureProfileStep : public BaseTemperatureProfileStep
{
  TemperatureProfileStep(double temp, long dur, TemperatureProfileStepDuration unit) :
  BaseTemperatureProfileStep(temp, dur, unit)
  {

  }

  double getCurrentTargetTemperature(long currentDuration)
  {
    return ProfileType::getCurrentTargetTemperature(this, currentDuration);
  }

  friend TemperatureProfile;
};

class ConstantTemperatureProfileStepType
{
public:
  static double getCurrentTargetTemperature(TemperatureProfileStep<ConstantTemperatureProfileStepType> *step, long duration)
  {
    //Log.info(String::format("ConstantTemperatureProfileStepType duration: %d s,  %.2f C", duration, step->GetTargetTemperature()));
    return step->GetTargetTemperature();
  }
};

class LinearTemperatureProfileStepType
{
public:
  static double getCurrentTargetTemperature(TemperatureProfileStep<LinearTemperatureProfileStepType> *step, long duration)
  {
    long stepDuration = step->GetDurationInSeconds();
    if(duration >= stepDuration)
      return step->GetTargetTemperature();

    double temp = step->GetStartTemperature() + (step->GetTargetTemperature()-step->GetStartTemperature())*((double)duration/(double)stepDuration);
    //Log.info(String::format("LinearTemperatureProfileStepType duration: %d s,  %.2f C, step duration: %d s, start temp: %.2f C", duration, temp, stepDuration, step->GetStartTemperature()));
    return temp;
  }
};

class TemperatureProfile
{
public:
  TemperatureProfile()
  {
    _currentStep = _profileSteps.end();
  }

  void ClearProfile()
  {
    DeactivateTemperatureProfile();
    while(!_profileSteps.empty())
    {
      delete(_profileSteps.front());
      _profileSteps.pop_front();

    }
    _currentStep = _profileSteps.end();
  }

  template<typename ProfileType>
  void AddProfileStep(double temp, long dur, TemperatureProfileStepDuration unit)
  {
    _profileSteps.push_back(new TemperatureProfileStep<ProfileType>(temp, dur, unit));
  }

  bool ActivateTemperatureProfile()
  {
    if(Time.isValid() == false)
      return false;

    if(_profileSteps.size() == 0)
      return false;

    _currentStep = _profileSteps.begin();
    (*_currentStep)->SetStartTemperature((*_currentStep)->GetTargetTemperature());
    _currentStepStartTimestamp = Time.now();
    _isActive = true;
    return true;
  }

  void DeactivateTemperatureProfile()
  {
    _isActive = false;
  }

  bool IsActiveTemperatureProfile()
  {
    return _isActive;
  }

  bool GetCurrentTargetTemperature(double &targetTemperature)
  {
    if(_isActive == false)
      return false;

    long currentDuration = difftime(Time.now(), _currentStepStartTimestamp);
    double duration = (*_currentStep)->GetDurationInSeconds();
    if(currentDuration > duration)
    {
      if(std::next(_currentStep,1) != _profileSteps.end())
      {
        double startTemp = (*_currentStep)->GetTargetTemperature();
        ++_currentStep;
        _currentStepStartTimestamp += duration;
        currentDuration -= duration;
        (*_currentStep)->SetStartTemperature(startTemp);
        return GetCurrentTargetTemperature(targetTemperature);
      }
      else
      {
        targetTemperature = (*_currentStep)->getCurrentTargetTemperature(currentDuration);
        return true;
      }
    }
    else
    {
      targetTemperature = (*_currentStep)->getCurrentTargetTemperature(currentDuration);
      return true;
    }
  }
private:
  std::list<BaseTemperatureProfileStep*> _profileSteps;
  std::list<BaseTemperatureProfileStep*>::iterator _currentStep;
  time_t _currentStepStartTimestamp;
  bool _isActive = false;
};

#endif
