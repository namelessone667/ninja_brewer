#include "ParticlePublisher.h"
#include "theApp.h"
#include "application.h"

void ParticlePublisher::init(const NinjaModel &model)
{
    if(registeredToCloud || !WiFi.ready())
      return;

    Particle.connect();

    Particle.variable("beerTemp", model.BeerTemp.Get());
    Particle.variable("fridgeTemp", model.FridgeTemp.Get());
    Particle.variable("setpoint", model.SetPoint.Get());
    Particle.variable("output", model.Output.Get());
    Particle.variable("heatOutput", model.HeatOutput.Get());
    //Particle.variable("pidKp", model.PID_Kp.Get());
    //Particle.variable("pidKi", model.PID_Ki.Get());
    //Particle.variable("pidKd", model.PID_Kd.Get());
    Particle.variable("heatPidKp", model.HeatPID_Kp.Get());
    Particle.variable("heatPidKi", model.HeatPID_Ki.Get());
    Particle.variable("heatPidKd", model.HeatPID_Kd.Get());
    Particle.variable("pidMode", model.PIDMode.Get());
    Particle.variable("heatPidMode", model.HeatPIDMode.Get());
    Particle.variable("standBy", model.StandBy.Get());
#ifdef TEMP_PROFILES
    Particle.variable("profileTemp", model.TempProfileTemperature.Get());
#endif
    IPAddress localIP = WiFi.localIP();
    sprintf(localIPstr, "%u.%u.%u.%u", localIP[0], localIP[1], localIP[2], localIP[3]);
    Particle.variable("localIP", localIPstr, STRING);

    Particle.function("setSetPoint", setNewSetPoint);
    Particle.function("setPIDAuto", setPIDModeAuto);
    Particle.function("setPIDManual", setPIDModeManual);
    Particle.function("setHPIDMan", setHeatPIDManual);
    Particle.function("setHPIDAuto", setHeatPIDAuto);
    Particle.function("reboot", reboot);
    Particle.function("startControl", startControl);
    Particle.function("stopControl", stopControl);
    Particle.function("saveState", saveState);
    Particle.function("reinitLCD", reinitLCD);
    Particle.function("switchSensor", switchSensors);


    registeredToCloud = true;
}

void ParticlePublisher::publish(const NinjaModel &model)
{
  if(!registeredToCloud)
    init(model);
}

#ifdef DEBUG_HERMS
void ParticlePublisher::publish(const NinjaModel& model, double pTerm, double iTerm, double dTerm)
{
  publish(model);
}
#endif

int ParticlePublisher::setNewSetPoint(String args)
{
  double setpoint = strtod(args, NULL);
  if(setpoint == 0.0)
    return -1;

  theApp::getInstance().setNewTargetTemp(setpoint);
  return 1;
}

int ParticlePublisher::setPIDModeAuto(String args)
{
  double output = strtod(args, NULL);
  theApp::getInstance().setPID(PID_AUTOMATIC, output);
  return 1;
}

int ParticlePublisher::setPIDModeManual(String args)
{
  double output = strtod(args, NULL);
  theApp::getInstance().setPID(PID_MANUAL, output);

  return 1;
}

int ParticlePublisher::setHeatPIDManual(String args)
{
  double output = strtod(args, NULL);
  theApp::getInstance().setHeatPID(PID_MANUAL, output);

  return 1;
}

int ParticlePublisher::setHeatPIDAuto(String args)
{
  double output = strtod(args, NULL);
  theApp::getInstance().setHeatPID(PID_AUTOMATIC, output);
  return 1;
}

int ParticlePublisher::reboot(String args)
{
  theApp::getInstance().reboot();
  return 1;
}

int ParticlePublisher::startControl(String args)
{
  theApp::getInstance().ActivateController();
  return 1;
}

int ParticlePublisher::stopControl(String args)
{
  theApp::getInstance().DisableController();
  return 1;
}

int ParticlePublisher::saveState(String args)
{
  theApp::getInstance().saveState();
  return 1;
}

int ParticlePublisher::reinitLCD(String args)
{
  theApp::getInstance().reinitLCD();
  return 1;
}

int ParticlePublisher::switchSensors(String)
{
  theApp::getInstance().switchSensors();
  return 1;
}
