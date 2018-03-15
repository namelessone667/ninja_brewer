#include "ParticlePublisher.h"
#include "theApp.h"
#include "application.h"

void ParticlePublisher::init(const NinjaModel &model)
{
    Particle.connect();

    Particle.variable("beerTemp", model.BeerTemp.Get());
    Particle.variable("fridgeTemp", model.FridgeTemp);
    Particle.variable("setpoint", model.SetPoint);
    Particle.variable("output", model.Output);
    Particle.variable("heat-output", model.HeatOutput);
    Particle.variable("pidKp", model.PID_Kp);
    Particle.variable("pidKi", model.PID_Ki);
    Particle.variable("pidKd", model.PID_Kd);
    Particle.variable("heatPidKp", model.HeatPID_Kp);
    Particle.variable("heatPidKi", model.HeatPID_Ki);
    Particle.variable("heatPidKd", model.HeatPID_Kd);
    Particle.variable("pidMode", model.PIDMode);
    Particle.variable("heatPidMode", model.HeatPIDMode);
    Particle.variable("standBy", model.StandBy);

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
}

void ParticlePublisher::publish(const NinjaModel &model)
{

}

int ParticlePublisher::setNewSetPoint(String args)
{
  double setpoint = strtod(args, NULL);
  if(setpoint == 0.0)
    return -1;

  if(setpoint > MAX_FRIDE_TEMP || setpoint < MIN_FRIDGE_TEMP)
    return 0;

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
