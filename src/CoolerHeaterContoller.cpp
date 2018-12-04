#include "CoolerHeaterContoller.h"

CoolerHeaterContoller::CoolerHeaterContoller(int cooling_ssr_pin, int heating_ssr_pin)
{
  _cool_pin = cooling_ssr_pin;
  _heat_pin = heating_ssr_pin;
  controllerState[0] = IDLE;
  controllerState[1] = IDLE;
  controllerMode = COOLER_HEATER;
  startTime = 0;
  stopTime = 0;

  peakEstimator = 30;    // to predict COOL overshoot; units of deg C per hour (always positive)
  peakEstimate = 0;

  isActive = false;
  isConfigured = false;

  pinMode(_cool_pin, OUTPUT);  // configure relay pins and write default HIGH (relay open)
  pinMode(_heat_pin, OUTPUT);

  digitalWrite(_cool_pin, LOW);
  digitalWrite(_heat_pin, LOW);

}

void CoolerHeaterContoller::Configure(const NinjaModel& config)
{
  controllerMode = config.ControllerMode;
  idleDiff = config.IdleDiff;     // constrain fridge temperature to +/- 0.5 deg C (0.9 deg F) differential
  peakDiff = config.PeakDiff;      // constrain allowed peak error to +/- 0.25 deg C (0.45 deg F) differential
  coolMinOff = config.CoolMinOff;     // minimum compressor off time, seconds (5 min)
  coolMinOn = config.CoolMinOn;    // minimum compressor on time, seconds (1.5 min)
  coolMaxOn = config.CoolMaxOn;     // maximum compressor on time, seconds (45 min)
  peakMaxTime = config.PeakMaxTime;   // maximum runTime to consider for peak estimation, seconds (20 min)
  peakMaxWait = config.PeakMaxWait;   // maximum wait on peak, seconds (30 min)
  heatMinOff = config.HeatMinOff;     // minimum HEAT off time, seconds (5 min)
  heatWindow = config.HeatWindow;  // window size for HEAT time proportioning, ms (5 min)
  minIdleTime = config.MinIdleTime; // minimum idle time between cool -> heat or heat -> cool
  no_heat_below = config.NoHeatBelow;
  no_cool_above = config.NoCoolAbove;
  peakEstimator = config.PeakEstimator;

  isConfigured = true;
}

