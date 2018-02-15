#ifndef CoolerHeaterContoller_h
#define CoolerHeaterContoller_h

#include "application.h"

//TODO: refactor peakDetect to separate function that will be called from app when a peak is detected
//TODO: create separate classes for SSR actuators
//TODO: set heat PID I-term to 0 when goig to IDLE from HEAT
//TODO: notify the app somehow when controllerState changes
//TODO: save peakestimator to eeprom when state changes from IDLE, COOL -> IDLE, IDLE

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

struct AppConfig;

class CoolerHeaterContoller
{
  public:
    CoolerHeaterContoller(int, int);
    void Update(double currentTemp, double setTemp, double heatOutput, bool peakDetected);
    void Activate();
    void Disable();
    opState GetState();
    void Configure(const AppConfig&);
  private:
    int _cool_pin;
    int _heat_pin;

    double idleDiff = 0.5;     // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
    double peakDiff = 0.25;      // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
    unsigned int coolMinOff = 600;     // minimum compressor off time, seconds (5 min)
    unsigned int coolMinOn = 60;    // minimum compressor on time, seconds (1.5 min)
    unsigned int coolMaxOn = 2700;     // maximum compressor on time, seconds (45 min)
    unsigned int peakMaxTime = 1200;   // maximum runTime to consider for peak estimation, seconds (20 min)
    unsigned int peakMaxWait = 1800;   // maximum wait on peak, seconds (30 min)
    unsigned int heatMinOff = 600;     // minimum HEAT off time, seconds (5 min)
    unsigned long heatWindow = 60000;  // window size for HEAT time proportioning, ms (5 min)
    unsigned int minIdleTime = 120; // minimum idle time between cool -> heat or heat -> cool
    double no_heat_below = 10;
    double no_cool_above = 25;

    double peakEstimator;
    double peakEstimate;      // to determine prediction error = (estimate - actual)

    byte controllerState[2];
    byte controllerMode;

    unsigned long startTime;  // timing variables for enforcing min/max cycling times
    unsigned long stopTime;

    bool isActive;

    void updatecontrollerState(byte);
    void updatecontrollerState(byte, byte);
    void gotoIdle();
    double tuneEstimator(double currentEstimator, double error);

};

#endif
