#include "ParticlePublisher.h"
#include "theApp.h"
#include "application.h"

void ParticlePublisher::init(const Model &model)
{
    Particle.connect();

    Particle.variable("beerTemp", model._appState.beerTemp);
    Particle.variable("fridgeTemp", model._appState.fridgeTemp);
    Particle.variable("setpoint", model._appConfig.setpoint);
    Particle.variable("output", model._appConfig.output);
    Particle.variable("heat-output", model._appConfig.heatOutput);
    Particle.variable("pidKp", model._appConfig.pid_Kp);
    Particle.variable("pidKi", model._appConfig.pid_Ki);
    Particle.variable("pidKd", model._appConfig.pid_Kd);
    Particle.variable("heatPidKp", model._appConfig.heatpid_Kp);
    Particle.variable("heatPidKi", model._appConfig.heatpid_Ki);
    Particle.variable("heatPidKd", model._appConfig.heatpid_Kd);
    Particle.variable("pidMode", model._appConfig.pid_mode);
    Particle.variable("heatPidMode", model._appConfig.heatpid_mode);
    Particle.variable("standBy", model._appConfig.standBy);

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

void ParticlePublisher::publish(const Model &model)
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