void CoolerHeaterContoller::Update(double currentTemp, double setTemp, double heatOutput, bool peakDetected)
{
  if(!isConfigured)
    return;

  if(!isActive)
  {
    gotoIdle();
    return;
  }

  switch (controllerState[0]) {  // MAIN switch -- IDLE/peak detection, COOL, HEAT routines
    default:
    case IDLE:
      //if we are in IDLE state less than minIdleTime second, stay IDLE
      if(controllerMode != HEATER_ONLY && ((long)((millis() - stopTime) / 1000) < minIdleTime))
        break;

      if (controllerState[1] == IDLE) {   // only switch to HEAT/COOL if not waiting for COOL peak
        if ((currentTemp > setTemp + idleDiff) && (((long)((millis() - stopTime) / 1000) > coolMinOff) || stopTime == 0) &&
            (currentTemp < no_cool_above) && controllerMode != HEATER_ONLY) {  // switch to COOL only if temp exceeds IDLE range and min off time met
          updatecontrollerState(COOL);    // update current fridge status and t - 1 history
          digitalWrite(_cool_pin, /*LOW*/HIGH);  // close relay 1; supply power to fridge compressor
          //digitalWrite(LED_BUILTIN, HIGH);
          startTime = millis();       // record COOLing start time
        }
        else if (controllerMode == HEATER_ONLY ||
                ((currentTemp < setTemp - idleDiff) && (((long)((millis() - stopTime) / 1000) > heatMinOff) || stopTime == 0) &&
                  (currentTemp > no_heat_below) && controllerMode != COOLER_ONLY) ) {  // switch to HEAT only if temp below IDLE range and min off time met
          updatecontrollerState(HEAT);
          //if (programState & 0b010000) heatSetpoint = setTemp;  // update heat PID setpoint if in automatic mode
          //heatSetpoint = setTemp;
          //heatPID.Compute();      // compute new heat PID output, update timings to align PID and time proportioning routine

          startTime = millis();   // start new time proportioned window
        }
      }
      else if (controllerState[1] == COOL) {  // do peak detect if waiting on COOL
        if (peakDetected) {        // negative peak detected...
#ifdef BREWPI_LINK
          double oldestimator = peakEstimator;
#endif
          peakEstimator = tuneEstimator(peakEstimator, peakEstimate - currentTemp);  // (error = estimate - actual) positive error requires larger estimator; negative:smaller
#ifdef BREWPI_LINK
          PiLink::getInstance().debugMessage("Tunning peak estimator, peak estimate: %fC, current temp: %fC, old estimator: %f, new estimator: %f", peakEstimate, currentTemp, oldestimator, peakEstimator);
#endif
          controllerState[1] = IDLE;          // stop peak detection until next COOL cycle completes
        }
        else {                                                               // no peak detected
          double offTime = (long)(millis() - stopTime) / 1000;      // IDLE time in seconds
          if (offTime < peakMaxWait) break;    // keep waiting for filter confirmed peak if too soon
#ifdef BREWPI_LINK
          double oldestimator = peakEstimator;
#endif
          peakEstimator = tuneEstimator(peakEstimator, peakEstimate - currentTemp);  // temp is drifting in the right direction, but too slowly; update estimator
#ifdef BREWPI_LINK
          PiLink::getInstance().debugMessage("Peak max wait reached, tunning peak estimator, peak estimate: %fC, current temp: %fC, old estimator: %f, new estimator: %f", peakEstimate, currentTemp, oldestimator, peakEstimator);
#endif
          controllerState[1] = IDLE;                                             // stop peak detection
        }
      }
      break;

    case COOL:  // run compressor until peak predictor lands on controller setTemp
      { double runTime = (long)(millis() - startTime) / 1000;  // runtime in seconds
      if (runTime < coolMinOn) break;     // ensure minimum compressor runtime
      if (currentTemp < setTemp - idleDiff) {  // temp already below output - idle differential: most likely cause is change in setpoint or long minimum runtime
        updatecontrollerState(IDLE, IDLE);    // go IDLE, ignore peaks
        digitalWrite(_cool_pin, /*HIGH*/LOW);       // open relay 1; power down fridge compressor
        //digitalWrite(LED_BUILTIN, LOW);
#ifdef BREWPI_LINK
      PiLink::getInstance().debugMessage("Switching fridge off, temp already below output - idle diff. Current temp: %fC, Estimated temp: %fC", currentTemp, currentTemp -(min(runTime, peakMaxTime) / 3600) * peakEstimator);
#endif
        stopTime = millis();              // record idle start
        break;
      }
      if ((currentTemp - (min(runTime, peakMaxTime) / 3600) * peakEstimator) < setTemp - idleDiff) {  // if estimated peak exceeds setTemp - differential, set IDLE and wait for actual peak
        peakEstimate = currentTemp - (min(runTime, peakMaxTime) / 3600) * peakEstimator;   // record estimated peak prediction
        updatecontrollerState(IDLE);     // go IDLE, wait for peak
        digitalWrite(_cool_pin, /*HIGH*/LOW);
        //digitalWrite(LED_BUILTIN, LOW);
        stopTime = millis();
      }
      if (runTime > coolMaxOn) {  // if compressor runTime exceeds max on time, skip peak detect, go IDLE
        updatecontrollerState(IDLE, IDLE);
        digitalWrite(_cool_pin, /*HIGH*/LOW);
        //digitalWrite(LED_BUILTIN, LOW);
        stopTime = millis();
      }
      break; }

    case HEAT:  // run HEAT using time proportioning
      {
        //heatSetpoint = setTemp;
        //heatInput = fridgeTemp;
        //heatPID.Compute();

        double runTime = millis() - startTime;  // runtime in ms
        while(runTime > heatWindow*1000)
        {
            startTime += heatWindow*1000;
            runTime -= heatWindow*1000;
        }
        //double heatOutput2 = heatOutput < 2 ? 0 : heatOutput;
        double heatOutputTime = heatWindow*heatOutput*10;
        //Serial.println("---HEAT START---");
        //Serial.print("runtime:");
        //Serial.println(runTime);
        //Serial.print("heatOutput:");
        //Serial.println(heatOutput);
        //Serial.print("SampleTime:");
        //Serial.println(heatPID.GetSampleTime());

      if ((runTime < heatOutputTime) && !digitalRead(_heat_pin))
      {
        digitalWrite(_heat_pin, /*LOW*/HIGH);           // active duty; close relay, write only once
        //Serial.println("turning relay ON");
      }
      else if ((runTime >= heatOutputTime) && digitalRead(_heat_pin))
      {
        digitalWrite(_heat_pin, /*HIGH*/LOW);  // active duty completed; rest of window idle; write only once
        //Serial.println("turning relay OFF");
      }
      //if (programState & 0b010000) heatSetpoint = setTemp;
      /*heatSetpoint = setTemp;
      if (heatPID.Compute()) {  // if heatPID computes (once per window), current window complete, start new
        //Serial.println("new PID values computed");
        startTime = millis();
      }*/
      if (controllerMode != HEATER_ONLY && (currentTemp > setTemp + idleDiff)) {  // temp exceeds setpoint, go to idle to decide if it is time to COOL
        updatecontrollerState(IDLE, IDLE);
        digitalWrite(_heat_pin, /*HIGH*/LOW);
        stopTime = millis();
      }
      break; }
  }
}

