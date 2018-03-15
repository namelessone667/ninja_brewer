#ifndef enum_h
#define enum_h

enum ApplicationState
{
  INIT,
  RUNNING,
  STAND_BY,
  IN_ERROR,
  UNDEFINED = -1
};

enum opMode {
  COOLER_HEATER = 0,
  COOLER_ONLY = 1,
  HEATER_ONLY = 2
};

enum opState {  // fridge operation states
  IDLE,
  COOL,
  HEAT,
};

#endif
