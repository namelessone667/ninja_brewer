#ifndef INinjaModelSerializer_h
#define INinjaModelSerializer_h

#include "NinjaModel.h"

class INinjaModelSerializer
{
public:
  virtual bool Load(NinjaModel& model) = 0;
  virtual bool Save(const NinjaModel& model) = 0;
  virtual bool LoadTempProfile(TemperatureProfile& tempProfile) = 0;
  virtual bool SaveTempProfile(const TemperatureProfile& tempProfile) = 0;
};

#endif
