#ifndef TemperatureProfile_h_
#define TemperatureProfile_h_

#include <list>
#include <time.h>

enum TemperatureProfileStepDuration {
  SECONDS,
  MINUTES,
  HOURS,
  DAYS
};

struct BaseTemperatureProfileStep
{
  double targetTemperature;
  double duration;
  TemperatureProfileStepDuration durationUnit;
  virtual double getCurrentTargetTemperature(long currentDuration) = 0;
};

template<typename ProfileType>
struct TemperatureProfileStep : BaseTemperatureProfileStep
{
  double getCurrentTargetTemperature(long currentDuration)
  {
    return ProfileType::getCurrentTargetTemperature(this, currentDuration);
  }
};

class ConstantTemperatureProfileStepType
{
public:
  static double getCurrentTargetTemperature(TemperatureProfileStep<ConstantTemperatureProfileStepType> *step, long duration)
  {
    return step->targetTemperature;
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
    _profileSteps.clear();
    _currentStep = _profileSteps.end();
  }

  void AddProfileStep(BaseTemperatureProfileStep *newStep)
  {
    _profileSteps.push_back(newStep);
  }

  bool ActivateTemperatureProfile()
  {
    if(Time.isValid() == false)
      return false;

    if(_profileSteps.size() == 0)
      return false;

    _currentStep = _profileSteps.begin();
    _currentStepStartTimestamp = Time.now();
    _isActive = true;
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

    double currentDuration = difftime(Time.now(), _currentStepStartTimestamp);
    double duration = (*_currentStep)->duration;
    switch((*_currentStep)->durationUnit)
    {
      case MINUTES:
        duration *= 60;
        break;
      case HOURS:
        duration *= 60*60;
        break;
      case DAYS:
        duration *= 24*60*60;
        break;
      default:
        break;
    }
    if(currentDuration > duration)
    {
      if(_currentStep != _profileSteps.end())
      {
        ++_currentStep;
        _currentStepStartTimestamp += duration;
        currentDuration -= duration;
        return GetCurrentTargetTemperature(targetTemperature);
      }
      else
      {
        targetTemperature = (*_currentStep)->targetTemperature;
        return false;
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
