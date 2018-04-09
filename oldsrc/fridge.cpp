#include "fridge.h"
#include "myUbidots.h"

byte fridgeState[2] = { IDLE, IDLE };      // [0] - current fridge state; [1] - fridge state t - 1 history
double peakEstimator = 30;    // to predict COOL overshoot; units of deg C per hour (always positive)
double peakEstimate = 0;      // to determine prediction error = (estimate - actual)
unsigned long startTime = 0;  // timing variables for enforcing min/max cycling times
unsigned long stopTime = 0;

int getFridgeStateVal(byte state)
{
    switch (state)
    {
        case IDLE:
            return 0;
        case HEAT:
            return 1;
        case COOL:
            return -1;
        default:
            return 0;
    }
}

/*void publishFridgeState()
{
    if(WiFi.ready())
    {
        ubidots.add("fridgenow", getFridgeStateVal(fridgeState[0]));
        //ubidots.add("fridgebefore", getFridgeStateVal(fridgeState[1]));
        ubidots.sendAll();
    }
}*/

bool gotoIdle()
{
    switch (fridgeState[0])
    {
        case IDLE:
        case HEAT:
            digitalWrite(relay1, /*HIGH*/LOW);       // open relay 1; power down fridge compressor
            digitalWrite(relay2, /*HIGH*/LOW);       // open relay 2; power down heating element
            digitalWrite(LED_BUILTIN, LOW);
            updateFridgeState(IDLE, IDLE);
            return false;
        case COOL:
            double runTime = (unsigned long)(millis() - startTime) / 1000;
             if (runTime < coolMinOn)
                return true;
            digitalWrite(relay1, /*HIGH*/LOW);       // open relay 1; power down fridge compressor
            digitalWrite(relay2, /*HIGH*/LOW);       // open relay 2; power down heating element
            digitalWrite(LED_BUILTIN, LOW);
            updateFridgeState(IDLE, IDLE);
            return false;
    }
    return true;
}

