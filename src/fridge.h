#ifndef FRIDGE_H
#define FRIDGE_H

#include "application.h"
#include "probe.h"
#include "PID_v1.h"
#define LED_BUILTIN D7
#define NO_COOL_LIMIT   25
#define NO_HEAT_LIMIT   10

enum opState {  // fridge operation states
  IDLE,
  COOL,
  HEAT,
};

const double fridgeIdleDiff = 0.5;     // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
const double fridgePeakDiff = 0.25;      // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
const unsigned int coolMinOff = 600;     // minimum compressor off time, seconds (5 min)
const unsigned int coolMinOn = 60;    // minimum compressor on time, seconds (1.5 min)
const unsigned int coolMaxOn = 2700;     // maximum compressor on time, seconds (45 min)
const unsigned int peakMaxTime = 1200;   // maximum runTime to consider for peak estimation, seconds (20 min)
const unsigned int peakMaxWait = 1800;   // maximum wait on peak, seconds (30 min)
const unsigned int heatMinOff = 600;     // minimum HEAT off time, seconds (5 min)
const unsigned long heatWindow = 60000;  // window size for HEAT time proportioning, ms (5 min)

extern byte fridgeState[2];      // [0] - current fridge state; [1] - fridge state t - 1 history
extern double peakEstimator;     // to predict COOL overshoot; units of deg F per hour (always positive)
extern double peakEstimate;      // to determine prediction error = (estimate - actual)
extern unsigned long startTime;  // timing variables for enforcing min/max cycling times
extern unsigned long stopTime;

extern probe* fridge;  // external variables declared in globals.h
extern probe* beer;
extern double Output , fridgeTemp, heatSetpoint, heatOutput, heatInput;
extern const byte relay1, relay2;
extern byte programState;
extern PID heatPID;

void updateFridge();  // core functions
void tuneEstimator(double* estimator, double error);
void updateFridgeState(byte state);
void updateFridgeState(byte state0, byte state1);
bool gotoIdle();
void publishFridgeState();

inline byte getFridgeState(byte index) { return fridgeState[index]; };  // inlines for accessing fridge variables
inline double getPeakEstimator() { return peakEstimator; };
inline double* getPeakEstimatorAddr() { return &peakEstimator; };
inline unsigned long getStartTime() { return startTime; };
inline unsigned long getStopTime() { return stopTime; };

#endif