double CoolerHeaterContoller::tuneEstimator(double currentEstimator, double error) {  // tune fridge overshoot estimator
  //if (abs(error) <= peakDiff) return;            // leave estimator unchanged if error falls within contstrained peak differential
  if ((-1*peakDiff) <= error && error <= 0) return currentEstimator;
  if (error > 0) return currentEstimator * constrain(1.2 + 0.03 * abs(error), 1.2, 1.5);                 // if positive error; increase estimator 20% - 50% relative to error
    else return max(0.05, currentEstimator / constrain(1.2 + 0.03 * abs(error), 1.2, 1.5));  // if negative error; decrease estimator 17% - 33% relative to error, constrain to non-zero value

}

void CoolerHeaterContoller::updatecontrollerState(byte state)
{
  controllerState[1] = controllerState[0];
  controllerState[0] = state;
}

void CoolerHeaterContoller::updatecontrollerState(byte state0, byte state1)
{  // update current fridge state and history
  controllerState[1] = state1;
  controllerState[0] = state0;
}

void CoolerHeaterContoller::Activate()
{
  isActive = true;
}

void CoolerHeaterContoller::Disable()
{
  isActive = false;
}

void CoolerHeaterContoller::gotoIdle()
{
    switch (controllerState[0])
    {
        case IDLE:
          break;
        case HEAT:
            digitalWrite(_cool_pin, /*HIGH*/LOW);       // open relay 1; power down fridge compressor
            digitalWrite(_heat_pin, /*HIGH*/LOW);       // open relay 2; power down heating element
            updatecontrollerState(IDLE, IDLE);
            break;
        case COOL:
            double runTime = (unsigned long)(millis() - startTime) / 1000;
             if (runTime < coolMinOn)
                break;
            digitalWrite(_cool_pin, /*HIGH*/LOW);       // open relay 1; power down fridge compressor
            digitalWrite(_heat_pin, /*HIGH*/LOW);       // open relay 2; power down heating element
            updatecontrollerState(IDLE, IDLE);
            break;
    }
}

opState CoolerHeaterContoller::GetState()
{
  return (opState)controllerState[0];
}

opState CoolerHeaterContoller::GetStateBefore()
{
  return (opState)controllerState[1];
}

double CoolerHeaterContoller::GetPeakEstimator()
{
  return peakEstimator;
}