void updateFridge() {        // maintain fridge at temperature set by mainPID -- COOLing with predictive differential, HEATing with time proportioned heatPID
  switch (fridgeState[0]) {  // MAIN switch -- IDLE/peak detection, COOL, HEAT routines
    default:
    case IDLE:
    //TODO: implement minimum IDLE time between cool and heat (and vice versa)
      if (fridgeState[1] == IDLE) {   // only switch to HEAT/COOL if not waiting for COOL peak
        if ((fridge->getFilter() > Output + fridgeIdleDiff) && (((unsigned long)((millis() - stopTime) / 1000) > coolMinOff) || stopTime == 0) &&
            (fridge->getFilter() < NO_COOL_LIMIT)) {  // switch to COOL only if temp exceeds IDLE range and min off time met
          updateFridgeState(COOL);    // update current fridge status and t - 1 history
          digitalWrite(relay1, /*LOW*/HIGH);  // close relay 1; supply power to fridge compressor
          digitalWrite(LED_BUILTIN, HIGH);
          startTime = millis();       // record COOLing start time
        }
        else if ((fridge->getFilter() < Output - fridgeIdleDiff) && (((unsigned long)((millis() - stopTime) / 1000) > heatMinOff) || stopTime == 0) &&
            (fridge->getFilter() > NO_HEAT_LIMIT)) {  // switch to HEAT only if temp below IDLE range and min off time met
          updateFridgeState(HEAT);
          //if (programState & 0b010000) heatSetpoint = Output;  // update heat PID setpoint if in automatic mode
          //heatSetpoint = Output;
          //heatPID.Compute();      // compute new heat PID output, update timings to align PID and time proportioning routine
          heatPID.SetITerm(0);
          startTime = millis();   // start new time proportioned window
        }
      }
      else if (fridgeState[1] == COOL) {  // do peak detect if waiting on COOL
        if (fridge->peakDetect()) {        // negative peak detected...
          tuneEstimator(&peakEstimator, peakEstimate - fridge->getFilter());  // (error = estimate - actual) positive error requires larger estimator; negative:smaller
          fridgeState[1] = IDLE;          // stop peak detection until next COOL cycle completes
        }
        else {                                                               // no peak detected
          double offTime = (unsigned long)(millis() - stopTime) / 1000;      // IDLE time in seconds
          if (offTime < peakMaxWait) break;                                  // keep waiting for filter confirmed peak if too soon
          tuneEstimator(&peakEstimator, peakEstimate - fridge->getFilter());  // temp is drifting in the right direction, but too slowly; update estimator
          fridgeState[1] = IDLE;                                             // stop peak detection
        }
      }
      break;

    case COOL:  // run compressor until peak predictor lands on controller Output
      { double runTime = (unsigned long)(millis() - startTime) / 1000;  // runtime in seconds
      if (runTime < coolMinOn) break;     // ensure minimum compressor runtime
      if (fridge->getFilter() < Output - fridgeIdleDiff) {  // temp already below output - idle differential: most likely cause is change in setpoint or long minimum runtime
        updateFridgeState(IDLE, IDLE);    // go IDLE, ignore peaks
        digitalWrite(relay1, /*HIGH*/LOW);       // open relay 1; power down fridge compressor
        digitalWrite(LED_BUILTIN, LOW);
        stopTime = millis();              // record idle start
        break;
      }
      if ((fridge->getFilter() - (min(runTime, peakMaxTime) / 3600) * peakEstimator) < Output - fridgeIdleDiff) {  // if estimated peak exceeds Output - differential, set IDLE and wait for actual peak
        peakEstimate = fridge->getFilter() - (min(runTime, peakMaxTime) / 3600) * peakEstimator;   // record estimated peak prediction
        updateFridgeState(IDLE);     // go IDLE, wait for peak
        digitalWrite(relay1, /*HIGH*/LOW);
        digitalWrite(LED_BUILTIN, LOW);
        stopTime = millis();
      }
      if (runTime > coolMaxOn) {  // if compressor runTime exceeds max on time, skip peak detect, go IDLE
        updateFridgeState(IDLE, IDLE);
        digitalWrite(relay1, /*HIGH*/LOW);
        digitalWrite(LED_BUILTIN, LOW);
        stopTime = millis();
      }
      break; }

    case HEAT:  // run HEAT using time proportioning
      {
        heatSetpoint = Output;
        heatInput = fridgeTemp;
        heatPID.Compute();

        double runTime = millis() - startTime;  // runtime in ms
        while(runTime > heatWindow)
        {
            startTime += heatWindow;
            runTime -= heatWindow;
        }
        double heatOutput2 = heatOutput < 2 ? 0 : heatOutput;
        double heatOutputTime = heatWindow*heatOutput2/(double)100;
        //Serial.println("---HEAT START---");
        //Serial.print("runtime:");
        //Serial.println(runTime);
        //Serial.print("heatOutput:");
        //Serial.println(heatOutput);
        //Serial.print("SampleTime:");
        //Serial.println(heatPID.GetSampleTime());

      if ((runTime < heatOutputTime) && !digitalRead(relay2))
      {
        digitalWrite(relay2, /*LOW*/HIGH);           // active duty; close relay, write only once
        //Serial.println("turning relay ON");
      }
      else if ((runTime >= heatOutputTime) && digitalRead(relay2))
      {
        digitalWrite(relay2, /*HIGH*/LOW);  // active duty completed; rest of window idle; write only once
        //Serial.println("turning relay OFF");
      }
      //if (programState & 0b010000) heatSetpoint = Output;
      /*heatSetpoint = Output;
      if (heatPID.Compute()) {  // if heatPID computes (once per window), current window complete, start new
        //Serial.println("new PID values computed");
        startTime = millis();
      }*/
      if (fridge->getFilter() > Output + fridgeIdleDiff) {  // temp exceeds setpoint, go to idle to decide if it is time to COOL
        updateFridgeState(IDLE, IDLE);
        digitalWrite(relay2, /*HIGH*/LOW);
        stopTime = millis();
      }
      break; }
  }
}

void tuneEstimator(double* estimator, double error) {  // tune fridge overshoot estimator
  //if (abs(error) <= fridgePeakDiff) return;            // leave estimator unchanged if error falls within contstrained peak differential
  if ((-1*fridgePeakDiff) <= error && error <= 0) return;
  if (error > 0) *estimator *= constrain(1.2 + 0.03 * abs(error), 1.2, 1.5);                 // if positive error; increase estimator 20% - 50% relative to error
    else *estimator = max(0.05, *estimator / constrain(1.2 + 0.03 * abs(error), 1.2, 1.5));  // if negative error; decrease estimator 17% - 33% relative to error, constrain to non-zero value
  EEPROM.put(100, peakEstimator);              // update estimator value stored in EEPROM
}

void updateFridgeState(byte state) {  // update current fridge state
  //publishFridgeState();
  fridgeState[1] = fridgeState[0];
  fridgeState[0] = state;
  //publishFridgeState();
}

void updateFridgeState(byte state0, byte state1) {  // update current fridge state and history
  //publishFridgeState();
  fridgeState[1] = state1;
  fridgeState[0] = state0;
  //publishFridgeState();
}
