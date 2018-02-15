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

    double idleDiff;     // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
    double peakDiff;      // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
    int coolMinOff;     // minimum compressor off time, seconds (5 min)
    int coolMinOn;    // minimum compressor on time, seconds (1.5 min)
    int coolMaxOn;     // maximum compressor on time, seconds (45 min)
    int peakMaxTime;   // maximum runTime to consider for peak estimation, seconds (20 min)
    int peakMaxWait;   // maximum wait on peak, seconds (30 min)
    int heatMinOff;     // minimum HEAT off time, seconds (5 min)
    int heatWindow;  // window size for HEAT time proportioning, seconds (5 min)
    int minIdleTime; // minimum idle time between cool -> heat or heat -> cool
    double no_heat_below;
    double no_cool_above;

    double peakEstimator;
    double peakEstimate;      // to determine prediction error = (estimate - actual)

    byte controllerState[2];
    byte controllerMode;

    unsigned long startTime;  // timing variables for enforcing min/max cycling times
    unsigned long stopTime;

    bool isActive;
    bool isConfigured;

    void updatecontrollerState(byte);
    void updatecontrollerState(byte, byte);
    void gotoIdle();
    double tuneEstimator(double currentEstimator, double error);

};

#endif
